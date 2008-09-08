#include	"Timer.h"

Timer *Timer::_redTimer  = NULL;
Timer *Timer::_blueTimer = NULL;

Timer::Timer(int moves, int seconds, int increment)
{
  _moves     = moves;
  _seconds   = seconds;
  _increment = increment;

  resetTimer();
}

void Timer::setTimers(int moves, int seconds, int inceremnt)
{
  if (_redTimer) delete _redTimer;
  if (_blueTimer) delete _blueTimer;

  _redTimer  = new Timer(moves, seconds, inceremnt);
  _blueTimer = new Timer(moves, seconds, inceremnt);
}

bool Timer::haveTimeLeftForMove()
{
  if (_seconds == 0 && _increment == 0) return true; // Timer is shut off.

  
  int timePerMove = (_secondsLeft - 1) / _movesLeft + _increment;

  //cerr << "Time per move is: " << timePerMove << endl;

  time_t now = time(NULL);
  int used = (int)difftime(now,_startTime);
  //cerr << "Time used is: " << used << endl;
  if (used < timePerMove)
    return true;
  else
    return false;
}

void Timer::moveMade()
{
  int timeThisMove = (int)difftime(_endTime, _startTime);
  _secondsLeft -= timeThisMove;
  _secondsLeft += _increment;

  //cerr << "Time left was: " << _secondsLeft << endl;

  if (_movesLeft == 1 && _secondsLeft >= 0) resetTimer();
  else _movesLeft--;
}

bool Timer::surpasedTime()
{
  if (_secondsLeft < 0) return true;
  else return false;
}

void Timer::startTimer()
{
  _startTime = time(NULL);
}

void Timer::stopTimer()
{
  _endTime = time(NULL);
}

void Timer::resetTimer()
{
  _secondsLeft = _seconds;
  _movesLeft   = _moves;
}

Timer *Timer::timerForColor(color c)
{
  if (c == RED) return _redTimer;
  else if (c == BLUE) return _blueTimer;
  else return NULL;
}

int Timer::timeLeft()
{
  int used = (int)difftime(time(NULL), _startTime);
  return _seconds - used;
}
