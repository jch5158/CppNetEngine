@echo off

:: 인자로 전달받은 매크로 경로들
set "SOLUTION_DIR=%~1"
set "PROJECT_DIR=%~2"
set "VCPKG_TRIPLET=%~3"

:: 1. 폴더명 중복 사이에 백슬래시(\) 추가
set "VCPKG_TARGET_DIR=%SOLUTION_DIR%vcpkg_installed\%VCPKG_TRIPLET%\%VCPKG_TRIPLET%"

set "PROTOC_PATH=%VCPKG_TARGET_DIR%\tools\protobuf"
set "VCPKG_INC=%VCPKG_TARGET_DIR%\include"
set "VCPKG_BIN=%VCPKG_TARGET_DIR%\bin"

:: protoc.exe 실행 시 필요한 종속성 DLL 임시 PATH 추가
set "PATH=%VCPKG_BIN%;%PATH%"

set "PROTO_DIR=%SOLUTION_DIR%Common\Protocol"

:: 2. \" 이스케이프 방지를 위해 %PROJECT_DIR%. 형태로 마침표 추가
:: (Generated 같은 하위 폴더를 쓰는 것이 파일 관리상 가장 좋지만, 루트에 뽑으려면 이렇게 해야 함)
for %%f in ("%PROTO_DIR%\*.proto") do (
    "%PROTOC_PATH%\protoc.exe" -I="%PROTO_DIR%" -I="%VCPKG_INC%" --cpp_out="%PROJECT_DIR%\Generated\." "%%f"
)