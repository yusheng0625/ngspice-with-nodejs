/*
 * utils.hh
 *
 */

#ifndef INTERFACE_NGSPICE_HH_
#define INTERFACE_NGSPICE_HH_
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <ngspice/sharedspice.h>

namespace ngspice
{
	typedef struct _SpiceResult
	{
		double value;
		double time;
	} SpiceResult;

	class INgSpice
	{
	public:
		INgSpice();
		virtual ~INgSpice();
		static INgSpice *m_s;
		static INgSpice *instance()
		{
			if (m_s == nullptr)
			{
				m_s = new INgSpice();
			}
			return m_s;
		}

	public:
		// callbacks
		static int _SendChar(char *output, int id, void *data);
		static int _SendStat(char *status, int id, void *data);
		static int _ControlledExit(int status, NG_BOOL bUnload, NG_BOOL bQuit, int id, void *data);
		static int _SendData(pvecvaluesall all, int count, int id, void *data);
		static int _SendInitData(pvecinfoall all, int id, void *data);
		static int _BGThreadRunning(NG_BOOL bRunning, int id, void *data);

		// callbacks
		static int _GetVSRCData(double *voltages, double actualTime, char *nodeName, int id, void *data);
		static int _GetISRCData(double *currents, double actualTime, char *nodeName, int id, void *data);
		static int _GetSyncData(double actualTime, double *deltaTime, double oldDeltaTime, int redoStep, int id, int location, void *data);

		// callbacks
		static int _SendEvtData(int nodeIdx, double step, double value, char *sVal, void *bVal, int size, int mode, int id, void *data);
		static int _SendInitEvtData(int nodeIdx, int maxNodeIdx, char *nodeName, char *nodeType, int id, void *data);

	public:
		bool init();
		bool loadCircuit(std::string listings);
		void getResult(
			std::vector<std::vector<std::string>> &votagesList,
			std::vector<std::string> &currentsList,
			std::vector<std::tuple<std::string, double>> &votagesRes,
			std::vector<std::tuple<std::string, double>> &currentsRes);

		bool run();
		bool stop();
		bool isRunning();
		std::string logs()
		{
			return _logs;
		}

	private:
		bool command(std::string command);
		bool _bLoadedCircuit = false;
		std::string _logs;
		std::map<std::string, SpiceResult> _lastVectors;

	private:
		// parsing results.
		double getVoltage(std::string edge);
		double getCurrent(std::string element);
	};
}
using namespace ngspice;

#endif /* INTERFACE_NGSPICE_HH_ */
