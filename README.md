LCEMP is my Minecraft Legacy Console Edition source fork that enables LAN multiplayer hosting.

notes:
  - This is NOT the full source code.
  - You need to provide the required asset files yourself.
  - Code quality is not perfect. I am still learning C++.
  - If you use this in other LCE-based projects, credit me.

features:
  - Fully working multiplayer
  - Breaking and placing blocks synced
  - Kick system
  - Up to 8 players (modifiable in source)
  - Keyboard and mouse support
  - Gamma fixed
  - Fullscreen support

launch_arguments:
  - name: -name
    usage: "-name <username>"
    description: Sets your in-game username.

  - name: -ip
    usage: "-ip <targetip>"
    description: >
      Manually connect to an IP if LAN advertising does not work
      or if the server cannot be discovered automatically.

  - name: -port
    usage: "-port <targetport>"
    description: >
      Override the default port if it was changed in the source.

example:
  command: "Minecraft.Client.exe -name Steve -ip 192.168.0.25 -port 25565"

required_assets:
    directories:
  - path: Minecraft.Client\music\
  - path: Minecraft.Client\Common\Media\
  - path: Minecraft.Client\Common\res\
  - path: Minecraft.Client\Common\DummyTexturePack\
  - path: Minecraft.Client\DurangoMedia\
  - path: Minecraft.Client\OrbisMedia\
  - path: Minecraft.Client\PS3Media\
  - path: Minecraft.Client\PSVitaMedia\
  - path: Minecraft.Client\Windows64Media\
  - path: Minecraft.Client\redist64\
  - path: Minecraft.Client\PS3_GAME\
  - path: Minecraft.Client\PS4_GAME\
  - path: Minecraft.Client\sce_sys\
  - path: Minecraft.Client\TROPDIR\
  - path: Minecraft.Client\PS3\PS3Extras\DirectX\
  - path: Minecraft.Client\PS3\PS3Extras\HeapInspector\
  - path: Minecraft.Client\Common\Network\Sony\
  - path: Minecraft.Client\common\dlc\
  - path: Minecraft.Client\durango\sound\
  - path: Minecraft.Client\Xbox\4JLibs\
  - path: Minecraft.Client\Windows64\4JLibs\
  - path: Minecraft.Client\PSVita\4JLibs\
  - path: Minecraft.Client\PS3\4JLibs\
  - path: Minecraft.Client\Orbis\4JLibs\
  - path: Minecraft.Client\Durango\4JLibs\
  - path: Minecraft.Client\Windows64\Miles\
  - path: Minecraft.Client\PSVita\Miles\
  - path: Minecraft.Client\PS3\Miles\
  - path: Minecraft.Client\Orbis\Miles\
  - path: Minecraft.Client\Durango\Miles\
  - path: Minecraft.Client\Durango\Iggy\
  - path: Minecraft.Client\Windows64\Iggy\
  - path: Minecraft.Client\PSVita\Iggy\
  - path: Minecraft.Client\PS3\Iggy\
  - path: Minecraft.Client\Orbis\Iggy\
  - path: Minecraft.Client\PS3\Sentient\
  - path: Minecraft.Client\Orbis\Sentient\
  - path: Minecraft.Client\Durango\Sentient\
  - path: Minecraft.Client\Xbox\Sentient\
  - path: Minecraft.Client\Windows64\Sentient\
  - path: Minecraft.Client\PSVita\Sentient\
  - path: Minecraft.Client\PS3\PS3Extras\boost_1_53_0\

    files:
  - path: Minecraft.Client\xbox\MinecraftWindows.rc
  - path: Minecraft.Client\xbox\MinecraftWindows.ico
  - path: Minecraft.Client\xbox\small.ico
  - path: x64\Debug\iggy_w64.dll
  - path: x64\Debug\mss64.dll

install:
  - Get required assets.
  - Replace your Minecraft.Client and Minecraft.World source folder with this one.
  - Build.
  - Run with optional launch arguments if needed.

contributing:
  - If you find issues, open a PR.
  - I will review and merge if valid.

author: notpies
