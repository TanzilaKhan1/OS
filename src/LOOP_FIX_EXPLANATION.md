# Loop Fix - Read Until Enter Pressed

## 🐛 The Bug

**Problem:** Only reading ONE character then stopping immediately.

**Root Cause:** Wrong loop condition in `kmain.c:80`

### Before (BROKEN):
```c
while (total_len == 0) {  // ❌ WRONG!
    len = read(STDIN_FILENO, msg2 + total_len, sizeof(msg2) - total_len - 1);
    if (len > 0) {
        total_len += len;  // Now total_len > 0, loop EXITS!
```

**What happened:**
1. User types 's'
2. `read()` returns 1 byte
3. `total_len` becomes 1
4. Loop condition `total_len == 0` is now FALSE
5. Loop exits immediately
6. Only 's' was captured!

---

## ✅ The Fix

### After (CORRECT):
```c
int got_newline = 0;

while (!got_newline) {  // ✅ CORRECT! Loop until newline
    len = read(STDIN_FILENO, msg2 + total_len, sizeof(msg2) - total_len - 1);

    if (len > 0) {
        total_len += len;

        // Check if we got Enter key (newline)
        if (msg2[total_len - 1] == '\n' || msg2[total_len - 1] == '\r') {
            got_newline = 1;  // Now exit loop
            msg2[total_len] = '\0';
        }
    }
    // ... timeout check ...
}
```

**What happens now:**
1. User types 'h' → captured, loop continues
2. User types 'e' → captured, loop continues
3. User types 'l' → captured, loop continues
4. User types 'l' → captured, loop continues
5. User types 'o' → captured, loop continues
6. User presses Enter (sends '\n') → captured, `got_newline = 1`, loop exits
7. Full string "hello\n" captured! ✅

---

## 🎯 Expected Behavior Now

### Test Case 1: Simple Input
```
Enter text: hello world<Enter>
You entered: hello world
```

### Test Case 2: Long Input with Spaces
```
Enter text: the quick brown fox jumps over the lazy dog<Enter>
You entered: the quick brown fox jumps over the lazy dog
```

### Test Case 3: Numbers and Symbols
```
Enter text: test123!@#$%^&*()<Enter>
You entered: test123!@#$%^&*()
```

---

## 🔧 How It Works

### Character-by-Character Flow:

```
User types: H e l l o [Enter]
           ↓ ↓ ↓ ↓ ↓    ↓
           │ │ │ │ │    └─ '\n' detected → exit loop
           │ │ │ │ └────── 'o' stored, continue loop
           │ │ │ └──────── 'l' stored, continue loop
           │ │ └────────── 'l' stored, continue loop
           │ └──────────── 'e' stored, continue loop
           └────────────── 'H' stored, continue loop

Result: buffer = "Hello\n\0"
        total_len = 6 (including newline)
```

### With `read_available_nonblocking()`:

1. Each call to `read()` returns **available characters**
2. If typing slowly: returns 1 char at a time
3. If typing fast: may return multiple chars at once
4. Loop continues accumulating until '\n' or '\r' received
5. SysTick continues running throughout! ⚡

---

## 📊 Comparison

| Aspect | Old Code | New Code |
|--------|----------|----------|
| **Loop Condition** | `while (total_len == 0)` | `while (!got_newline)` |
| **Exit Trigger** | Any character | Enter key only |
| **Characters Read** | 1 only | All until Enter |
| **Behavior** | ❌ Broken | ✅ Correct |

---

## 🧪 Testing Tips

1. **Type slowly** (one char at a time) - should capture all
2. **Type fast** (paste text) - should capture all
3. **Press Enter** - should stop and display
4. **Try special chars** (`!@#$%^&*()`) - should work
5. **Watch SysTickTime** - should increase properly (not frozen)

---

## ✨ Key Points

- ✅ Loop now continues until **Enter is pressed**
- ✅ Accumulates **all characters** typed
- ✅ SysTick timer **keeps running** (non-blocking I/O)
- ✅ Works with **any length** input (up to buffer size)
- ✅ Handles **spaces and special characters**
- ✅ Proper **null termination** on exit

---

## 🚀 Flash & Test

```bash
# From compile directory
make clean && make

# Flash to board (use your method)
st-flash write target/duos 0x08000000

# Open serial terminal
screen /dev/tty.usbmodem1103 115200
```

**Expected output:**
```
Enter text: hello world how are you<Enter>
You entered: hello world how are you

SysTickTime: 5234  ← Should show actual time elapsed
```

---

## 🎉 Summary

The fix changes the loop from:
- **"Keep looping while buffer is empty"** (stops at first char)

To:
- **"Keep looping until newline received"** (reads full line)

This is the correct behavior for line-oriented input! 🎯
