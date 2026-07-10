MATEMON UPGRADE PATCH - OPERATOR BOSSES

Main concept fixed:
- Sumi is the addition boss only.
- Minus is the subtraction boss only.
- Multiplor is the multiplication boss only.

Flow:
1. The game asks you to scan the current boss/operator card.
2. Sumi generates only addition exercises.
3. When Sumi HP reaches 0, the game moves to Minus.
4. Minus generates only subtraction exercises.
5. When Minus HP reaches 0, the game moves to Multiplor.
6. Multiplor generates only multiplication exercises.
7. Correct answers damage the boss.
8. Wrong answers remove one life and show the correct answer.

Difficulty:
- Each boss gets harder internally as you damage it.
- Sumi starts with small additions and increases the number range.
- Minus starts with small non-negative subtraction and increases the number range.
- Multiplor starts with small times tables and grows up to 1-9 multiplication.

Button fix:
- Confirm now uses a debounced confirmPressed() helper.
- It should register one clean press instead of feeling like it needs multiple taps.

Buzzer:
- Config.h now uses BUZZER_PIN.
- Set USE_ACTIVE_BUZZER to 0 for passive piezo using tone().
- Set USE_ACTIVE_BUZZER to 1 for active buzzer modules.

Important:
- The game still accepts answers from 0 to 99 because the current input system uses two digit card scans.
