# Invokes a Cmd.exe shell script and updates the environment.
function Invoke-CmdScript {
  param(
    [String] $scriptName
  )
  $cmdLine = """$scriptName"" $args & set"
  & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
  select-string '^([^=]*)=(.*)$' | foreach-object {
    $varName = $_.Matches[0].Groups[1].Value
    $varValue = $_.Matches[0].Groups[2].Value
    set-item Env:$varName $varValue
  }
}

Invoke-CmdScript "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

$buildDir = "E:/sb"
$packagesDir = ".\packages"

Remove-Item -LiteralPath $buildDir -Force -Recurse -ErrorAction Ignore
Remove-Item -LiteralPath $packagesDir -Force -Recurse -ErrorAction Ignore
New-Item -ItemType Directory -Force -Path $buildDir
New-Item -ItemType Directory -Force -Path $packagesDir

cmake -B $buildDir -S . -G Ninja -DLIBMPV_DIR=E:/projects/mpv-dev-x86_64-20210103-git-3e175df -DLIBMPV_INCLUDE=E:/projects/mpv-dev-x86_64-20210103-git-3e175df/include -DCMAKE_BUILD_TYPE=Release -DVCPKG_INSTALL_OPTIONS="--x-buildtrees-root=E:/sa"  -DCMAKE_TOOLCHAIN_FILE=E:/projects/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCPACK_GENERATOR=WIX

cmake --build $buildDir -- -j4 package

Copy-Item "$buildDir\*.msi" -Destination $packagesDir



