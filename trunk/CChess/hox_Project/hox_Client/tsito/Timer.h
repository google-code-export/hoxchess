#ifndef	__TIMER_H__
#define	__TIMER_H__

/*
 * Timer.h (c) Noah Roberts 2003-04-12
 * Timer classes keep track of game clocks including move timers and total game clocks.
 */

#include	<string>
#include	"Board.h" // Definition of color.

#include	<ctime>

class Timer
{
private:
  static Timer *_redTimer;
  static Timer *_blueTimer;

  int	_moves;
  int	_movesLeft;

  int	_seconds;
  int	_secondsLeft;
  int	_increment;

  time_t	_startTime;
  time_t	_endTime;

  Timer(int moves, int seconds, int increment);

public:
  static void setTimers(int moves, int seconds, int increment);
  static Timer* timerForColor(color c);
  static void resetTimers() { _redTimer->resetTimer(); _blueTimer->resetTimer(); }

  bool	haveTimeLeftForMove();
  void 	moveMade();
  void	startTimer();
  void	stopTimer();
  void	resetTimer();

  bool	surpasedTime();

  int movesLeft() { return _movesLeft;   }
  int timeLeft();
  bool active() { return !(_moves == 0 && _seconds == 0); }
};


#endif /* __TIMER_H__ */
