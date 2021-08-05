#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "interfaces.h"
#include "args.h"
#include "key.h"
#include "game/input.h"
#include "game/state.h"
#include "game/pak.h"

// Return value of game_t::update
enum game_update_ret_t {
	GAME_UPDATE_CONTINUE = 0, // Continue game execution
	GAME_UPDATE_RESET, // Reload modules with new settings
	GAME_UPDATE_CLOSE, // Close game window
	GAME_UPDATE_FAILED, // The game failed to update and now must close
};

class game_t {
private:
	interfaces_t m_i; // Interfaces
	const args_t &m_a; // Command line arguments

  // Pak file
  pak_t m_pak;

	// Game state
	game_state_t *m_state;

public:
	// i: Interfaces
	// argc/argv: Command line arguments
	game_t(interfaces_t &i, const args_t &args);
	~game_t();

	// Run game tick
	game_update_ret_t update();

	// Get game state struct
	FINLINE const game_state_t &state() const {return *m_state;}

  // Get mutable game window state struct
  FINLINE game_state_win_t &wstate() {return m_state->w;}

  // Get mutable game render state struct
  FINLINE game_state_render_t &rstate() {return m_state->r;}
};

#endif //GAME_H
