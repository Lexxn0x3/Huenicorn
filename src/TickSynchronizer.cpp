#include <Huenicorn/TickSynchronizer.hpp>

#include <thread>


namespace Huenicorn
{
  TickSynchronizer::TickSynchronizer(TimeUnitType tickInterval):
  m_tickInterval(tickInterval)
  {
    std::fill(m_loadRateHistory, m_loadRateHistory + LoadRateHistorySize, Duration{0.0});
  }


  TickSynchronizer::TimeUnit TickSynchronizer::tickInterval() const
  {
    return m_tickInterval;
  }


  const TickSynchronizer::Excess& TickSynchronizer::lastExcess() const
  {
    return m_lastExcess;
  }


  float TickSynchronizer::loadRate(bool asPercents) const
  {
    return asPercents ? m_loadRate * 100 : m_loadRate;
  }


  void TickSynchronizer::setTickInterval(TimeUnitType tickInterval)
  {
    m_tickInterval = Duration{tickInterval};
  }


  void TickSynchronizer::start()
  {
    m_timePoint = ClockType::now();
  }


  bool TickSynchronizer::sync()
  {
    int factor = 1 + _syncWithTick(m_timePoint);
    m_timePoint += (m_tickInterval * factor);

    return factor <= 2;
  }


  TickSynchronizer::TimeUnitType TickSynchronizer::_syncWithTick(const TimePoint& startTime)
  {
    // Measure
    TimePoint now = ClockType::now();
    m_loadRate = _computeLoad(startTime, now);

    // Sync
    if(m_tickInterval == TimeUnit::zero()){
      return 0;
    }

    TimePoint next = startTime + m_tickInterval;

    if(next > now){
      std::this_thread::sleep_until(next);
    }
    else{
      Duration excess = now - (startTime + m_tickInterval);
      float ratio = excess / m_tickInterval;

      m_lastExcess = {excess, ratio};
      return m_lastExcess.rate;
    }

    return 0;
  }
}
