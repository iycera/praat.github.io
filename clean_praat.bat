@echo off
chcp 65001 >nul
echo ========================================
echo    Praat 清理脚本
echo ========================================
echo.

echo [1/3] 关闭 Praat 进程...
taskkill /IM Praat.exe /F >nul 2>&1
echo ✓ 已尝试关闭 Praat 进程

echo.
echo [2/3] 清理编译文件...
if exist *.o (
    del /q *.o
    echo ✓ 删除 .o 文件
)

if exist *.a (
    del /q *.a
    echo ✓ 删除 .a 文件
)

echo.
echo [3/3] 清理调试文件...
if exist debug_*.txt (
    del /q debug_*.txt
    echo ✓ 删除调试文件
)

if exist praat_language_preference.txt (
    del /q praat_language_preference.txt
    echo ✓ 删除语言偏好文件
)

echo.
echo ========================================
echo   清理完成
echo ========================================
echo 已清理的文件:
echo - 编译产生的 .o 和 .a 文件
echo - 调试输出文件
echo - 语言偏好文件
echo.
echo 保留的文件:
echo - Praat.exe (如果存在)
echo - 源代码文件
echo - 语言包文件
echo.

echo 按任意键退出...
pause >nul
