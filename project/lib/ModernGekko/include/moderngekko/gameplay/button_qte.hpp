#pragma once

#include <cstdint>

namespace moderngekko::game {

// Gameplay state for a mash-button QTE. This owns progress and completion;
// it has no motion, controller-protocol, or renderer dependency.
class ButtonQte {
public:
  struct Result {
    unsigned presses = 0;
    unsigned required = 0;
    bool advanced = false;
    bool completed_now = false;
  };

  // The integration supplies a live interaction identity and its input owner.
  // The press that entered the interaction must not also advance the QTE.
  bool Begin(std::uint64_t interaction, unsigned owner, unsigned required,
             bool button_down) {
    Reset();
    if (!interaction || owner >= 4 || !required) return false;
    m_interaction = interaction;
    m_owner = owner;
    m_required = required;
    m_armed = !button_down;
    return true;
  }

  Result Update(std::uint64_t interaction, unsigned port, bool button_down,
                bool accepting_input) {
    Result result{m_presses, m_required};
    if (!m_interaction || interaction != m_interaction || port != m_owner)
      return result;
    // Pause, disconnection, and other input suspensions preserve progress.
    // Resumption requires a release before another press can count.
    if (!accepting_input) {
      m_armed = false;
      return result;
    }
    if (!button_down) {
      m_armed = true;
      return result;
    }
    if (!m_armed || m_presses == m_required) return result;
    m_armed = false;
    result.presses = ++m_presses;
    result.advanced = true;
    result.completed_now = m_presses == m_required;
    return result;
  }

  // Call on cancellation, owner/target removal, level exit, or state load.
  // A new interaction always has its own progress and completion event.
  void Reset() {
    m_interaction = 0;
    m_owner = m_required = m_presses = 0;
    m_armed = false;
  }

private:
  std::uint64_t m_interaction = 0;
  unsigned m_owner = 0;
  unsigned m_required = 0;
  unsigned m_presses = 0;
  bool m_armed = false;
};

} // namespace moderngekko::game
