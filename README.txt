DV1633 Scripting and Other Languages - Course Project
======================================================
Student: Linus Johansson


BUILD INSTRUCTIONS
------------------
Requirements:
  - Visual Studio 2022 (or later) with the "Desktop development with C++" workload installed.
  - No additional software or package installation is needed; all
    dependencies (Raylib, EnTT, LuaBridge3, Lua 5.4) are included
    in the project folder.

Steps:
  1. Open "Application.sln" in the root of the project folder with Visual Studio.
  2. Set the build configuration to "Release" or "Debug" (either works) and
     the platform to "x64" using the dropdowns in the toolbar.
  3. Right-click the "Application" project in the Solution Explorer and
     select "Set as Startup Project".
  4. Press Ctrl+Shift+B (or Build > Build Solution) to compile.
  5. The executable is produced at:
       Application/x64/Debug/Application.exe   (Debug build)
       Application/x64/Release/Application.exe (Release build)

Note: The Lua script files are loaded using relative paths at runtime.
The executable must be run from a working directory where the folder
"src/lua/scripts/" is accessible. Running through Visual Studio (F5 or
Ctrl+F5) sets this automatically. If running the .exe directly, launch
it from the "Application/Application/" folder.


USAGE INSTRUCTIONS
------------------
Controls (in-game):
  A / D       Move left / right
  W           Jump (while grounded)
  Space       Attack
  R           Return to main menu at any time

Main Menu:
  The main menu presents three options:
    - Play   : starts the game using the current level (editor-saved or built-in).
    - Editor : opens the level editor.
    - Quit   : closes the application.

Game:
  The game is a 2D platformer. The player must defeat all enemies in
  each room, then walk to the right edge of the screen to advance to the
  next room. The final room is the boss arena; defeating the boss triggers
  the win screen. The player loses if their HP reaches zero or they fall
  off a platform.

Level Editor:
  The editor is accessed from the main menu and allows the player to
  design custom levels that are automatically used the next time the
  game is played.

  Toolbar (bottom of screen):
    Platform  - Click and drag on the canvas to draw a platform rectangle.
    Enemy     - Click on the canvas to place a patrol enemy (cursor = feet position).
    Boss      - Click on the canvas to place the boss enemy (cursor = feet position).
    Spawn     - Click on the canvas to set the player spawn point for the current room.
    < / >     - Switch between rooms.
    + Room    - Add a new room after the current one.
    - Room    - Delete the current room (minimum of one room required).
    Save      - Saves the level to "src/lua/scripts/level.lua". The game
                will use this file the next time Play is selected.
    Menu      - Return to the main menu.

  The last room in the list is always treated as the boss arena.
  Right-clicking any placed object (enemy or platform) removes it.
  All placement snaps to a 10-pixel grid.

  If no level has been saved, the game falls back to a built-in
  three-room layout automatically.
