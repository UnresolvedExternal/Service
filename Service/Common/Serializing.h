#define LOG(exp) StringANSI::Format("{0}: {1}", #exp, (exp)).StdPrintLine()
#define LOGS(exp) screen->Print(2000, (y += screen->FontY()) %= 8192, zSTRING{ #exp } + ": " + zSTRING{ exp })