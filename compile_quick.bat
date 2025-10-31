@echo off
chcp 65001 >nul

REM === 自动提取 PRAAT_VERSION_NUM ===
set PRAAT_VERSION_NUM=
for /f "tokens=3" %%i in ('findstr /b /c:"#define PRAAT_VERSION_NUM" main\main_Praat.h') do set PRAAT_VERSION_NUM=%%i

REM 输出自动检测到的版本号
echo PRAAT_VERSION_NUM = %PRAAT_VERSION_NUM%

REM 指定目标架构（默认64位。如需32位请修改下一行）
copy /y makefiles\makefile.defs.msys-mingw64 makefile.defs

REM 清理和关闭历史产物
taskkill /IM Praat%PRAAT_VERSION_NUM%i18n.exe /F >nul 2>&1
taskkill /IM Praat.exe /F >nul 2>&1
del Praat.exe >nul 2>nul
del Praat%PRAAT_VERSION_NUM%i18n.exe >nul 2>nul

setlocal enabledelayedexpansion

echo 开始编译...
mingw32-make -j4

if %errorlevel% equ 0 (
    if exist Praat%PRAAT_VERSION_NUM%i18n.exe (
        set MAIN_PRAAT=Praat%PRAAT_VERSION_NUM%i18n.exe
    ) else (
        set MAIN_PRAAT=Praat.exe
    )
    echo.
    echo ✓ 编译成功！!MAIN_PRAAT! 已生成
    echo.
    echo 运行测试: .\!MAIN_PRAAT!
) else (
    echo.
    echo ❌ 编译失败！
)
echo.
pause
