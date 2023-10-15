# SIC_assembler

111上系統程式Programming Assignment #1

---

使用 C 語言完成一個 SIC assembler。

功能：讀取一個 SIC assembly program，將其轉成 machine code，再生成一個object file。

report的Test run results指令未分類檔案，在CMD打開根目錄打以下指令執行即可。

```
gcc -c ./src/hw01.c -o ./obj/hw01.o

gcc -o ./bin/test.exe ./obj/hw01.o

cd bin
test.exe ../test/test.sic
```
