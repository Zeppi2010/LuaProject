DV1633 Scripting and Other Languages - Course Project
======================================================
Student: Linus Johansson


BUILD INSTRUCTIONS
------------------
Requirements:
  - Visual Studio 2022 (or later) with the "Desktop development with C++"
    workload installed.
  - No additional software or package installation is needed; all
    dependencies (Raylib, EnTT, LuaBridge3, Lua 5.4) are included
    in the project folder.

Steps:
  1. Open "Application.sln" (located in the project root folder) with
     Visual Studio.
  2. In the toolbar, set the build configuration to "Debug" and the
     platform to "x64".
  3. Right-click the "Application" project in the Solution Explorer and
     select "Set as Startup Project".
  4. Press Ctrl+Shift+B (or Build > Build Solution) to compile both the
     Lualib and Application projects.
  5. The compiled executable is produced at:
       Application\bin\Application_d.exe

Running from Visual Studio:
  Press F5 or Ctrl+F5. Visual Studio automatically sets the correct
  working directory, so the game will find its Lua scripts immediately.

Running the executable directly:
  The executable loads Lua scripts using the relative path
  "src\lua\scripts\", so it must be launched from the folder that
  contains the "src" directory. Navigate to the "Application\Application\"
  subfolder and run the executable from there, or create a shortcut with
  that folder set as the working (start-in) directory.

  Additionally, "raylib.dll" (found in "Dependencies\RayLib\lib\") must
  be present in the same folder as the executable for it to run. Copy it
  next to "Application_d.exe" if it is not already there.


USAGE INSTRUCTIONS
------------------
Controls (in-game):
  A / D       Move left / right
  W           Jump (while grounded)
  Space       Attack
  R           Return to main menu at any time

Main Menu:
  The main menu presents three options:
    - Play   : starts the game using the current level (editor-saved
               or built-in default).
    - Editor : opens the level editor.
    - Quit   : closes the application.

Game:
  The game is a 2D platformer. Defeat all enemies in a room, then walk
  to the right edge of the screen to advance to the next room. The final
  room is the boss arena; defeating the boss triggers the win screen.
  The player loses if their HP reaches zero or they fall off a platform.

Level Editor:
  The editor is accessed from the main menu and allows the player to
  design custom levels that are automatically used the next time the
  game is played.

  Toolbar (bottom of screen):
    Platform  - Click and drag on the canvas to draw a platform rectangle.
    Enemy     - Click on the canvas to place a patrol enemy
                (the cursor marks the feet position).
    Boss      - Click on the canvas to place the boss enemy
                (the cursor marks the feet position).
    Spawn     - Click on the canvas to set the player spawn point for
                the current room (the cursor marks the feet position).
    < / >     - Switch between rooms.
    + Room    - Add a new room after the current one.
    - Room    - Delete the current room (minimum of one room required).
    Save      - Saves the level to "src\lua\scripts\level.lua". The game
                will load this file the next time Play is selected.
    Menu      - Return to the main menu without saving.

  The last room in the list is always treated as the boss arena
  (labelled "[Boss room]" in the toolbar).
  Right-clicking any placed enemy or platform removes it.
  All placement snaps to a 10-pixel grid.

  If no level has been saved yet, the game falls back to a built-in
  three-room layout automatically.
