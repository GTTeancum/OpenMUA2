#include "moderngekko/gameplay/button_qte.hpp"
#include <iostream>

using moderngekko::game::ButtonQte;

int main() {
  ButtonQte qte;
  constexpr std::uint64_t target = 0x9001000000000201ull;
  if (!qte.Begin(target, 0, 12, true)) return 1;
  // Entering with X held and holding it for many updates must not count.
  for (int i = 0; i < 1000; ++i)
    if (qte.Update(target, 0, true, true).presses) return 2;
  for (unsigned press = 1; press <= 12; ++press) {
    const auto release = qte.Update(target, 0, false, true);
    if (release.advanced || release.completed_now) return 3;
    const auto down = qte.Update(target, 0, true, true);
    if (!down.advanced || down.presses != press || down.required != 12 ||
        down.completed_now != (press == 12)) return 4;
    for (int i = 0; i < 20; ++i) {
      const auto hold = qte.Update(target, 0, true, true);
      if (hold.advanced || hold.completed_now || hold.presses != press) return 5;
    }
  }
  // Completion is one event, even if the player continues mashing.
  qte.Update(target, 0, false, true);
  const auto finished = qte.Update(target, 0, true, true);
  if (finished.advanced || finished.completed_now || finished.presses != 12) return 6;

  if (!qte.Begin(target, 2, 3, false)) return 7;
  if (qte.Update(target, 0, true, true).advanced ||
      qte.Update(target + 1, 2, true, true).advanced) return 8;
  if (qte.Update(target, 2, true, true).presses != 1) return 9;
  qte.Update(target, 2, false, true);
  qte.Update(target, 2, false, false); // suspend while released
  if (qte.Update(target, 2, true, true).advanced) return 10;
  qte.Update(target, 2, false, true);
  if (qte.Update(target, 2, true, true).presses != 2) return 11;
  qte.Update(target, 2, true, false); // suspend while held
  if (qte.Update(target, 2, true, true).advanced) return 12;
  qte.Update(target, 2, false, true);
  if (!qte.Update(target, 2, true, true).completed_now) return 13;

  // Cancellation, state loading, and handle reuse cannot inherit old progress.
  qte.Reset();
  if (qte.Update(target, 2, true, true).advanced) return 14;
  if (!qte.Begin(target + 1, 2, 2, false) ||
      qte.Update(target + 1, 2, true, true).presses != 1) return 15;
  for (const auto invalid : {0, 1, 2}) {
    const bool accepted = invalid == 0 ? qte.Begin(0, 0, 2, false) :
        invalid == 1 ? qte.Begin(target, 4, 2, false) :
                       qte.Begin(target, 0, 0, false);
    if (accepted || qte.Update(target + 1, 2, true, true).advanced) return 16;
  }
  std::cout << "Button QTE: repeated presses, completion, ownership, suspension and reset passed\n";
}
