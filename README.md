# Raylib Blackjack (C Implementation)

This project, developed with **C programming** and the **Raylib library**, is a Blackjack game that is highly visual and supports local multiplayer. Apart from standard console applications, the interface elements are rotated to simulate a natural casino table perspective.

---

## 🌟 Features

* **Multiplayer Integration:** Up to 5 players can be added to the game.
* **Dynamic UI:** Player cards, buttons, and information are rotated using **trigonometric calculations** around the card table's center.
* **Advanced Click Control:** Specific collision detection algorithm implemented for rotated buttons.
* **Bet System:** Separated bet and balance calculations for every player.
* **Game Loop:** Add Player -> Betting -> Card Dealing -> Player Turn -> Dealer Turn -> Calculating Result.
* **Sound and Music:** Background music and card dealing sound effects.

---

## 🛠️ Setup and Compile

To compile this project, a C compiler (GCC/Clang), CMake, and the Raylib library should be installed on your computer.

### Requirements
* C99 compatible compiler
* CMake (3.20 or higher)
* Raylib 4.0+

### Step-by-Step Compiling Guide (Linux/macOS/Windows Git Bash)

1. **Clone the project or download:**
   ```bash
   git clone https://github.com/emin255/blackjack-raylib.git 
   cd blackjack-raylib
Create a build folder and start CMake:

Bash

mkdir build
cd build
cmake ..
Compile the Project:

Bash

make
Start the Game:

Note: Ensure that masa.png, cards.png, ses.ogg, and arkaplan.ogg are in the same folder as the .exe file.

Bash

./Blackjack
🎮 How To Play
Starting Screen: Add player(s) to the empty seats and press "Oyun Başlasın" (Start Game).

Betting Screen: Every player sets the bet amount by clicking +10, +50, +100 or bahsi sifirla (reset bet) and clicks Bahsi koy (Place bet).

Game Turn:

HIT: Take a card.

STAND: Play with the current score and end turn.

DOUBLE: Double the bet amount, take exactly one card, and pass the turn.

Dealer's Turn: Dealer takes cards until the total reaches 17.

Result: Winners and losers are determined, balances are updated, and the "Tekrar Oyna" (Play again) button appears.

📂 Project Structure
main.c: Game loop and UI Management.

blackjack.c: Game logic (Creating deck, shuffling, point calculation, determining winners).

blackjack.h: Data structures (struct oyuncu, struct kart).

CMakeLists.txt: Build configuration.

🧠 Technical Details
The critical part of this game is positioning the UI elements relative to the player's seat. To accomplish this, the method below is used in main.c:

C

// Example: Calculating the button's location relative to the player's seat
float rad = aci * DEG2RAD;
Vector2 forward = { -sinf(rad), cosf(rad) }; // Player's view direction
Vector2 buttonPos = { 
    seatPos.x + (forward.x * distance), 
    seatPos.y + (forward.y * distance) 
};
With this function, all buttons and information are aligned relative to the player's seat angle.

### Game Flow Diagram (State Machine)

```mermaid
stateDiagram-v2
    [*] --> ADD_PLAYER: Start
    ADD_PLAYER --> BETTING: "Start Game" Button
    BETTING --> DEALING: All Bets Placed
    DEALING --> PLAYER_TURN: Cards Dealt
    
    state PLAYER_TURN {
        [*] --> Decision
        Decision --> Hit: HIT
        Decision --> Stand: STAND
        Decision --> Double: DOUBLE
        Hit --> Decision: < 21
        Hit --> Stand: >= 21
    }

    PLAYER_TURN --> DEALER_TURN: All Players Finished
    DEALER_TURN --> RESULTS: Dealer >= 17
    RESULTS --> ADD_PLAYER: "Play Again" Button
