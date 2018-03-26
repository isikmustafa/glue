#ifndef __GLUE__CORE__TIMER__
#define __GLUE__CORE__TIMER__

#include <chrono>

namespace glue
{
	namespace core
	{
		class Timer
		{
		public:
			Timer();

			void start();
			double getTime();

		private:
			std::chrono::time_point<std::chrono::system_clock> m_start_time;
		};
	}
}

#endif