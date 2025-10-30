@echo off
chcp 65001 >nul
echo 编译 Praat i18n 版本...
echo.

echo 关闭可能运行的 Praat 进程...
taskkill /IM Praat.exe /F >nul 2>&1

echo 开始编译...
mingw32-make -j4

if %errorlevel% equ 0 (
    echo.
    echo ✓ 编译成功！Praat.exe 已生成
    echo.
    echo 运行测试: .\Praat.exe
) else (
    echo.
    echo ❌ 编译失败！
)

echo.
pause
