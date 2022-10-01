## Libportableのソースコードをビルドする方法

- C コンパイラ 
Microsoft Visual Studio .   
 
- or

- gcc, msys/msys2  
gcc for windows:  
https://sourceforge.net/projects/libportable/files/Tools/gcc-win64-10.1.1.7z  
msys msys2 project on:  
https://sourceforge.net/projects/mingw/files/MSYS  
https://sourceforge.net/projects/msys2/

## ビルド
- vc14以上  (cmd shell)  

	nmake -f Makefile.msvc clean  
	nmake -f Makefile.msvc
	
	link against minimal msvcrt.dll:  
	nmake -f Makefile.msvc MSVC_CRT=1

- gcc/mingw64 コンパイラ (msys shell)

	make clean  
	make
	
	GCCリンク時間最適化の有効化(LTO):   
	make LTO=1
	
	If gcc is a cross-compiler, use the CROSS_COMPILING option:
	
	make clean  
	64bits:  
	make CROSS_COMPILING=x86_64-w64-mingw32-  
	32bits:  
	make CROSS_COMPILING=x86_64-w64-mingw32- BITS=32  

- clangコンパイラー、MSVCコンパイラーがインストールされている場合  
	**(cmd shell):**
	
	nmake -f Makefile.msvc CC=clang-cl clean  
	nmake -f Makefile.msvc CC=clang-cl  
	
	**(msys shell):**  
	
	make clean  
	make CC=clang DFLAGS=--target=x86_64-w64-windows-gnu        // (64bits target build)  
	make CC=clang DFLAGS=--target=i686-w64-windows-gnu         // (32bits target build)  
	
	make clean  
	make CC=clang DFLAGS=--target=i686-pc-windows-msvc  
	make CC=clang DFLAGS=--target=x86_64-pc-windows-msvc  


## Firefox に libportable を追加しますか？
**バイナリ静的注入(例:**
	
	setdll32 /d:portable32.dll mozglue.dll       // 32 bits firefox    
	setdll64 /d:portable64.dll mozglue.dll       // 64 bits firefox    

**Firefoxのソースコードからコンパイルされたパッチの例:**   
```
diff --git a/toolkit/library/nsDllMain.cpp b/toolkit/library/nsDllMain.cpp
--- a/toolkit/library/nsDllMain.cpp
+++ b/toolkit/library/nsDllMain.cpp
@@ -8,6 +8,16 @@
 #include "mozilla/Assertions.h"
 #include "mozilla/WindowsVersion.h"
 
+#if defined(_MSC_VER) && defined(TT_MEMUTIL)
+#if defined(_M_IX86)
+#pragma comment(lib,"portable32.lib")
+#pragma comment(linker, "/include:_GetCpuFeature_tt")
+#elif defined(_M_AMD64) || defined(_M_X64)
+#pragma comment(lib,"portable64.lib")
+#pragma comment(linker, "/include:GetCpuFeature_tt")
+#endif
+#endif /* _MSC_VER && TT_MEMUTIL */
+
 #if defined(__GNUC__)
 // If DllMain gets name mangled, it won't be seen.
 extern "C" {
```