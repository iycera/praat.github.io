@echo off
chcp 65001 >nul

REM === 自动提取 PRAAT_VERSION_NUM ===
set PRAAT_VERSION_NUM=
for /f "tokens=3" %%i in ('findstr /b /c:"#define PRAAT_VERSION_NUM" main\main_Praat.h') do set PRAAT_VERSION_NUM=%%i

REM 输出自动检测到的版本号
echo PRAAT_VERSION_NUM = %PRAAT_VERSION_NUM%

REM 指定目标架构（默认32位，避免UTF-8乱码问题。如需64位请修改下一行）
REM 将 makefile.defs.msys-mingw32 改为 makefile.defs.msys-mingw64
copy /y makefiles\makefile.defs.msys-mingw32 makefile.defs

REM 生成嵌入语言包（每次编译前自动更新）
echo [0/5] 生成嵌入语言包...
python sys\generate_embedded_packs.py
if %errorlevel% neq 0 (
    echo 警告: 生成嵌入语言包失败，将继续使用现有文件
    echo 如果遇到乱码问题，请手动运行: python sys\generate_embedded_packs.py
)

REM 清理历史产物（先结束正在运行的进程，再删除旧文件）
taskkill /IM Praat%PRAAT_VERSION_NUM%i18n.exe /F >nul 2>&1
taskkill /IM Praat.exe /F >nul 2>&1
del Praat.exe >nul 2>nul
del Praat%PRAAT_VERSION_NUM%i18n.exe >nul 2>nul

setlocal enabledelayedexpansion

echo [1/5] 检查编译环境...
where mingw32-make >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 mingw32-make，请确保 MinGW 已安装并添加到 PATH
    pause
    exit /b 1
)
echo ✓ MinGW 环境检查通过

echo.
echo [2/5] 开始编译 Praat...
echo 使用多线程编译 (4 线程)
mingw32-make -j4

if %errorlevel% neq 0 (
    echo.
    echo ❌ 编译失败！请检查上面的错误信息
    echo.
    echo 常见问题解决方案:
    echo 1. 确保没有 Praat.exe 或 Praat%PRAAT_VERSION_NUM%i18n.exe 进程在运行
    echo 2. 检查源代码是否有语法错误
    echo 3. 确保所有依赖库都已编译
    echo.
    pause
    exit /b 1
)
echo.
echo [3/5] 编译完成！
if exist Praat%PRAAT_VERSION_NUM%i18n.exe (
    set "MAIN_PRAAT=Praat%PRAAT_VERSION_NUM%i18n.exe"
) else (
    set "MAIN_PRAAT=Praat.exe"
)
echo ✓ !MAIN_PRAAT! 已成功生成
echo.

echo ========================================
echo   编译信息
echo ========================================
echo 编译时间: %date% %time%
echo 输出文件: !MAIN_PRAAT!
echo 文件大小:
if exist !MAIN_PRAAT! (
    for %%A in (!MAIN_PRAAT!) do echo %%~zA 字节
) else (
    echo 文件未找到
)
echo.

echo 可用的语言包:
if exist sys\language_packs\*.json (
    dir /b sys\language_packs\*.json | find /c ".json"
    echo 个语言包文件
) else (
    echo 语言包目录未找到
)
echo.
echo ========================================
echo   测试建议
echo ========================================
echo 1. 运行: .\!MAIN_PRAAT!
echo 2. 检查 i18n 菜单是否显示
echo 3. 测试语言切换功能
echo 4. 查看 debug_i18n_simple.txt 调试信息
echo.
echo 按任意键退出...
pause >nul
