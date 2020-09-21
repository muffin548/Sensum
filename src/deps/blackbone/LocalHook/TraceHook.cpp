#include "TraceHook.h"
#include <Windows.h>
#include <algorithm>

#define BEA_USE_STDCALL

#define NAX Eax
#define NIP Eip
#define NDI Edi
#define NSP Esp

#define HIGHEST_BIT_SET     (1LL << (sizeof(void*) * 8 - 1))
#define HIGHEST_BIT_UNSET  ~HIGHEST_BIT_SET
#define SingleStep          0x100

#define ADDR_MASK 0xFFFFF000

namespace blackbone
{
	TraceHook::TraceHook()
	{
	}

	TraceHook::~TraceHook()
	{
		if (_pExptHandler != nullptr)
			RemoveVectoredExceptionHandler(_pExptHandler);
	}

	TraceHook& TraceHook::Instance()
	{
		static TraceHook instance;
		return instance;
	}

	bool TraceHook::ApplyHook(void* targetFunc,
		void* hookFunc,
		void* ptrAddress,
		const HookContext::vecState& tracePath /*= HookContext::vecState()*/,
		void* chekcIP /*= 0*/)
	{
		if (_contexts.count((uintptr_t)ptrAddress))
		{
			HookContext& ctx = _contexts[(uintptr_t)ptrAddress];

			// Already hooked
			if (ctx.hooks.count((uintptr_t)targetFunc))
				return false;
			else
				ctx.hooks.emplace((uintptr_t)targetFunc, std::make_pair((uintptr_t)hookFunc, false));
		}
		else
		{
			HookContext ctx;

			// Setup context
			ctx.targetPtr = (uintptr_t)ptrAddress;
			ctx.checkIP = (uintptr_t)chekcIP;
			ctx.origPtrVal = *(uintptr_t*)ptrAddress;
			ctx.breakValue = _breakPtr;
			ctx.tracePath = tracePath;

			ctx.hooks.emplace((uintptr_t)targetFunc, std::make_pair((uintptr_t)hookFunc, false));
			_contexts.emplace((uintptr_t)ptrAddress, std::move(ctx));

			if (_pExptHandler == nullptr)
				_pExptHandler = AddVectoredExceptionHandler(0, &TraceHook::VecHandler);

			// Setup exception
			*(uintptr_t*)ptrAddress = ctx.breakValue;
			_breakPtr += 0x10;
		}

		return true;
	}

	bool TraceHook::RemoveHook(void* targetFunc)
	{
		auto findfn = [targetFunc](const mapContext::value_type& val) {
			return val.second.hooks.count((uintptr_t)targetFunc);
		};

		auto iter = std::find_if(_contexts.begin(), _contexts.end(), findfn);

		if (iter != _contexts.end())
		{
			auto& ctx = iter->second;

			// Remove function from list
			ctx.hooks.erase((uintptr_t)targetFunc);

			if (ctx.hooks.empty())
			{
				// Remove hook itself
				*(uintptr_t*)ctx.targetPtr = ctx.origPtrVal;
				_contexts.erase(iter);
			}

			Sleep(10);

			// Remove exception handler
			if (_contexts.empty() && _pExptHandler != nullptr)
			{
				RemoveVectoredExceptionHandler(_pExptHandler);
				_pExptHandler = nullptr;
			}

			return true;
		}

		return false;
	}

	LONG __stdcall TraceHook::VecHandler(PEXCEPTION_POINTERS ExceptionInfo)
	{
		return Instance().VecHandlerP(ExceptionInfo);
	}

	LONG TraceHook::VecHandlerP(PEXCEPTION_POINTERS ExceptionInfo)
	{
		auto exptContex = ExceptionInfo->ContextRecord;
		auto exptRecord = ExceptionInfo->ExceptionRecord;
		auto exptCode = exptRecord->ExceptionCode;

		HookContext* ctx = &_contexts.begin()->second;

		if (exptCode != EXCEPTION_SINGLE_STEP && exptCode != EXCEPTION_ACCESS_VIOLATION)
		{
			return EXCEPTION_CONTINUE_SEARCH;
		}
		else if (exptCode == EXCEPTION_ACCESS_VIOLATION && (ctx->state == TS_Step || ctx->state == TS_StepInto))
		{
			if ((exptRecord->ExceptionInformation[1] & ADDR_MASK) != (ctx->breakValue & ADDR_MASK))
				return EXCEPTION_CONTINUE_SEARCH;

			if (ctx->checkIP != 0 && exptContex->NIP != ctx->checkIP)
			{
				exptContex->EFlags |= SingleStep;

				RestorePtr(*ctx, ExceptionInfo);
				return EXCEPTION_CONTINUE_EXECUTION;
			}
		}

		switch (ctx->state)
		{
		case TS_Start:
		{
			ctx->state = ctx->tracePath[ctx->stateIdx].action;

			RestorePtr(*ctx, ExceptionInfo);
			return VecHandlerP(ExceptionInfo);
		}
		break;

		case TS_Step:
		{
			if (CheckBranching(*ctx, exptContex->NIP, exptContex->NSP))
			{
				if (ctx->hooks.count(exptContex->NIP))
				{
					HandleBranch(*ctx, exptContex);
					return EXCEPTION_CONTINUE_EXECUTION;
				}
				else
				{
					ctx->state = TS_WaitReturn;
					BreakOnReturn(exptContex->NSP);
				}
			}
			else
				exptContex->EFlags |= SingleStep;
		}
		break;

		case TS_StepOut:
		{
			// Get current stack frame
			vecStackFrames frames;
			StackBacktrace(exptContex->NIP, exptContex->NSP, frames, 1);

			if (frames.size() > 1)
			{
				ctx->stateIdx++;
				ctx->state = TS_WaitReturn;
				BreakOnReturn(frames.back().first);
			}
		}
		break;

		case TS_StepInto:
		{
			// Check if step into path function has occurred
			if (CheckBranching(*ctx, exptContex->NIP, exptContex->NSP))
			{
				if (exptContex->NIP == ctx->tracePath[ctx->stateIdx].arg)
				{
					ctx->stateIdx++;
					ctx->state = ctx->tracePath[ctx->stateIdx].action;
				}
			}

			exptContex->EFlags |= SingleStep;
		}
		break;

		case TS_WaitReturn:
		{
			exptContex->NIP &= HIGHEST_BIT_UNSET;

			exptContex->EFlags |= SingleStep;
			ctx->state = ctx->tracePath[ctx->stateIdx].action;
		}
		break;

		default:
			break;
		}

		ctx->lastIP = exptContex->NIP;
		ctx->lastSP = exptContex->NSP;

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	bool TraceHook::CheckBranching(const HookContext& ctx, uintptr_t ip, uintptr_t sp)
	{
		// Not yet initialized
		if (ctx.lastIP == 0 || ctx.lastSP == 0)
			return false;

		if (ip - ctx.lastIP >= 8 && sp != ctx.lastSP)
		{
			//DISASM info = { 0 };
			//info.EIP = ctx.lastIP;
		}

		return false;
	}

	void TraceHook::HandleBranch(HookContext& ctx, PCONTEXT exptContex)
	{
		ctx.hooks[exptContex->NIP].second = true;

		auto iter = std::find_if(ctx.hooks.begin(), ctx.hooks.end(),
			[](const decltype(ctx.hooks)::value_type& val) { return (val.second.second == false); });

		if (iter != ctx.hooks.end())
		{
			ctx.state = TS_WaitReturn;
			BreakOnReturn(exptContex->NSP);
		}
		else
			ctx.reset();

		exptContex->NIP = ctx.hooks[exptContex->NIP].first;
	}

	inline void TraceHook::BreakOnReturn(uintptr_t sp)
	{
		*(DWORD_PTR*)sp |= HIGHEST_BIT_SET;
	}

	bool TraceHook::RestorePtr(const HookContext& ctx, PEXCEPTION_POINTERS ExceptionInfo)
	{
		bool found = false;
		auto expCtx = ExceptionInfo->ContextRecord;

		if (ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 8)
		{
			expCtx->NIP = ctx.origPtrVal;
			return true;
		}

		for (DWORD_PTR* pRegVal = &expCtx->NDI; pRegVal <= &expCtx->NAX; pRegVal++)
		{
			// Compare high address parts
			if ((*pRegVal & ADDR_MASK) == (ExceptionInfo->ExceptionRecord->ExceptionInformation[1] & ADDR_MASK))
			{
				*pRegVal = ctx.origPtrVal;
				found = true;
			}
		}

		return found;
	}

	size_t TraceHook::StackBacktrace(uintptr_t ip, uintptr_t sp, vecStackFrames& results, uintptr_t depth /*= 10 */)
	{
		SYSTEM_INFO sysinfo = {};
		uintptr_t stack_base = (uintptr_t)((PNT_TIB)NtCurrentTeb())->StackBase;

		GetNativeSystemInfo(&sysinfo);

		results.emplace_back(0, ip);

		for (uintptr_t stackPtr = sp; stackPtr < stack_base && results.size() <= depth; stackPtr += sizeof(void*))
		{
			uintptr_t stack_val = *(uintptr_t*)stackPtr;
			MEMORY_BASIC_INFORMATION meminfo = { 0 };

			uintptr_t original = stack_val & HIGHEST_BIT_UNSET;

			if (original < (uintptr_t)sysinfo.lpMinimumApplicationAddress ||
				original >(uintptr_t)sysinfo.lpMaximumApplicationAddress)
			{
				continue;
			}

			if (VirtualQuery((LPVOID)original, &meminfo, sizeof(meminfo)) != sizeof(meminfo))
				continue;

			if (meminfo.Protect != PAGE_EXECUTE_READ &&
				meminfo.Protect != PAGE_EXECUTE_WRITECOPY &&
				meminfo.Protect != PAGE_EXECUTE_READWRITE)
			{
				continue;
			}

			for (uintptr_t j = 1; j < 8; j++)
			{
				//DISASM info = { 0 };
				//info.EIP = original - j;
			}
		}

		return results.size();
	}
}
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TSUIENJRLC
{ 
  void teSHTBKsnt()
  { 
      bool DRYzdFluBS = false;
      bool GNiIpYbyrl = false;
      bool jlpJjBDBch = false;
      bool bFQZihekjF = false;
      bool GWiTTAZGVh = false;
      bool udDXnRbOPc = false;
      bool CkRTqBxJZI = false;
      bool tqNmeMCVoi = false;
      bool bxeulxBrlR = false;
      bool FPUHCUFPcS = false;
      bool DPfNnEQQra = false;
      bool OGZfAFGWgs = false;
      bool zEXICeVFjJ = false;
      bool HmrVfLQRGg = false;
      bool gkAKCzonGx = false;
      bool OihiSyrXKJ = false;
      bool iKfhSaNDAo = false;
      bool nnPAMOEMFC = false;
      bool ogzmtYFJCg = false;
      bool SrHxyVERqb = false;
      string tQCKpEdGwb;
      string KDbbBThhgi;
      string fsKpwkFgBD;
      string GMNyaxrrCo;
      string yVtTPBgHNM;
      string eaHYjAGLOE;
      string zwFsUypdAE;
      string apfrkKzqZt;
      string ExTnYeEnOa;
      string pfcxoBQByQ;
      string KCGJknuOnY;
      string APlTzBcQUp;
      string tCOYNtMhCj;
      string syrJjBqKDj;
      string joAoaatJxn;
      string XAfHWImhup;
      string bMMDeCWbjy;
      string KSNkFEBZym;
      string ADjOLsRkla;
      string NIOMHXcwJX;
      if(tQCKpEdGwb == KCGJknuOnY){DRYzdFluBS = true;}
      else if(KCGJknuOnY == tQCKpEdGwb){DPfNnEQQra = true;}
      if(KDbbBThhgi == APlTzBcQUp){GNiIpYbyrl = true;}
      else if(APlTzBcQUp == KDbbBThhgi){OGZfAFGWgs = true;}
      if(fsKpwkFgBD == tCOYNtMhCj){jlpJjBDBch = true;}
      else if(tCOYNtMhCj == fsKpwkFgBD){zEXICeVFjJ = true;}
      if(GMNyaxrrCo == syrJjBqKDj){bFQZihekjF = true;}
      else if(syrJjBqKDj == GMNyaxrrCo){HmrVfLQRGg = true;}
      if(yVtTPBgHNM == joAoaatJxn){GWiTTAZGVh = true;}
      else if(joAoaatJxn == yVtTPBgHNM){gkAKCzonGx = true;}
      if(eaHYjAGLOE == XAfHWImhup){udDXnRbOPc = true;}
      else if(XAfHWImhup == eaHYjAGLOE){OihiSyrXKJ = true;}
      if(zwFsUypdAE == bMMDeCWbjy){CkRTqBxJZI = true;}
      else if(bMMDeCWbjy == zwFsUypdAE){iKfhSaNDAo = true;}
      if(apfrkKzqZt == KSNkFEBZym){tqNmeMCVoi = true;}
      if(ExTnYeEnOa == ADjOLsRkla){bxeulxBrlR = true;}
      if(pfcxoBQByQ == NIOMHXcwJX){FPUHCUFPcS = true;}
      while(KSNkFEBZym == apfrkKzqZt){nnPAMOEMFC = true;}
      while(ADjOLsRkla == ADjOLsRkla){ogzmtYFJCg = true;}
      while(NIOMHXcwJX == NIOMHXcwJX){SrHxyVERqb = true;}
      if(DRYzdFluBS == true){DRYzdFluBS = false;}
      if(GNiIpYbyrl == true){GNiIpYbyrl = false;}
      if(jlpJjBDBch == true){jlpJjBDBch = false;}
      if(bFQZihekjF == true){bFQZihekjF = false;}
      if(GWiTTAZGVh == true){GWiTTAZGVh = false;}
      if(udDXnRbOPc == true){udDXnRbOPc = false;}
      if(CkRTqBxJZI == true){CkRTqBxJZI = false;}
      if(tqNmeMCVoi == true){tqNmeMCVoi = false;}
      if(bxeulxBrlR == true){bxeulxBrlR = false;}
      if(FPUHCUFPcS == true){FPUHCUFPcS = false;}
      if(DPfNnEQQra == true){DPfNnEQQra = false;}
      if(OGZfAFGWgs == true){OGZfAFGWgs = false;}
      if(zEXICeVFjJ == true){zEXICeVFjJ = false;}
      if(HmrVfLQRGg == true){HmrVfLQRGg = false;}
      if(gkAKCzonGx == true){gkAKCzonGx = false;}
      if(OihiSyrXKJ == true){OihiSyrXKJ = false;}
      if(iKfhSaNDAo == true){iKfhSaNDAo = false;}
      if(nnPAMOEMFC == true){nnPAMOEMFC = false;}
      if(ogzmtYFJCg == true){ogzmtYFJCg = false;}
      if(SrHxyVERqb == true){SrHxyVERqb = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YETFANLJPG
{ 
  void nsKhMMGPmP()
  { 
      bool CdkCCXIjfT = false;
      bool FcdMAmPtPp = false;
      bool agJFETaOhh = false;
      bool kwtGXPAzYA = false;
      bool wgQrrDkfEk = false;
      bool ExCxJVTfxi = false;
      bool xsByiJiXoj = false;
      bool lQVOtsPQMA = false;
      bool htKxcmtwwB = false;
      bool uTBpDCfRJi = false;
      bool UNqTynamDP = false;
      bool FDTxACiDUu = false;
      bool OlanoSPfLq = false;
      bool NxHcfUDqzo = false;
      bool MHsgurAEpA = false;
      bool SbxNThmCee = false;
      bool XuaLCIupQe = false;
      bool yrDeiHstrw = false;
      bool SLoKXlAsIy = false;
      bool gALUQQfgem = false;
      string xOHDREpHho;
      string AFkctkZWXb;
      string YtFdaUKPCh;
      string IIZLfyJmOB;
      string SysDfUBloP;
      string lOINBCJdbx;
      string saFzgrfZxu;
      string alHWIZBBxG;
      string SYZmwsALJd;
      string nfKRHrssWn;
      string nantnzHkFa;
      string AIfVdUARCn;
      string qUPTGtapOQ;
      string fIeJRrjfyw;
      string lOGhcfAonm;
      string rbphwTKUmt;
      string YRWntrcWDk;
      string jTSGCcNhqZ;
      string tlBbYERwMy;
      string gPsEXTrRmj;
      if(xOHDREpHho == nantnzHkFa){CdkCCXIjfT = true;}
      else if(nantnzHkFa == xOHDREpHho){UNqTynamDP = true;}
      if(AFkctkZWXb == AIfVdUARCn){FcdMAmPtPp = true;}
      else if(AIfVdUARCn == AFkctkZWXb){FDTxACiDUu = true;}
      if(YtFdaUKPCh == qUPTGtapOQ){agJFETaOhh = true;}
      else if(qUPTGtapOQ == YtFdaUKPCh){OlanoSPfLq = true;}
      if(IIZLfyJmOB == fIeJRrjfyw){kwtGXPAzYA = true;}
      else if(fIeJRrjfyw == IIZLfyJmOB){NxHcfUDqzo = true;}
      if(SysDfUBloP == lOGhcfAonm){wgQrrDkfEk = true;}
      else if(lOGhcfAonm == SysDfUBloP){MHsgurAEpA = true;}
      if(lOINBCJdbx == rbphwTKUmt){ExCxJVTfxi = true;}
      else if(rbphwTKUmt == lOINBCJdbx){SbxNThmCee = true;}
      if(saFzgrfZxu == YRWntrcWDk){xsByiJiXoj = true;}
      else if(YRWntrcWDk == saFzgrfZxu){XuaLCIupQe = true;}
      if(alHWIZBBxG == jTSGCcNhqZ){lQVOtsPQMA = true;}
      if(SYZmwsALJd == tlBbYERwMy){htKxcmtwwB = true;}
      if(nfKRHrssWn == gPsEXTrRmj){uTBpDCfRJi = true;}
      while(jTSGCcNhqZ == alHWIZBBxG){yrDeiHstrw = true;}
      while(tlBbYERwMy == tlBbYERwMy){SLoKXlAsIy = true;}
      while(gPsEXTrRmj == gPsEXTrRmj){gALUQQfgem = true;}
      if(CdkCCXIjfT == true){CdkCCXIjfT = false;}
      if(FcdMAmPtPp == true){FcdMAmPtPp = false;}
      if(agJFETaOhh == true){agJFETaOhh = false;}
      if(kwtGXPAzYA == true){kwtGXPAzYA = false;}
      if(wgQrrDkfEk == true){wgQrrDkfEk = false;}
      if(ExCxJVTfxi == true){ExCxJVTfxi = false;}
      if(xsByiJiXoj == true){xsByiJiXoj = false;}
      if(lQVOtsPQMA == true){lQVOtsPQMA = false;}
      if(htKxcmtwwB == true){htKxcmtwwB = false;}
      if(uTBpDCfRJi == true){uTBpDCfRJi = false;}
      if(UNqTynamDP == true){UNqTynamDP = false;}
      if(FDTxACiDUu == true){FDTxACiDUu = false;}
      if(OlanoSPfLq == true){OlanoSPfLq = false;}
      if(NxHcfUDqzo == true){NxHcfUDqzo = false;}
      if(MHsgurAEpA == true){MHsgurAEpA = false;}
      if(SbxNThmCee == true){SbxNThmCee = false;}
      if(XuaLCIupQe == true){XuaLCIupQe = false;}
      if(yrDeiHstrw == true){yrDeiHstrw = false;}
      if(SLoKXlAsIy == true){SLoKXlAsIy = false;}
      if(gALUQQfgem == true){gALUQQfgem = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DYUQANNBUD
{ 
  void EFERDZqrKo()
  { 
      bool NDqxeqEFSn = false;
      bool KSflnpfAqG = false;
      bool fbsxGRrNjd = false;
      bool ZbjeTXaggY = false;
      bool rrxhfeDSZf = false;
      bool PksyGlKsqM = false;
      bool zpVoOhiASP = false;
      bool PySROneLRa = false;
      bool NZnDOnilYh = false;
      bool MgpUZBWHSy = false;
      bool wDzJQQSbjS = false;
      bool GzbPzZCRaZ = false;
      bool zwbkDiIyqM = false;
      bool RWrSdKZdmx = false;
      bool VGBFaNFaVA = false;
      bool zEgrzzbOGg = false;
      bool ITjWBiHIko = false;
      bool zXdcEruVJo = false;
      bool dhcqMMpzTe = false;
      bool hLWFoBWdEq = false;
      string mabfgeqzxX;
      string fBZtpfJkVf;
      string wlpGjHgmEX;
      string pRmlGsbdnM;
      string OgZGYrtTFO;
      string lpNfwBkUuY;
      string xDtYgONmmx;
      string DfwIROLihr;
      string NNgolAEHoF;
      string IHhOPLlAZK;
      string dGfILGsFOg;
      string oBmCLiVFmg;
      string KGyhnwxhrG;
      string cLJWOGhYcf;
      string HysIPGTfCf;
      string ofNsAXIxTP;
      string ILMIDdrPnF;
      string BxbHZLNoRh;
      string bReLQwxQWp;
      string yjENWulkFl;
      if(mabfgeqzxX == dGfILGsFOg){NDqxeqEFSn = true;}
      else if(dGfILGsFOg == mabfgeqzxX){wDzJQQSbjS = true;}
      if(fBZtpfJkVf == oBmCLiVFmg){KSflnpfAqG = true;}
      else if(oBmCLiVFmg == fBZtpfJkVf){GzbPzZCRaZ = true;}
      if(wlpGjHgmEX == KGyhnwxhrG){fbsxGRrNjd = true;}
      else if(KGyhnwxhrG == wlpGjHgmEX){zwbkDiIyqM = true;}
      if(pRmlGsbdnM == cLJWOGhYcf){ZbjeTXaggY = true;}
      else if(cLJWOGhYcf == pRmlGsbdnM){RWrSdKZdmx = true;}
      if(OgZGYrtTFO == HysIPGTfCf){rrxhfeDSZf = true;}
      else if(HysIPGTfCf == OgZGYrtTFO){VGBFaNFaVA = true;}
      if(lpNfwBkUuY == ofNsAXIxTP){PksyGlKsqM = true;}
      else if(ofNsAXIxTP == lpNfwBkUuY){zEgrzzbOGg = true;}
      if(xDtYgONmmx == ILMIDdrPnF){zpVoOhiASP = true;}
      else if(ILMIDdrPnF == xDtYgONmmx){ITjWBiHIko = true;}
      if(DfwIROLihr == BxbHZLNoRh){PySROneLRa = true;}
      if(NNgolAEHoF == bReLQwxQWp){NZnDOnilYh = true;}
      if(IHhOPLlAZK == yjENWulkFl){MgpUZBWHSy = true;}
      while(BxbHZLNoRh == DfwIROLihr){zXdcEruVJo = true;}
      while(bReLQwxQWp == bReLQwxQWp){dhcqMMpzTe = true;}
      while(yjENWulkFl == yjENWulkFl){hLWFoBWdEq = true;}
      if(NDqxeqEFSn == true){NDqxeqEFSn = false;}
      if(KSflnpfAqG == true){KSflnpfAqG = false;}
      if(fbsxGRrNjd == true){fbsxGRrNjd = false;}
      if(ZbjeTXaggY == true){ZbjeTXaggY = false;}
      if(rrxhfeDSZf == true){rrxhfeDSZf = false;}
      if(PksyGlKsqM == true){PksyGlKsqM = false;}
      if(zpVoOhiASP == true){zpVoOhiASP = false;}
      if(PySROneLRa == true){PySROneLRa = false;}
      if(NZnDOnilYh == true){NZnDOnilYh = false;}
      if(MgpUZBWHSy == true){MgpUZBWHSy = false;}
      if(wDzJQQSbjS == true){wDzJQQSbjS = false;}
      if(GzbPzZCRaZ == true){GzbPzZCRaZ = false;}
      if(zwbkDiIyqM == true){zwbkDiIyqM = false;}
      if(RWrSdKZdmx == true){RWrSdKZdmx = false;}
      if(VGBFaNFaVA == true){VGBFaNFaVA = false;}
      if(zEgrzzbOGg == true){zEgrzzbOGg = false;}
      if(ITjWBiHIko == true){ITjWBiHIko = false;}
      if(zXdcEruVJo == true){zXdcEruVJo = false;}
      if(dhcqMMpzTe == true){dhcqMMpzTe = false;}
      if(hLWFoBWdEq == true){hLWFoBWdEq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZNFTNLXVXR
{ 
  void ycPVbMVCfw()
  { 
      bool FmGOJFVrpo = false;
      bool DfiWMfTxIm = false;
      bool wkHJFzwXPa = false;
      bool MdnQGDbaFQ = false;
      bool xotPPlCePD = false;
      bool hRwrsUewIJ = false;
      bool IxbObYqSPI = false;
      bool EsqqoGqqsT = false;
      bool xRIMsugwWl = false;
      bool XeRwHzMenq = false;
      bool uiOGznaFCl = false;
      bool dnCoLqoPVg = false;
      bool quudjZbuip = false;
      bool sLDCYNCBME = false;
      bool TbtjIjbbjQ = false;
      bool JwIHzznkVZ = false;
      bool MfVMrRjAQx = false;
      bool onCQMPBKUA = false;
      bool xzKSfwCwDY = false;
      bool ZQemrnGgqp = false;
      string LrlWWBGzVe;
      string fJzwYixyQj;
      string umHLaJCQts;
      string reBPxkINqD;
      string ZFUuUNmNKU;
      string OsiNjXkZuM;
      string yoFcmAHjzm;
      string ZiHNJoSLqM;
      string VnGZdywneY;
      string AiRdEkAqIU;
      string GDOWtflUDt;
      string FYiiqKheyo;
      string DTEZmqRcJY;
      string naDVGRZjey;
      string LajkTazimt;
      string CVsTQmzoiD;
      string OJLeFdUMcY;
      string jxzAcbiSFM;
      string qhoOVShjES;
      string SkBbjCOFeo;
      if(LrlWWBGzVe == GDOWtflUDt){FmGOJFVrpo = true;}
      else if(GDOWtflUDt == LrlWWBGzVe){uiOGznaFCl = true;}
      if(fJzwYixyQj == FYiiqKheyo){DfiWMfTxIm = true;}
      else if(FYiiqKheyo == fJzwYixyQj){dnCoLqoPVg = true;}
      if(umHLaJCQts == DTEZmqRcJY){wkHJFzwXPa = true;}
      else if(DTEZmqRcJY == umHLaJCQts){quudjZbuip = true;}
      if(reBPxkINqD == naDVGRZjey){MdnQGDbaFQ = true;}
      else if(naDVGRZjey == reBPxkINqD){sLDCYNCBME = true;}
      if(ZFUuUNmNKU == LajkTazimt){xotPPlCePD = true;}
      else if(LajkTazimt == ZFUuUNmNKU){TbtjIjbbjQ = true;}
      if(OsiNjXkZuM == CVsTQmzoiD){hRwrsUewIJ = true;}
      else if(CVsTQmzoiD == OsiNjXkZuM){JwIHzznkVZ = true;}
      if(yoFcmAHjzm == OJLeFdUMcY){IxbObYqSPI = true;}
      else if(OJLeFdUMcY == yoFcmAHjzm){MfVMrRjAQx = true;}
      if(ZiHNJoSLqM == jxzAcbiSFM){EsqqoGqqsT = true;}
      if(VnGZdywneY == qhoOVShjES){xRIMsugwWl = true;}
      if(AiRdEkAqIU == SkBbjCOFeo){XeRwHzMenq = true;}
      while(jxzAcbiSFM == ZiHNJoSLqM){onCQMPBKUA = true;}
      while(qhoOVShjES == qhoOVShjES){xzKSfwCwDY = true;}
      while(SkBbjCOFeo == SkBbjCOFeo){ZQemrnGgqp = true;}
      if(FmGOJFVrpo == true){FmGOJFVrpo = false;}
      if(DfiWMfTxIm == true){DfiWMfTxIm = false;}
      if(wkHJFzwXPa == true){wkHJFzwXPa = false;}
      if(MdnQGDbaFQ == true){MdnQGDbaFQ = false;}
      if(xotPPlCePD == true){xotPPlCePD = false;}
      if(hRwrsUewIJ == true){hRwrsUewIJ = false;}
      if(IxbObYqSPI == true){IxbObYqSPI = false;}
      if(EsqqoGqqsT == true){EsqqoGqqsT = false;}
      if(xRIMsugwWl == true){xRIMsugwWl = false;}
      if(XeRwHzMenq == true){XeRwHzMenq = false;}
      if(uiOGznaFCl == true){uiOGznaFCl = false;}
      if(dnCoLqoPVg == true){dnCoLqoPVg = false;}
      if(quudjZbuip == true){quudjZbuip = false;}
      if(sLDCYNCBME == true){sLDCYNCBME = false;}
      if(TbtjIjbbjQ == true){TbtjIjbbjQ = false;}
      if(JwIHzznkVZ == true){JwIHzznkVZ = false;}
      if(MfVMrRjAQx == true){MfVMrRjAQx = false;}
      if(onCQMPBKUA == true){onCQMPBKUA = false;}
      if(xzKSfwCwDY == true){xzKSfwCwDY = false;}
      if(ZQemrnGgqp == true){ZQemrnGgqp = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NPJKNJWOSI
{ 
  void fBmClwhJFS()
  { 
      bool EZujUgUzjQ = false;
      bool aYyUqoUmEK = false;
      bool uaSVLoxZya = false;
      bool ZdmzelEEkZ = false;
      bool FoDiPuyHuT = false;
      bool LNSGmcGZYe = false;
      bool itAIqxlzoE = false;
      bool jQCdFFWiFP = false;
      bool qpeePMKgLB = false;
      bool fefFpKnurS = false;
      bool JySpitKnAh = false;
      bool GXTInsPVsd = false;
      bool lEmmXEkxnu = false;
      bool QbpmFrXnuU = false;
      bool BWfduILWRx = false;
      bool tSBOgEKeRe = false;
      bool JNAFQdggCw = false;
      bool VaZXcPYJrP = false;
      bool sNfpAyZbGB = false;
      bool grWQKmVaMQ = false;
      string BlKrTPHRkd;
      string OzYgyjsBjk;
      string YPTqhKoMRl;
      string WrIaigYyCy;
      string IiJxjCfdYT;
      string rPqBgkRTKI;
      string atzyGVpyxR;
      string KuzkbpGWGe;
      string eKmytHAGsx;
      string OFmsXOiKqN;
      string LhgHIwPGYq;
      string euMfcQGBEz;
      string ViFADFHFMN;
      string GufsSxdfox;
      string qthBRTsHQH;
      string HgGrrbcfED;
      string oKsKjjOTMf;
      string lZLGHaBDhi;
      string LAoFGZonAq;
      string UxblIgBWQh;
      if(BlKrTPHRkd == LhgHIwPGYq){EZujUgUzjQ = true;}
      else if(LhgHIwPGYq == BlKrTPHRkd){JySpitKnAh = true;}
      if(OzYgyjsBjk == euMfcQGBEz){aYyUqoUmEK = true;}
      else if(euMfcQGBEz == OzYgyjsBjk){GXTInsPVsd = true;}
      if(YPTqhKoMRl == ViFADFHFMN){uaSVLoxZya = true;}
      else if(ViFADFHFMN == YPTqhKoMRl){lEmmXEkxnu = true;}
      if(WrIaigYyCy == GufsSxdfox){ZdmzelEEkZ = true;}
      else if(GufsSxdfox == WrIaigYyCy){QbpmFrXnuU = true;}
      if(IiJxjCfdYT == qthBRTsHQH){FoDiPuyHuT = true;}
      else if(qthBRTsHQH == IiJxjCfdYT){BWfduILWRx = true;}
      if(rPqBgkRTKI == HgGrrbcfED){LNSGmcGZYe = true;}
      else if(HgGrrbcfED == rPqBgkRTKI){tSBOgEKeRe = true;}
      if(atzyGVpyxR == oKsKjjOTMf){itAIqxlzoE = true;}
      else if(oKsKjjOTMf == atzyGVpyxR){JNAFQdggCw = true;}
      if(KuzkbpGWGe == lZLGHaBDhi){jQCdFFWiFP = true;}
      if(eKmytHAGsx == LAoFGZonAq){qpeePMKgLB = true;}
      if(OFmsXOiKqN == UxblIgBWQh){fefFpKnurS = true;}
      while(lZLGHaBDhi == KuzkbpGWGe){VaZXcPYJrP = true;}
      while(LAoFGZonAq == LAoFGZonAq){sNfpAyZbGB = true;}
      while(UxblIgBWQh == UxblIgBWQh){grWQKmVaMQ = true;}
      if(EZujUgUzjQ == true){EZujUgUzjQ = false;}
      if(aYyUqoUmEK == true){aYyUqoUmEK = false;}
      if(uaSVLoxZya == true){uaSVLoxZya = false;}
      if(ZdmzelEEkZ == true){ZdmzelEEkZ = false;}
      if(FoDiPuyHuT == true){FoDiPuyHuT = false;}
      if(LNSGmcGZYe == true){LNSGmcGZYe = false;}
      if(itAIqxlzoE == true){itAIqxlzoE = false;}
      if(jQCdFFWiFP == true){jQCdFFWiFP = false;}
      if(qpeePMKgLB == true){qpeePMKgLB = false;}
      if(fefFpKnurS == true){fefFpKnurS = false;}
      if(JySpitKnAh == true){JySpitKnAh = false;}
      if(GXTInsPVsd == true){GXTInsPVsd = false;}
      if(lEmmXEkxnu == true){lEmmXEkxnu = false;}
      if(QbpmFrXnuU == true){QbpmFrXnuU = false;}
      if(BWfduILWRx == true){BWfduILWRx = false;}
      if(tSBOgEKeRe == true){tSBOgEKeRe = false;}
      if(JNAFQdggCw == true){JNAFQdggCw = false;}
      if(VaZXcPYJrP == true){VaZXcPYJrP = false;}
      if(sNfpAyZbGB == true){sNfpAyZbGB = false;}
      if(grWQKmVaMQ == true){grWQKmVaMQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OYXBSZQLHO
{ 
  void BpIdIfIHcq()
  { 
      bool TrpeeTJMXe = false;
      bool HffyPYOrLd = false;
      bool QqWpUJAbsV = false;
      bool SoBQwMXRKg = false;
      bool IeJlcujxxf = false;
      bool kfYVUbVAKU = false;
      bool ThSBLRdPBC = false;
      bool AxhdVlhjMZ = false;
      bool ZCdMklPfuT = false;
      bool qXHoNZMHZK = false;
      bool QTwuenytzi = false;
      bool sknjuREFQZ = false;
      bool CyUgLcorQw = false;
      bool YaJBIFFhgx = false;
      bool nVfLaFiWtu = false;
      bool VGpAISzOjC = false;
      bool yGcDJSblbp = false;
      bool LVYTlfRIGF = false;
      bool sSyUaSjIrp = false;
      bool FqyeygrcBH = false;
      string dTJcZXKIWS;
      string PoQfHzGgzH;
      string UMNSReTQuS;
      string mfsIYBBSbe;
      string RhGIGyoneF;
      string GpwSyRGpKu;
      string DsMDeiwIGF;
      string xmFUolFFMS;
      string pfkpLwiZJH;
      string uKHgaXVCcW;
      string CggZHSkbcM;
      string LBdqFXHWqh;
      string zVyyKleYsQ;
      string QCboQIyLhs;
      string LHwwusgQbD;
      string PyKtWfhiPU;
      string mLhMwIYnZw;
      string YwoyVrYCCQ;
      string FOxoARqgRj;
      string aPwNFzYjAl;
      if(dTJcZXKIWS == CggZHSkbcM){TrpeeTJMXe = true;}
      else if(CggZHSkbcM == dTJcZXKIWS){QTwuenytzi = true;}
      if(PoQfHzGgzH == LBdqFXHWqh){HffyPYOrLd = true;}
      else if(LBdqFXHWqh == PoQfHzGgzH){sknjuREFQZ = true;}
      if(UMNSReTQuS == zVyyKleYsQ){QqWpUJAbsV = true;}
      else if(zVyyKleYsQ == UMNSReTQuS){CyUgLcorQw = true;}
      if(mfsIYBBSbe == QCboQIyLhs){SoBQwMXRKg = true;}
      else if(QCboQIyLhs == mfsIYBBSbe){YaJBIFFhgx = true;}
      if(RhGIGyoneF == LHwwusgQbD){IeJlcujxxf = true;}
      else if(LHwwusgQbD == RhGIGyoneF){nVfLaFiWtu = true;}
      if(GpwSyRGpKu == PyKtWfhiPU){kfYVUbVAKU = true;}
      else if(PyKtWfhiPU == GpwSyRGpKu){VGpAISzOjC = true;}
      if(DsMDeiwIGF == mLhMwIYnZw){ThSBLRdPBC = true;}
      else if(mLhMwIYnZw == DsMDeiwIGF){yGcDJSblbp = true;}
      if(xmFUolFFMS == YwoyVrYCCQ){AxhdVlhjMZ = true;}
      if(pfkpLwiZJH == FOxoARqgRj){ZCdMklPfuT = true;}
      if(uKHgaXVCcW == aPwNFzYjAl){qXHoNZMHZK = true;}
      while(YwoyVrYCCQ == xmFUolFFMS){LVYTlfRIGF = true;}
      while(FOxoARqgRj == FOxoARqgRj){sSyUaSjIrp = true;}
      while(aPwNFzYjAl == aPwNFzYjAl){FqyeygrcBH = true;}
      if(TrpeeTJMXe == true){TrpeeTJMXe = false;}
      if(HffyPYOrLd == true){HffyPYOrLd = false;}
      if(QqWpUJAbsV == true){QqWpUJAbsV = false;}
      if(SoBQwMXRKg == true){SoBQwMXRKg = false;}
      if(IeJlcujxxf == true){IeJlcujxxf = false;}
      if(kfYVUbVAKU == true){kfYVUbVAKU = false;}
      if(ThSBLRdPBC == true){ThSBLRdPBC = false;}
      if(AxhdVlhjMZ == true){AxhdVlhjMZ = false;}
      if(ZCdMklPfuT == true){ZCdMklPfuT = false;}
      if(qXHoNZMHZK == true){qXHoNZMHZK = false;}
      if(QTwuenytzi == true){QTwuenytzi = false;}
      if(sknjuREFQZ == true){sknjuREFQZ = false;}
      if(CyUgLcorQw == true){CyUgLcorQw = false;}
      if(YaJBIFFhgx == true){YaJBIFFhgx = false;}
      if(nVfLaFiWtu == true){nVfLaFiWtu = false;}
      if(VGpAISzOjC == true){VGpAISzOjC = false;}
      if(yGcDJSblbp == true){yGcDJSblbp = false;}
      if(LVYTlfRIGF == true){LVYTlfRIGF = false;}
      if(sSyUaSjIrp == true){sSyUaSjIrp = false;}
      if(FqyeygrcBH == true){FqyeygrcBH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CDLZXWQUHX
{ 
  void DNTEVjNqKi()
  { 
      bool rTzCVoVZdJ = false;
      bool XECkXplPFB = false;
      bool mzfnCUcsLd = false;
      bool hkjWqUhwuF = false;
      bool NWceZzwaFW = false;
      bool QOTIfsnKAA = false;
      bool LcOzalTesC = false;
      bool HpUznPWBOC = false;
      bool YCmnMTRbqy = false;
      bool gWwtskIcDm = false;
      bool eQgCPatTUP = false;
      bool WmwybJoZmH = false;
      bool UrnuVjtZGC = false;
      bool HsxaWIUkYl = false;
      bool YxHabtaxSj = false;
      bool whpwRmblfe = false;
      bool gUNRHcoUaT = false;
      bool nDBAhXjNSb = false;
      bool IwGWGYCBIi = false;
      bool ghEsFEjpSy = false;
      string ZRaMXPDRXL;
      string bjymEGqqmj;
      string ksqkxYgwVn;
      string DyADOxJTQg;
      string ICqpbAiIKn;
      string NTwBugZoPy;
      string hHuiHciuYG;
      string xXZGaiCBJx;
      string pYkhajCzHg;
      string XTERyBGHiy;
      string RmiEpptoZM;
      string eYfRIJWNIX;
      string ainqicSHHZ;
      string buSLfCogqj;
      string oMYtCynlIo;
      string eryLWhyqUN;
      string WlhAkjGTlG;
      string ToWYhNJtoj;
      string ogzRuiwGnx;
      string EyLdMDeYJS;
      if(ZRaMXPDRXL == RmiEpptoZM){rTzCVoVZdJ = true;}
      else if(RmiEpptoZM == ZRaMXPDRXL){eQgCPatTUP = true;}
      if(bjymEGqqmj == eYfRIJWNIX){XECkXplPFB = true;}
      else if(eYfRIJWNIX == bjymEGqqmj){WmwybJoZmH = true;}
      if(ksqkxYgwVn == ainqicSHHZ){mzfnCUcsLd = true;}
      else if(ainqicSHHZ == ksqkxYgwVn){UrnuVjtZGC = true;}
      if(DyADOxJTQg == buSLfCogqj){hkjWqUhwuF = true;}
      else if(buSLfCogqj == DyADOxJTQg){HsxaWIUkYl = true;}
      if(ICqpbAiIKn == oMYtCynlIo){NWceZzwaFW = true;}
      else if(oMYtCynlIo == ICqpbAiIKn){YxHabtaxSj = true;}
      if(NTwBugZoPy == eryLWhyqUN){QOTIfsnKAA = true;}
      else if(eryLWhyqUN == NTwBugZoPy){whpwRmblfe = true;}
      if(hHuiHciuYG == WlhAkjGTlG){LcOzalTesC = true;}
      else if(WlhAkjGTlG == hHuiHciuYG){gUNRHcoUaT = true;}
      if(xXZGaiCBJx == ToWYhNJtoj){HpUznPWBOC = true;}
      if(pYkhajCzHg == ogzRuiwGnx){YCmnMTRbqy = true;}
      if(XTERyBGHiy == EyLdMDeYJS){gWwtskIcDm = true;}
      while(ToWYhNJtoj == xXZGaiCBJx){nDBAhXjNSb = true;}
      while(ogzRuiwGnx == ogzRuiwGnx){IwGWGYCBIi = true;}
      while(EyLdMDeYJS == EyLdMDeYJS){ghEsFEjpSy = true;}
      if(rTzCVoVZdJ == true){rTzCVoVZdJ = false;}
      if(XECkXplPFB == true){XECkXplPFB = false;}
      if(mzfnCUcsLd == true){mzfnCUcsLd = false;}
      if(hkjWqUhwuF == true){hkjWqUhwuF = false;}
      if(NWceZzwaFW == true){NWceZzwaFW = false;}
      if(QOTIfsnKAA == true){QOTIfsnKAA = false;}
      if(LcOzalTesC == true){LcOzalTesC = false;}
      if(HpUznPWBOC == true){HpUznPWBOC = false;}
      if(YCmnMTRbqy == true){YCmnMTRbqy = false;}
      if(gWwtskIcDm == true){gWwtskIcDm = false;}
      if(eQgCPatTUP == true){eQgCPatTUP = false;}
      if(WmwybJoZmH == true){WmwybJoZmH = false;}
      if(UrnuVjtZGC == true){UrnuVjtZGC = false;}
      if(HsxaWIUkYl == true){HsxaWIUkYl = false;}
      if(YxHabtaxSj == true){YxHabtaxSj = false;}
      if(whpwRmblfe == true){whpwRmblfe = false;}
      if(gUNRHcoUaT == true){gUNRHcoUaT = false;}
      if(nDBAhXjNSb == true){nDBAhXjNSb = false;}
      if(IwGWGYCBIi == true){IwGWGYCBIi = false;}
      if(ghEsFEjpSy == true){ghEsFEjpSy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WZVKRKSVOR
{ 
  void VpfmURMGfY()
  { 
      bool kanYxZEPSm = false;
      bool ZkrdYYDKdX = false;
      bool AuRjhmYeao = false;
      bool lyIDZDJmjs = false;
      bool xIMWHlXudY = false;
      bool SGFIdoIDqJ = false;
      bool BYocBbbnEV = false;
      bool OPajZqiubu = false;
      bool PcBBWerzYj = false;
      bool tFwarYTjUf = false;
      bool JipIJDfVhl = false;
      bool FNausXXmGV = false;
      bool jcPTaJSArB = false;
      bool XzZwkglrrT = false;
      bool zVQldgdAgU = false;
      bool AgczwgSVhA = false;
      bool jfdgOGneJM = false;
      bool keumWNLcpt = false;
      bool eSSWBpVaAE = false;
      bool NQKqiSrjWX = false;
      string oWlqhhrPYJ;
      string fcYslVCXOE;
      string WLBFVMmEkm;
      string UiqESSKjBn;
      string ZGSQAIrlet;
      string rbAUgdVgVV;
      string tmelowkUZT;
      string KSVaeQrkEM;
      string UBZptriWXo;
      string xdOeslafaE;
      string WWLLjZGYoP;
      string kkbIBsNnrx;
      string NOxkWfDpgr;
      string oMJApxpOGT;
      string lJgHDLCGdA;
      string YmndpIZZnD;
      string VPrxaYaaRE;
      string DyhXLDhohK;
      string dxMgTCYeJh;
      string WPZCxVRjge;
      if(oWlqhhrPYJ == WWLLjZGYoP){kanYxZEPSm = true;}
      else if(WWLLjZGYoP == oWlqhhrPYJ){JipIJDfVhl = true;}
      if(fcYslVCXOE == kkbIBsNnrx){ZkrdYYDKdX = true;}
      else if(kkbIBsNnrx == fcYslVCXOE){FNausXXmGV = true;}
      if(WLBFVMmEkm == NOxkWfDpgr){AuRjhmYeao = true;}
      else if(NOxkWfDpgr == WLBFVMmEkm){jcPTaJSArB = true;}
      if(UiqESSKjBn == oMJApxpOGT){lyIDZDJmjs = true;}
      else if(oMJApxpOGT == UiqESSKjBn){XzZwkglrrT = true;}
      if(ZGSQAIrlet == lJgHDLCGdA){xIMWHlXudY = true;}
      else if(lJgHDLCGdA == ZGSQAIrlet){zVQldgdAgU = true;}
      if(rbAUgdVgVV == YmndpIZZnD){SGFIdoIDqJ = true;}
      else if(YmndpIZZnD == rbAUgdVgVV){AgczwgSVhA = true;}
      if(tmelowkUZT == VPrxaYaaRE){BYocBbbnEV = true;}
      else if(VPrxaYaaRE == tmelowkUZT){jfdgOGneJM = true;}
      if(KSVaeQrkEM == DyhXLDhohK){OPajZqiubu = true;}
      if(UBZptriWXo == dxMgTCYeJh){PcBBWerzYj = true;}
      if(xdOeslafaE == WPZCxVRjge){tFwarYTjUf = true;}
      while(DyhXLDhohK == KSVaeQrkEM){keumWNLcpt = true;}
      while(dxMgTCYeJh == dxMgTCYeJh){eSSWBpVaAE = true;}
      while(WPZCxVRjge == WPZCxVRjge){NQKqiSrjWX = true;}
      if(kanYxZEPSm == true){kanYxZEPSm = false;}
      if(ZkrdYYDKdX == true){ZkrdYYDKdX = false;}
      if(AuRjhmYeao == true){AuRjhmYeao = false;}
      if(lyIDZDJmjs == true){lyIDZDJmjs = false;}
      if(xIMWHlXudY == true){xIMWHlXudY = false;}
      if(SGFIdoIDqJ == true){SGFIdoIDqJ = false;}
      if(BYocBbbnEV == true){BYocBbbnEV = false;}
      if(OPajZqiubu == true){OPajZqiubu = false;}
      if(PcBBWerzYj == true){PcBBWerzYj = false;}
      if(tFwarYTjUf == true){tFwarYTjUf = false;}
      if(JipIJDfVhl == true){JipIJDfVhl = false;}
      if(FNausXXmGV == true){FNausXXmGV = false;}
      if(jcPTaJSArB == true){jcPTaJSArB = false;}
      if(XzZwkglrrT == true){XzZwkglrrT = false;}
      if(zVQldgdAgU == true){zVQldgdAgU = false;}
      if(AgczwgSVhA == true){AgczwgSVhA = false;}
      if(jfdgOGneJM == true){jfdgOGneJM = false;}
      if(keumWNLcpt == true){keumWNLcpt = false;}
      if(eSSWBpVaAE == true){eSSWBpVaAE = false;}
      if(NQKqiSrjWX == true){NQKqiSrjWX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BUWSWZBWXB
{ 
  void uStjsdBtGR()
  { 
      bool KQxWFGWDyS = false;
      bool nZmNDWjASg = false;
      bool KFBLarwVeu = false;
      bool RBIZXtsHRp = false;
      bool PADbQeiMIz = false;
      bool doIGDEOluM = false;
      bool JBXCBmRMoJ = false;
      bool cchcRJCZtu = false;
      bool JjMfQbrCWS = false;
      bool fHLEgNcAZd = false;
      bool NVWzVBHfns = false;
      bool CnqADZnqlZ = false;
      bool tPpWcVRcwD = false;
      bool gBAQhmMNJP = false;
      bool UMNcQqPLBG = false;
      bool ZkOAjxfAVz = false;
      bool jdjjMAnaRQ = false;
      bool NagWFhCMFl = false;
      bool GGbodulzfk = false;
      bool GMqKeqccPP = false;
      string LflyNzLhUf;
      string HdJprQCYGc;
      string wlKECBxAId;
      string TMXlZGMwai;
      string gojGosyHhr;
      string YtlnSZVgwM;
      string CKZqLuFXQk;
      string EFxXwGBEPJ;
      string CoRPEZajTA;
      string jeAOgVOrtt;
      string maqwnTxNqL;
      string XRRlJMrJml;
      string TVDPBYFiqa;
      string qJJUVbxVbk;
      string cMrHbSNNET;
      string cFFHZRWauP;
      string OOWQTOHwPg;
      string OWhCJHcwnh;
      string LqdWxlEBtE;
      string FWZMWOpbnN;
      if(LflyNzLhUf == maqwnTxNqL){KQxWFGWDyS = true;}
      else if(maqwnTxNqL == LflyNzLhUf){NVWzVBHfns = true;}
      if(HdJprQCYGc == XRRlJMrJml){nZmNDWjASg = true;}
      else if(XRRlJMrJml == HdJprQCYGc){CnqADZnqlZ = true;}
      if(wlKECBxAId == TVDPBYFiqa){KFBLarwVeu = true;}
      else if(TVDPBYFiqa == wlKECBxAId){tPpWcVRcwD = true;}
      if(TMXlZGMwai == qJJUVbxVbk){RBIZXtsHRp = true;}
      else if(qJJUVbxVbk == TMXlZGMwai){gBAQhmMNJP = true;}
      if(gojGosyHhr == cMrHbSNNET){PADbQeiMIz = true;}
      else if(cMrHbSNNET == gojGosyHhr){UMNcQqPLBG = true;}
      if(YtlnSZVgwM == cFFHZRWauP){doIGDEOluM = true;}
      else if(cFFHZRWauP == YtlnSZVgwM){ZkOAjxfAVz = true;}
      if(CKZqLuFXQk == OOWQTOHwPg){JBXCBmRMoJ = true;}
      else if(OOWQTOHwPg == CKZqLuFXQk){jdjjMAnaRQ = true;}
      if(EFxXwGBEPJ == OWhCJHcwnh){cchcRJCZtu = true;}
      if(CoRPEZajTA == LqdWxlEBtE){JjMfQbrCWS = true;}
      if(jeAOgVOrtt == FWZMWOpbnN){fHLEgNcAZd = true;}
      while(OWhCJHcwnh == EFxXwGBEPJ){NagWFhCMFl = true;}
      while(LqdWxlEBtE == LqdWxlEBtE){GGbodulzfk = true;}
      while(FWZMWOpbnN == FWZMWOpbnN){GMqKeqccPP = true;}
      if(KQxWFGWDyS == true){KQxWFGWDyS = false;}
      if(nZmNDWjASg == true){nZmNDWjASg = false;}
      if(KFBLarwVeu == true){KFBLarwVeu = false;}
      if(RBIZXtsHRp == true){RBIZXtsHRp = false;}
      if(PADbQeiMIz == true){PADbQeiMIz = false;}
      if(doIGDEOluM == true){doIGDEOluM = false;}
      if(JBXCBmRMoJ == true){JBXCBmRMoJ = false;}
      if(cchcRJCZtu == true){cchcRJCZtu = false;}
      if(JjMfQbrCWS == true){JjMfQbrCWS = false;}
      if(fHLEgNcAZd == true){fHLEgNcAZd = false;}
      if(NVWzVBHfns == true){NVWzVBHfns = false;}
      if(CnqADZnqlZ == true){CnqADZnqlZ = false;}
      if(tPpWcVRcwD == true){tPpWcVRcwD = false;}
      if(gBAQhmMNJP == true){gBAQhmMNJP = false;}
      if(UMNcQqPLBG == true){UMNcQqPLBG = false;}
      if(ZkOAjxfAVz == true){ZkOAjxfAVz = false;}
      if(jdjjMAnaRQ == true){jdjjMAnaRQ = false;}
      if(NagWFhCMFl == true){NagWFhCMFl = false;}
      if(GGbodulzfk == true){GGbodulzfk = false;}
      if(GMqKeqccPP == true){GMqKeqccPP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TNFBSYGVHW
{ 
  void MDzIzLXbzR()
  { 
      bool QAtzPltJyC = false;
      bool gnApINnDyQ = false;
      bool kHiwWFIxyi = false;
      bool qGmaWkAnjL = false;
      bool cxNQzQLhOc = false;
      bool tPumMJQxbJ = false;
      bool dxKPLaBZTW = false;
      bool MzSmnntSxF = false;
      bool tZiVobKzNN = false;
      bool UDMZhflyFY = false;
      bool PkOqPtxIyY = false;
      bool PxAVPDNoZj = false;
      bool QBLGCJmrFI = false;
      bool BuNYpILrFt = false;
      bool dBAoxWDpKd = false;
      bool RwXVHNZInG = false;
      bool HFDpWTjafG = false;
      bool rVifFOBVIf = false;
      bool GwgbpEXrQb = false;
      bool iDDzlYyKSd = false;
      string czdrnGHtph;
      string OkqgwZbOuR;
      string cnmQMXXOIe;
      string DVnlcrzuHW;
      string wnxUnXVtHI;
      string GALiQnMMfP;
      string irlLdrxFDE;
      string agHXUoqRcn;
      string BcHzXCKFUn;
      string elOqnEClpu;
      string ZQpDhZrmNb;
      string xtjqOFklma;
      string hrjPYnRQiQ;
      string mFagOkmDQy;
      string JWqspjCWbi;
      string SPyPNhfZet;
      string HbDwnrTCcc;
      string SgUoQKAqYn;
      string ktnAxWGLSf;
      string FMYWphStEy;
      if(czdrnGHtph == ZQpDhZrmNb){QAtzPltJyC = true;}
      else if(ZQpDhZrmNb == czdrnGHtph){PkOqPtxIyY = true;}
      if(OkqgwZbOuR == xtjqOFklma){gnApINnDyQ = true;}
      else if(xtjqOFklma == OkqgwZbOuR){PxAVPDNoZj = true;}
      if(cnmQMXXOIe == hrjPYnRQiQ){kHiwWFIxyi = true;}
      else if(hrjPYnRQiQ == cnmQMXXOIe){QBLGCJmrFI = true;}
      if(DVnlcrzuHW == mFagOkmDQy){qGmaWkAnjL = true;}
      else if(mFagOkmDQy == DVnlcrzuHW){BuNYpILrFt = true;}
      if(wnxUnXVtHI == JWqspjCWbi){cxNQzQLhOc = true;}
      else if(JWqspjCWbi == wnxUnXVtHI){dBAoxWDpKd = true;}
      if(GALiQnMMfP == SPyPNhfZet){tPumMJQxbJ = true;}
      else if(SPyPNhfZet == GALiQnMMfP){RwXVHNZInG = true;}
      if(irlLdrxFDE == HbDwnrTCcc){dxKPLaBZTW = true;}
      else if(HbDwnrTCcc == irlLdrxFDE){HFDpWTjafG = true;}
      if(agHXUoqRcn == SgUoQKAqYn){MzSmnntSxF = true;}
      if(BcHzXCKFUn == ktnAxWGLSf){tZiVobKzNN = true;}
      if(elOqnEClpu == FMYWphStEy){UDMZhflyFY = true;}
      while(SgUoQKAqYn == agHXUoqRcn){rVifFOBVIf = true;}
      while(ktnAxWGLSf == ktnAxWGLSf){GwgbpEXrQb = true;}
      while(FMYWphStEy == FMYWphStEy){iDDzlYyKSd = true;}
      if(QAtzPltJyC == true){QAtzPltJyC = false;}
      if(gnApINnDyQ == true){gnApINnDyQ = false;}
      if(kHiwWFIxyi == true){kHiwWFIxyi = false;}
      if(qGmaWkAnjL == true){qGmaWkAnjL = false;}
      if(cxNQzQLhOc == true){cxNQzQLhOc = false;}
      if(tPumMJQxbJ == true){tPumMJQxbJ = false;}
      if(dxKPLaBZTW == true){dxKPLaBZTW = false;}
      if(MzSmnntSxF == true){MzSmnntSxF = false;}
      if(tZiVobKzNN == true){tZiVobKzNN = false;}
      if(UDMZhflyFY == true){UDMZhflyFY = false;}
      if(PkOqPtxIyY == true){PkOqPtxIyY = false;}
      if(PxAVPDNoZj == true){PxAVPDNoZj = false;}
      if(QBLGCJmrFI == true){QBLGCJmrFI = false;}
      if(BuNYpILrFt == true){BuNYpILrFt = false;}
      if(dBAoxWDpKd == true){dBAoxWDpKd = false;}
      if(RwXVHNZInG == true){RwXVHNZInG = false;}
      if(HFDpWTjafG == true){HFDpWTjafG = false;}
      if(rVifFOBVIf == true){rVifFOBVIf = false;}
      if(GwgbpEXrQb == true){GwgbpEXrQb = false;}
      if(iDDzlYyKSd == true){iDDzlYyKSd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UVKDJIWQQX
{ 
  void mUToTxAwHU()
  { 
      bool ieTBhjYDck = false;
      bool HqNEWnKjOy = false;
      bool VaJMegZcMB = false;
      bool fRZwtXFfnq = false;
      bool mzQbenLhQB = false;
      bool jFiwmHUGVA = false;
      bool JwpaEpQRJz = false;
      bool idwWKWKhgq = false;
      bool VrLLRahcJm = false;
      bool CyJmgqJoWo = false;
      bool WsXRRFJMlQ = false;
      bool xmZeeRjdJd = false;
      bool ImIWwkTIpS = false;
      bool MULrTbTssL = false;
      bool QVqsoPXeIP = false;
      bool GeesfijBVg = false;
      bool lAWTDbagKC = false;
      bool xpkyMlwPgf = false;
      bool MnAZLhpSLk = false;
      bool qSXMZQzXoN = false;
      string GzUQoEqCDb;
      string AVyWcTADjw;
      string WBmoSIGZZm;
      string UYQFUDWctl;
      string uyeNyfeHod;
      string cwOOlEMexf;
      string yRbQGbEsCY;
      string xWJyBYWYpl;
      string dykwJYKBSP;
      string qqeOncVNGW;
      string alFPgFlPNN;
      string hmoLCzgfHh;
      string QDZlhFrtKX;
      string CJRSLqrUOD;
      string lunFgDCTMn;
      string wpGYGXLSha;
      string NrFpKYUWzK;
      string GMQDuVPUEd;
      string DYhVaMCsMm;
      string eZlnptoytI;
      if(GzUQoEqCDb == alFPgFlPNN){ieTBhjYDck = true;}
      else if(alFPgFlPNN == GzUQoEqCDb){WsXRRFJMlQ = true;}
      if(AVyWcTADjw == hmoLCzgfHh){HqNEWnKjOy = true;}
      else if(hmoLCzgfHh == AVyWcTADjw){xmZeeRjdJd = true;}
      if(WBmoSIGZZm == QDZlhFrtKX){VaJMegZcMB = true;}
      else if(QDZlhFrtKX == WBmoSIGZZm){ImIWwkTIpS = true;}
      if(UYQFUDWctl == CJRSLqrUOD){fRZwtXFfnq = true;}
      else if(CJRSLqrUOD == UYQFUDWctl){MULrTbTssL = true;}
      if(uyeNyfeHod == lunFgDCTMn){mzQbenLhQB = true;}
      else if(lunFgDCTMn == uyeNyfeHod){QVqsoPXeIP = true;}
      if(cwOOlEMexf == wpGYGXLSha){jFiwmHUGVA = true;}
      else if(wpGYGXLSha == cwOOlEMexf){GeesfijBVg = true;}
      if(yRbQGbEsCY == NrFpKYUWzK){JwpaEpQRJz = true;}
      else if(NrFpKYUWzK == yRbQGbEsCY){lAWTDbagKC = true;}
      if(xWJyBYWYpl == GMQDuVPUEd){idwWKWKhgq = true;}
      if(dykwJYKBSP == DYhVaMCsMm){VrLLRahcJm = true;}
      if(qqeOncVNGW == eZlnptoytI){CyJmgqJoWo = true;}
      while(GMQDuVPUEd == xWJyBYWYpl){xpkyMlwPgf = true;}
      while(DYhVaMCsMm == DYhVaMCsMm){MnAZLhpSLk = true;}
      while(eZlnptoytI == eZlnptoytI){qSXMZQzXoN = true;}
      if(ieTBhjYDck == true){ieTBhjYDck = false;}
      if(HqNEWnKjOy == true){HqNEWnKjOy = false;}
      if(VaJMegZcMB == true){VaJMegZcMB = false;}
      if(fRZwtXFfnq == true){fRZwtXFfnq = false;}
      if(mzQbenLhQB == true){mzQbenLhQB = false;}
      if(jFiwmHUGVA == true){jFiwmHUGVA = false;}
      if(JwpaEpQRJz == true){JwpaEpQRJz = false;}
      if(idwWKWKhgq == true){idwWKWKhgq = false;}
      if(VrLLRahcJm == true){VrLLRahcJm = false;}
      if(CyJmgqJoWo == true){CyJmgqJoWo = false;}
      if(WsXRRFJMlQ == true){WsXRRFJMlQ = false;}
      if(xmZeeRjdJd == true){xmZeeRjdJd = false;}
      if(ImIWwkTIpS == true){ImIWwkTIpS = false;}
      if(MULrTbTssL == true){MULrTbTssL = false;}
      if(QVqsoPXeIP == true){QVqsoPXeIP = false;}
      if(GeesfijBVg == true){GeesfijBVg = false;}
      if(lAWTDbagKC == true){lAWTDbagKC = false;}
      if(xpkyMlwPgf == true){xpkyMlwPgf = false;}
      if(MnAZLhpSLk == true){MnAZLhpSLk = false;}
      if(qSXMZQzXoN == true){qSXMZQzXoN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PKNFSUNQIM
{ 
  void QzIDftczJF()
  { 
      bool LXRVRBmQkJ = false;
      bool ubtDzLOxOi = false;
      bool BwqSLKmJTY = false;
      bool qXtBtQVhjp = false;
      bool GPsUIyGJLX = false;
      bool TkbkROgyKG = false;
      bool OdxMRWhsJi = false;
      bool pgNYblDVKN = false;
      bool DdyuEGuIje = false;
      bool jWWcbcRfDk = false;
      bool PNzchmXDIe = false;
      bool LLnwDjeBIK = false;
      bool bYJeySJjxy = false;
      bool UWRZyodVMR = false;
      bool irzESXiaZq = false;
      bool yPXrJpEUdP = false;
      bool cmFJKosOyN = false;
      bool GJopbiZEMZ = false;
      bool jiKGAEyZPH = false;
      bool kJqxnulptc = false;
      string BjIZxlzBYU;
      string zQqmmfkIcg;
      string EJMSJWCEIe;
      string gdJwdITmkp;
      string RWBnMaAxgk;
      string WBQOPqWoQZ;
      string itstmUGuJe;
      string MxWlPgIXFh;
      string WFXXHeMugl;
      string ldWLHWdGAr;
      string UFnCXYdeyh;
      string iyjiDdVUlI;
      string ILNXsiaLer;
      string DBWNMRRVAh;
      string BhwzCmtmHc;
      string BIUriJqPJo;
      string ySzFdhaVVl;
      string LzUDHZOOVF;
      string omDBqtfedD;
      string ZBMKjhipUT;
      if(BjIZxlzBYU == UFnCXYdeyh){LXRVRBmQkJ = true;}
      else if(UFnCXYdeyh == BjIZxlzBYU){PNzchmXDIe = true;}
      if(zQqmmfkIcg == iyjiDdVUlI){ubtDzLOxOi = true;}
      else if(iyjiDdVUlI == zQqmmfkIcg){LLnwDjeBIK = true;}
      if(EJMSJWCEIe == ILNXsiaLer){BwqSLKmJTY = true;}
      else if(ILNXsiaLer == EJMSJWCEIe){bYJeySJjxy = true;}
      if(gdJwdITmkp == DBWNMRRVAh){qXtBtQVhjp = true;}
      else if(DBWNMRRVAh == gdJwdITmkp){UWRZyodVMR = true;}
      if(RWBnMaAxgk == BhwzCmtmHc){GPsUIyGJLX = true;}
      else if(BhwzCmtmHc == RWBnMaAxgk){irzESXiaZq = true;}
      if(WBQOPqWoQZ == BIUriJqPJo){TkbkROgyKG = true;}
      else if(BIUriJqPJo == WBQOPqWoQZ){yPXrJpEUdP = true;}
      if(itstmUGuJe == ySzFdhaVVl){OdxMRWhsJi = true;}
      else if(ySzFdhaVVl == itstmUGuJe){cmFJKosOyN = true;}
      if(MxWlPgIXFh == LzUDHZOOVF){pgNYblDVKN = true;}
      if(WFXXHeMugl == omDBqtfedD){DdyuEGuIje = true;}
      if(ldWLHWdGAr == ZBMKjhipUT){jWWcbcRfDk = true;}
      while(LzUDHZOOVF == MxWlPgIXFh){GJopbiZEMZ = true;}
      while(omDBqtfedD == omDBqtfedD){jiKGAEyZPH = true;}
      while(ZBMKjhipUT == ZBMKjhipUT){kJqxnulptc = true;}
      if(LXRVRBmQkJ == true){LXRVRBmQkJ = false;}
      if(ubtDzLOxOi == true){ubtDzLOxOi = false;}
      if(BwqSLKmJTY == true){BwqSLKmJTY = false;}
      if(qXtBtQVhjp == true){qXtBtQVhjp = false;}
      if(GPsUIyGJLX == true){GPsUIyGJLX = false;}
      if(TkbkROgyKG == true){TkbkROgyKG = false;}
      if(OdxMRWhsJi == true){OdxMRWhsJi = false;}
      if(pgNYblDVKN == true){pgNYblDVKN = false;}
      if(DdyuEGuIje == true){DdyuEGuIje = false;}
      if(jWWcbcRfDk == true){jWWcbcRfDk = false;}
      if(PNzchmXDIe == true){PNzchmXDIe = false;}
      if(LLnwDjeBIK == true){LLnwDjeBIK = false;}
      if(bYJeySJjxy == true){bYJeySJjxy = false;}
      if(UWRZyodVMR == true){UWRZyodVMR = false;}
      if(irzESXiaZq == true){irzESXiaZq = false;}
      if(yPXrJpEUdP == true){yPXrJpEUdP = false;}
      if(cmFJKosOyN == true){cmFJKosOyN = false;}
      if(GJopbiZEMZ == true){GJopbiZEMZ = false;}
      if(jiKGAEyZPH == true){jiKGAEyZPH = false;}
      if(kJqxnulptc == true){kJqxnulptc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JOAEYQMLFY
{ 
  void wFOHDRXGCX()
  { 
      bool RKnyuKWZPh = false;
      bool INRtlksbGF = false;
      bool PBCoxuTaPE = false;
      bool BkHbXZWRVH = false;
      bool qkgFWojEDK = false;
      bool qPccrQIzAz = false;
      bool quKLfXFpWM = false;
      bool ObFyNeTwRH = false;
      bool KPWEMkHdjH = false;
      bool RWQVQwZVYz = false;
      bool jthquPKeOM = false;
      bool wLgsoaXFyu = false;
      bool PaFOtqKigR = false;
      bool EyXSXPhfJw = false;
      bool acOWKGpXgp = false;
      bool XUXdiIoSwE = false;
      bool FaxxIymNVY = false;
      bool sqwBTlMiEn = false;
      bool AmCqtjZtjO = false;
      bool aOjFqAfROL = false;
      string FrbWVTaUqV;
      string XoZEpYmlps;
      string nqtzcLjtTe;
      string PCgLXQhfQh;
      string NSCycNJERy;
      string WuMBoNybsJ;
      string ifAMGUHNfZ;
      string qGVIeUnuWO;
      string CecUHUoeVK;
      string SFmGyhSprH;
      string GuYrHPYEWU;
      string ZCFdHEDeEl;
      string ecqVGkTOKx;
      string BxDPIGGjNr;
      string ukIBjQAIzs;
      string dnlGbcDDkW;
      string wJsYSbypUA;
      string ySRLpsTpkp;
      string jKPQSNLxmf;
      string DCzpwkrihI;
      if(FrbWVTaUqV == GuYrHPYEWU){RKnyuKWZPh = true;}
      else if(GuYrHPYEWU == FrbWVTaUqV){jthquPKeOM = true;}
      if(XoZEpYmlps == ZCFdHEDeEl){INRtlksbGF = true;}
      else if(ZCFdHEDeEl == XoZEpYmlps){wLgsoaXFyu = true;}
      if(nqtzcLjtTe == ecqVGkTOKx){PBCoxuTaPE = true;}
      else if(ecqVGkTOKx == nqtzcLjtTe){PaFOtqKigR = true;}
      if(PCgLXQhfQh == BxDPIGGjNr){BkHbXZWRVH = true;}
      else if(BxDPIGGjNr == PCgLXQhfQh){EyXSXPhfJw = true;}
      if(NSCycNJERy == ukIBjQAIzs){qkgFWojEDK = true;}
      else if(ukIBjQAIzs == NSCycNJERy){acOWKGpXgp = true;}
      if(WuMBoNybsJ == dnlGbcDDkW){qPccrQIzAz = true;}
      else if(dnlGbcDDkW == WuMBoNybsJ){XUXdiIoSwE = true;}
      if(ifAMGUHNfZ == wJsYSbypUA){quKLfXFpWM = true;}
      else if(wJsYSbypUA == ifAMGUHNfZ){FaxxIymNVY = true;}
      if(qGVIeUnuWO == ySRLpsTpkp){ObFyNeTwRH = true;}
      if(CecUHUoeVK == jKPQSNLxmf){KPWEMkHdjH = true;}
      if(SFmGyhSprH == DCzpwkrihI){RWQVQwZVYz = true;}
      while(ySRLpsTpkp == qGVIeUnuWO){sqwBTlMiEn = true;}
      while(jKPQSNLxmf == jKPQSNLxmf){AmCqtjZtjO = true;}
      while(DCzpwkrihI == DCzpwkrihI){aOjFqAfROL = true;}
      if(RKnyuKWZPh == true){RKnyuKWZPh = false;}
      if(INRtlksbGF == true){INRtlksbGF = false;}
      if(PBCoxuTaPE == true){PBCoxuTaPE = false;}
      if(BkHbXZWRVH == true){BkHbXZWRVH = false;}
      if(qkgFWojEDK == true){qkgFWojEDK = false;}
      if(qPccrQIzAz == true){qPccrQIzAz = false;}
      if(quKLfXFpWM == true){quKLfXFpWM = false;}
      if(ObFyNeTwRH == true){ObFyNeTwRH = false;}
      if(KPWEMkHdjH == true){KPWEMkHdjH = false;}
      if(RWQVQwZVYz == true){RWQVQwZVYz = false;}
      if(jthquPKeOM == true){jthquPKeOM = false;}
      if(wLgsoaXFyu == true){wLgsoaXFyu = false;}
      if(PaFOtqKigR == true){PaFOtqKigR = false;}
      if(EyXSXPhfJw == true){EyXSXPhfJw = false;}
      if(acOWKGpXgp == true){acOWKGpXgp = false;}
      if(XUXdiIoSwE == true){XUXdiIoSwE = false;}
      if(FaxxIymNVY == true){FaxxIymNVY = false;}
      if(sqwBTlMiEn == true){sqwBTlMiEn = false;}
      if(AmCqtjZtjO == true){AmCqtjZtjO = false;}
      if(aOjFqAfROL == true){aOjFqAfROL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JHTRTBJXBN
{ 
  void MtdiqLwHqY()
  { 
      bool FpsLQKOewU = false;
      bool RqRiZPsshg = false;
      bool lVlYqflVtP = false;
      bool UbrNlntToa = false;
      bool bpbeVYYGls = false;
      bool oXPMZjDUUk = false;
      bool YrjgzpyExl = false;
      bool UZDHEutGCR = false;
      bool GfjBaxMfmB = false;
      bool jZzECiGuee = false;
      bool ZTIAXPTHHd = false;
      bool oxRCdrIhFC = false;
      bool dSRuCtZyFV = false;
      bool nybXrlWjcL = false;
      bool dEYILmIwZJ = false;
      bool aXocAGCNyd = false;
      bool oUCkpqUtKq = false;
      bool VXEtYZtIiO = false;
      bool lcXutQIzWk = false;
      bool jJlzVIBmSU = false;
      string hJuMAqphaQ;
      string MTBIOyBFHS;
      string IWUnEeTLHB;
      string ZxOXnFZHnR;
      string LWWuXnHCbg;
      string qqkBwKoHIV;
      string WdZZqOEcUI;
      string XsRpHkAYPP;
      string WQjZJhMMbc;
      string QSzRthxxEJ;
      string jdhSJyYutK;
      string KPqszFszPI;
      string CrBqrFtzuZ;
      string nDUGfrhQQL;
      string JQSQSAaGSn;
      string nxhwGrbqaE;
      string DZAUWHukMG;
      string XciDzlZkYq;
      string dnfGlpUYzn;
      string PqpMIiQxsL;
      if(hJuMAqphaQ == jdhSJyYutK){FpsLQKOewU = true;}
      else if(jdhSJyYutK == hJuMAqphaQ){ZTIAXPTHHd = true;}
      if(MTBIOyBFHS == KPqszFszPI){RqRiZPsshg = true;}
      else if(KPqszFszPI == MTBIOyBFHS){oxRCdrIhFC = true;}
      if(IWUnEeTLHB == CrBqrFtzuZ){lVlYqflVtP = true;}
      else if(CrBqrFtzuZ == IWUnEeTLHB){dSRuCtZyFV = true;}
      if(ZxOXnFZHnR == nDUGfrhQQL){UbrNlntToa = true;}
      else if(nDUGfrhQQL == ZxOXnFZHnR){nybXrlWjcL = true;}
      if(LWWuXnHCbg == JQSQSAaGSn){bpbeVYYGls = true;}
      else if(JQSQSAaGSn == LWWuXnHCbg){dEYILmIwZJ = true;}
      if(qqkBwKoHIV == nxhwGrbqaE){oXPMZjDUUk = true;}
      else if(nxhwGrbqaE == qqkBwKoHIV){aXocAGCNyd = true;}
      if(WdZZqOEcUI == DZAUWHukMG){YrjgzpyExl = true;}
      else if(DZAUWHukMG == WdZZqOEcUI){oUCkpqUtKq = true;}
      if(XsRpHkAYPP == XciDzlZkYq){UZDHEutGCR = true;}
      if(WQjZJhMMbc == dnfGlpUYzn){GfjBaxMfmB = true;}
      if(QSzRthxxEJ == PqpMIiQxsL){jZzECiGuee = true;}
      while(XciDzlZkYq == XsRpHkAYPP){VXEtYZtIiO = true;}
      while(dnfGlpUYzn == dnfGlpUYzn){lcXutQIzWk = true;}
      while(PqpMIiQxsL == PqpMIiQxsL){jJlzVIBmSU = true;}
      if(FpsLQKOewU == true){FpsLQKOewU = false;}
      if(RqRiZPsshg == true){RqRiZPsshg = false;}
      if(lVlYqflVtP == true){lVlYqflVtP = false;}
      if(UbrNlntToa == true){UbrNlntToa = false;}
      if(bpbeVYYGls == true){bpbeVYYGls = false;}
      if(oXPMZjDUUk == true){oXPMZjDUUk = false;}
      if(YrjgzpyExl == true){YrjgzpyExl = false;}
      if(UZDHEutGCR == true){UZDHEutGCR = false;}
      if(GfjBaxMfmB == true){GfjBaxMfmB = false;}
      if(jZzECiGuee == true){jZzECiGuee = false;}
      if(ZTIAXPTHHd == true){ZTIAXPTHHd = false;}
      if(oxRCdrIhFC == true){oxRCdrIhFC = false;}
      if(dSRuCtZyFV == true){dSRuCtZyFV = false;}
      if(nybXrlWjcL == true){nybXrlWjcL = false;}
      if(dEYILmIwZJ == true){dEYILmIwZJ = false;}
      if(aXocAGCNyd == true){aXocAGCNyd = false;}
      if(oUCkpqUtKq == true){oUCkpqUtKq = false;}
      if(VXEtYZtIiO == true){VXEtYZtIiO = false;}
      if(lcXutQIzWk == true){lcXutQIzWk = false;}
      if(jJlzVIBmSU == true){jJlzVIBmSU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KGPGJPGLSB
{ 
  void xmpLaSWnkH()
  { 
      bool EOCPjTiokb = false;
      bool VdbaEkKOcx = false;
      bool UiUjEsbBNL = false;
      bool QjmwVOYxFo = false;
      bool daEiNKItYi = false;
      bool NEXFpUsRKp = false;
      bool qhbYLlpeEo = false;
      bool LNInStdlWt = false;
      bool kgcexLLgFA = false;
      bool VHlPxwzeWa = false;
      bool TcojXAwimw = false;
      bool qHuQEKEwbA = false;
      bool ZIZESEbrpW = false;
      bool uyOuZHqrHw = false;
      bool OctJJOLVKm = false;
      bool qCDWBLDkQS = false;
      bool YErPArrzpx = false;
      bool SMoekxNPuE = false;
      bool rFgDesuqtO = false;
      bool DFbnXKjeIQ = false;
      string NcnECasEsl;
      string prgoWOtcjr;
      string wRhuXQUxul;
      string QEUYQHqNwS;
      string hNsWFChMUd;
      string saYVVwtsfk;
      string GwIHVlWHYP;
      string WggQKhXLoV;
      string iHbRsoeyCT;
      string MygKMkFkAS;
      string eZnEGLsILn;
      string XiDKCyUpOf;
      string GXFEDYzVDY;
      string cmKWpITWLg;
      string wWmOwcuWIN;
      string QOWULGUiBh;
      string MXElKkYfTs;
      string dxegjikqHZ;
      string dbMpINPoLi;
      string njVcTYSBDL;
      if(NcnECasEsl == eZnEGLsILn){EOCPjTiokb = true;}
      else if(eZnEGLsILn == NcnECasEsl){TcojXAwimw = true;}
      if(prgoWOtcjr == XiDKCyUpOf){VdbaEkKOcx = true;}
      else if(XiDKCyUpOf == prgoWOtcjr){qHuQEKEwbA = true;}
      if(wRhuXQUxul == GXFEDYzVDY){UiUjEsbBNL = true;}
      else if(GXFEDYzVDY == wRhuXQUxul){ZIZESEbrpW = true;}
      if(QEUYQHqNwS == cmKWpITWLg){QjmwVOYxFo = true;}
      else if(cmKWpITWLg == QEUYQHqNwS){uyOuZHqrHw = true;}
      if(hNsWFChMUd == wWmOwcuWIN){daEiNKItYi = true;}
      else if(wWmOwcuWIN == hNsWFChMUd){OctJJOLVKm = true;}
      if(saYVVwtsfk == QOWULGUiBh){NEXFpUsRKp = true;}
      else if(QOWULGUiBh == saYVVwtsfk){qCDWBLDkQS = true;}
      if(GwIHVlWHYP == MXElKkYfTs){qhbYLlpeEo = true;}
      else if(MXElKkYfTs == GwIHVlWHYP){YErPArrzpx = true;}
      if(WggQKhXLoV == dxegjikqHZ){LNInStdlWt = true;}
      if(iHbRsoeyCT == dbMpINPoLi){kgcexLLgFA = true;}
      if(MygKMkFkAS == njVcTYSBDL){VHlPxwzeWa = true;}
      while(dxegjikqHZ == WggQKhXLoV){SMoekxNPuE = true;}
      while(dbMpINPoLi == dbMpINPoLi){rFgDesuqtO = true;}
      while(njVcTYSBDL == njVcTYSBDL){DFbnXKjeIQ = true;}
      if(EOCPjTiokb == true){EOCPjTiokb = false;}
      if(VdbaEkKOcx == true){VdbaEkKOcx = false;}
      if(UiUjEsbBNL == true){UiUjEsbBNL = false;}
      if(QjmwVOYxFo == true){QjmwVOYxFo = false;}
      if(daEiNKItYi == true){daEiNKItYi = false;}
      if(NEXFpUsRKp == true){NEXFpUsRKp = false;}
      if(qhbYLlpeEo == true){qhbYLlpeEo = false;}
      if(LNInStdlWt == true){LNInStdlWt = false;}
      if(kgcexLLgFA == true){kgcexLLgFA = false;}
      if(VHlPxwzeWa == true){VHlPxwzeWa = false;}
      if(TcojXAwimw == true){TcojXAwimw = false;}
      if(qHuQEKEwbA == true){qHuQEKEwbA = false;}
      if(ZIZESEbrpW == true){ZIZESEbrpW = false;}
      if(uyOuZHqrHw == true){uyOuZHqrHw = false;}
      if(OctJJOLVKm == true){OctJJOLVKm = false;}
      if(qCDWBLDkQS == true){qCDWBLDkQS = false;}
      if(YErPArrzpx == true){YErPArrzpx = false;}
      if(SMoekxNPuE == true){SMoekxNPuE = false;}
      if(rFgDesuqtO == true){rFgDesuqtO = false;}
      if(DFbnXKjeIQ == true){DFbnXKjeIQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KRAVWBVTGV
{ 
  void JxpxrTsZpa()
  { 
      bool dxuolpIqjD = false;
      bool KgkMmzFsaN = false;
      bool milhXNxHDN = false;
      bool KUoERIZwNM = false;
      bool hxKCXnehTg = false;
      bool nGiAkuKMRY = false;
      bool rBmWrprfte = false;
      bool YIeNepXjKi = false;
      bool hPmWZEloTW = false;
      bool xMeoUfJmJg = false;
      bool wymoHeaUUt = false;
      bool XaChpbNQxm = false;
      bool EDMXgMdpsy = false;
      bool fjZcmjcyOt = false;
      bool dRTYzWJwyr = false;
      bool SspVcskOHu = false;
      bool AjaVNmAZuG = false;
      bool zPBCJlinYK = false;
      bool CgqBfmtDzs = false;
      bool yNpTsspxKL = false;
      string jbGjRAQXTo;
      string BFDWnuHjgC;
      string kUoJNMaGWj;
      string DwhAFKyYju;
      string nluTXfFHpj;
      string iJtdQPlxWE;
      string ZKBjkFWCRc;
      string whzkOzZUlg;
      string HSPmAtBEio;
      string YBBfVckKhR;
      string ICoMmAKIPR;
      string pQLoxGuNjq;
      string DSTbEugTPk;
      string NOmTYEcCzz;
      string jsnRzIoLhP;
      string hWaqNoiNIr;
      string TpsdDadrQs;
      string crOjGKeCcw;
      string FUkADpnuFi;
      string VPtGfszJNX;
      if(jbGjRAQXTo == ICoMmAKIPR){dxuolpIqjD = true;}
      else if(ICoMmAKIPR == jbGjRAQXTo){wymoHeaUUt = true;}
      if(BFDWnuHjgC == pQLoxGuNjq){KgkMmzFsaN = true;}
      else if(pQLoxGuNjq == BFDWnuHjgC){XaChpbNQxm = true;}
      if(kUoJNMaGWj == DSTbEugTPk){milhXNxHDN = true;}
      else if(DSTbEugTPk == kUoJNMaGWj){EDMXgMdpsy = true;}
      if(DwhAFKyYju == NOmTYEcCzz){KUoERIZwNM = true;}
      else if(NOmTYEcCzz == DwhAFKyYju){fjZcmjcyOt = true;}
      if(nluTXfFHpj == jsnRzIoLhP){hxKCXnehTg = true;}
      else if(jsnRzIoLhP == nluTXfFHpj){dRTYzWJwyr = true;}
      if(iJtdQPlxWE == hWaqNoiNIr){nGiAkuKMRY = true;}
      else if(hWaqNoiNIr == iJtdQPlxWE){SspVcskOHu = true;}
      if(ZKBjkFWCRc == TpsdDadrQs){rBmWrprfte = true;}
      else if(TpsdDadrQs == ZKBjkFWCRc){AjaVNmAZuG = true;}
      if(whzkOzZUlg == crOjGKeCcw){YIeNepXjKi = true;}
      if(HSPmAtBEio == FUkADpnuFi){hPmWZEloTW = true;}
      if(YBBfVckKhR == VPtGfszJNX){xMeoUfJmJg = true;}
      while(crOjGKeCcw == whzkOzZUlg){zPBCJlinYK = true;}
      while(FUkADpnuFi == FUkADpnuFi){CgqBfmtDzs = true;}
      while(VPtGfszJNX == VPtGfszJNX){yNpTsspxKL = true;}
      if(dxuolpIqjD == true){dxuolpIqjD = false;}
      if(KgkMmzFsaN == true){KgkMmzFsaN = false;}
      if(milhXNxHDN == true){milhXNxHDN = false;}
      if(KUoERIZwNM == true){KUoERIZwNM = false;}
      if(hxKCXnehTg == true){hxKCXnehTg = false;}
      if(nGiAkuKMRY == true){nGiAkuKMRY = false;}
      if(rBmWrprfte == true){rBmWrprfte = false;}
      if(YIeNepXjKi == true){YIeNepXjKi = false;}
      if(hPmWZEloTW == true){hPmWZEloTW = false;}
      if(xMeoUfJmJg == true){xMeoUfJmJg = false;}
      if(wymoHeaUUt == true){wymoHeaUUt = false;}
      if(XaChpbNQxm == true){XaChpbNQxm = false;}
      if(EDMXgMdpsy == true){EDMXgMdpsy = false;}
      if(fjZcmjcyOt == true){fjZcmjcyOt = false;}
      if(dRTYzWJwyr == true){dRTYzWJwyr = false;}
      if(SspVcskOHu == true){SspVcskOHu = false;}
      if(AjaVNmAZuG == true){AjaVNmAZuG = false;}
      if(zPBCJlinYK == true){zPBCJlinYK = false;}
      if(CgqBfmtDzs == true){CgqBfmtDzs = false;}
      if(yNpTsspxKL == true){yNpTsspxKL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DAHTNJIEDJ
{ 
  void qqZWRTgRrH()
  { 
      bool gYKXVzMkSz = false;
      bool rRqyEkfOBF = false;
      bool HTnrJrSiEc = false;
      bool AYttZaKgRJ = false;
      bool diegZUFeNQ = false;
      bool isDeHMkSNc = false;
      bool gdfozTXwXc = false;
      bool ZyxqbXnGRo = false;
      bool lnUntGUOlN = false;
      bool prSobADphG = false;
      bool xJGkqjwLDj = false;
      bool XdBYprIrur = false;
      bool yIKKmhfHPa = false;
      bool TyYKrchjxJ = false;
      bool JIKCoLqtzj = false;
      bool qLtoInzuyI = false;
      bool EowcPZFpfM = false;
      bool JYulhoxHVa = false;
      bool qkHejIDjCD = false;
      bool dllnEULhlO = false;
      string KFzukUVNqo;
      string uyeVDYmGji;
      string cBPZqGgtRT;
      string pNafoSbZfl;
      string CifCJmJhxu;
      string LiAwwJpYEe;
      string mauXzgXygj;
      string wNZyfsUNQE;
      string xzwVHHNejf;
      string cCVBzanIDb;
      string OLBNyyIBPC;
      string ncIAhneYPq;
      string TsNAHtqYzF;
      string FMJnokxiWI;
      string zLOhqzsVTL;
      string wOrLHVyAFE;
      string GjCKwbSUKw;
      string pwBGrIdfJH;
      string AVEmIqUyfS;
      string mxcSCOIEkO;
      if(KFzukUVNqo == OLBNyyIBPC){gYKXVzMkSz = true;}
      else if(OLBNyyIBPC == KFzukUVNqo){xJGkqjwLDj = true;}
      if(uyeVDYmGji == ncIAhneYPq){rRqyEkfOBF = true;}
      else if(ncIAhneYPq == uyeVDYmGji){XdBYprIrur = true;}
      if(cBPZqGgtRT == TsNAHtqYzF){HTnrJrSiEc = true;}
      else if(TsNAHtqYzF == cBPZqGgtRT){yIKKmhfHPa = true;}
      if(pNafoSbZfl == FMJnokxiWI){AYttZaKgRJ = true;}
      else if(FMJnokxiWI == pNafoSbZfl){TyYKrchjxJ = true;}
      if(CifCJmJhxu == zLOhqzsVTL){diegZUFeNQ = true;}
      else if(zLOhqzsVTL == CifCJmJhxu){JIKCoLqtzj = true;}
      if(LiAwwJpYEe == wOrLHVyAFE){isDeHMkSNc = true;}
      else if(wOrLHVyAFE == LiAwwJpYEe){qLtoInzuyI = true;}
      if(mauXzgXygj == GjCKwbSUKw){gdfozTXwXc = true;}
      else if(GjCKwbSUKw == mauXzgXygj){EowcPZFpfM = true;}
      if(wNZyfsUNQE == pwBGrIdfJH){ZyxqbXnGRo = true;}
      if(xzwVHHNejf == AVEmIqUyfS){lnUntGUOlN = true;}
      if(cCVBzanIDb == mxcSCOIEkO){prSobADphG = true;}
      while(pwBGrIdfJH == wNZyfsUNQE){JYulhoxHVa = true;}
      while(AVEmIqUyfS == AVEmIqUyfS){qkHejIDjCD = true;}
      while(mxcSCOIEkO == mxcSCOIEkO){dllnEULhlO = true;}
      if(gYKXVzMkSz == true){gYKXVzMkSz = false;}
      if(rRqyEkfOBF == true){rRqyEkfOBF = false;}
      if(HTnrJrSiEc == true){HTnrJrSiEc = false;}
      if(AYttZaKgRJ == true){AYttZaKgRJ = false;}
      if(diegZUFeNQ == true){diegZUFeNQ = false;}
      if(isDeHMkSNc == true){isDeHMkSNc = false;}
      if(gdfozTXwXc == true){gdfozTXwXc = false;}
      if(ZyxqbXnGRo == true){ZyxqbXnGRo = false;}
      if(lnUntGUOlN == true){lnUntGUOlN = false;}
      if(prSobADphG == true){prSobADphG = false;}
      if(xJGkqjwLDj == true){xJGkqjwLDj = false;}
      if(XdBYprIrur == true){XdBYprIrur = false;}
      if(yIKKmhfHPa == true){yIKKmhfHPa = false;}
      if(TyYKrchjxJ == true){TyYKrchjxJ = false;}
      if(JIKCoLqtzj == true){JIKCoLqtzj = false;}
      if(qLtoInzuyI == true){qLtoInzuyI = false;}
      if(EowcPZFpfM == true){EowcPZFpfM = false;}
      if(JYulhoxHVa == true){JYulhoxHVa = false;}
      if(qkHejIDjCD == true){qkHejIDjCD = false;}
      if(dllnEULhlO == true){dllnEULhlO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HOAMJCFFXP
{ 
  void MNJKwBxKLp()
  { 
      bool bqeeUfDNTB = false;
      bool GkotQtgmVc = false;
      bool QzQGDuMrrG = false;
      bool gsGtWncmuP = false;
      bool xXWDEZtmQx = false;
      bool wjffsbaXaZ = false;
      bool SLVNTNetWb = false;
      bool QsqFfscMCd = false;
      bool AZbtKxNaAu = false;
      bool NFLVxkBIip = false;
      bool tzPngCrZzA = false;
      bool yNUxYsjbCW = false;
      bool zEsbOKhVIt = false;
      bool rUdLiBJmMI = false;
      bool EnaAxzkOwD = false;
      bool BYFMqpGnZV = false;
      bool UVQFIdjINX = false;
      bool jxOtRLcreO = false;
      bool ZSYFsSOQUs = false;
      bool tVMqmhWVrS = false;
      string UbYoKYVZWo;
      string wtLUVGYzZV;
      string SUeyaRZPzq;
      string mbTOABTqOF;
      string CHISDtBhed;
      string pxkITzxaNr;
      string zGZfCTaxTU;
      string mhjUSLMLtg;
      string QqkVBppKuA;
      string DscWFwgbQi;
      string SDNJqHMQpH;
      string mkffbHVEdM;
      string dQCTQsENOW;
      string djuKNMdaet;
      string cZUmNRMxty;
      string acyEFCBtsx;
      string CzCSdCpVSP;
      string dtYjjEuPFy;
      string fHSPwAlqEL;
      string LihgrcWCIK;
      if(UbYoKYVZWo == SDNJqHMQpH){bqeeUfDNTB = true;}
      else if(SDNJqHMQpH == UbYoKYVZWo){tzPngCrZzA = true;}
      if(wtLUVGYzZV == mkffbHVEdM){GkotQtgmVc = true;}
      else if(mkffbHVEdM == wtLUVGYzZV){yNUxYsjbCW = true;}
      if(SUeyaRZPzq == dQCTQsENOW){QzQGDuMrrG = true;}
      else if(dQCTQsENOW == SUeyaRZPzq){zEsbOKhVIt = true;}
      if(mbTOABTqOF == djuKNMdaet){gsGtWncmuP = true;}
      else if(djuKNMdaet == mbTOABTqOF){rUdLiBJmMI = true;}
      if(CHISDtBhed == cZUmNRMxty){xXWDEZtmQx = true;}
      else if(cZUmNRMxty == CHISDtBhed){EnaAxzkOwD = true;}
      if(pxkITzxaNr == acyEFCBtsx){wjffsbaXaZ = true;}
      else if(acyEFCBtsx == pxkITzxaNr){BYFMqpGnZV = true;}
      if(zGZfCTaxTU == CzCSdCpVSP){SLVNTNetWb = true;}
      else if(CzCSdCpVSP == zGZfCTaxTU){UVQFIdjINX = true;}
      if(mhjUSLMLtg == dtYjjEuPFy){QsqFfscMCd = true;}
      if(QqkVBppKuA == fHSPwAlqEL){AZbtKxNaAu = true;}
      if(DscWFwgbQi == LihgrcWCIK){NFLVxkBIip = true;}
      while(dtYjjEuPFy == mhjUSLMLtg){jxOtRLcreO = true;}
      while(fHSPwAlqEL == fHSPwAlqEL){ZSYFsSOQUs = true;}
      while(LihgrcWCIK == LihgrcWCIK){tVMqmhWVrS = true;}
      if(bqeeUfDNTB == true){bqeeUfDNTB = false;}
      if(GkotQtgmVc == true){GkotQtgmVc = false;}
      if(QzQGDuMrrG == true){QzQGDuMrrG = false;}
      if(gsGtWncmuP == true){gsGtWncmuP = false;}
      if(xXWDEZtmQx == true){xXWDEZtmQx = false;}
      if(wjffsbaXaZ == true){wjffsbaXaZ = false;}
      if(SLVNTNetWb == true){SLVNTNetWb = false;}
      if(QsqFfscMCd == true){QsqFfscMCd = false;}
      if(AZbtKxNaAu == true){AZbtKxNaAu = false;}
      if(NFLVxkBIip == true){NFLVxkBIip = false;}
      if(tzPngCrZzA == true){tzPngCrZzA = false;}
      if(yNUxYsjbCW == true){yNUxYsjbCW = false;}
      if(zEsbOKhVIt == true){zEsbOKhVIt = false;}
      if(rUdLiBJmMI == true){rUdLiBJmMI = false;}
      if(EnaAxzkOwD == true){EnaAxzkOwD = false;}
      if(BYFMqpGnZV == true){BYFMqpGnZV = false;}
      if(UVQFIdjINX == true){UVQFIdjINX = false;}
      if(jxOtRLcreO == true){jxOtRLcreO = false;}
      if(ZSYFsSOQUs == true){ZSYFsSOQUs = false;}
      if(tVMqmhWVrS == true){tVMqmhWVrS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HWVCVALLYV
{ 
  void YbwfkxsXYj()
  { 
      bool AZamGESrYd = false;
      bool oHoHnSxSVA = false;
      bool jmbUywQGHC = false;
      bool DJciybTqcM = false;
      bool ZZdTMbJPSq = false;
      bool alRfSNxFoK = false;
      bool nLkVZedGIL = false;
      bool zGpJZJJIHW = false;
      bool lAnPaQrwPk = false;
      bool dNCRXNnHsg = false;
      bool xDHGrUwIFI = false;
      bool DoxRXmXUuJ = false;
      bool mhqWgkDXkL = false;
      bool VBOaYishXF = false;
      bool UjqGgoPjtk = false;
      bool pSMjtPMkYG = false;
      bool QXWtmCaWso = false;
      bool towonCqjor = false;
      bool UTdzUCodfI = false;
      bool VHIRnxMxSe = false;
      string DhSQyunCFQ;
      string NjsPcjWlwU;
      string EwLNFShwBl;
      string KynUpESUhl;
      string BpThVFVqUL;
      string HgAQeGVEKe;
      string MrATFPuXZX;
      string RBEKTNcdrz;
      string xqapddxXpi;
      string hNJDYJoAbF;
      string KxQwtFcoxQ;
      string drMLgVRJob;
      string RXhppDrAlJ;
      string XcClozZdwX;
      string AIaYEpBxVC;
      string rfKzmlPaGp;
      string NbCmrBoaWh;
      string umMRaJxIkz;
      string CteiRrRCyC;
      string izWGKYFmfC;
      if(DhSQyunCFQ == KxQwtFcoxQ){AZamGESrYd = true;}
      else if(KxQwtFcoxQ == DhSQyunCFQ){xDHGrUwIFI = true;}
      if(NjsPcjWlwU == drMLgVRJob){oHoHnSxSVA = true;}
      else if(drMLgVRJob == NjsPcjWlwU){DoxRXmXUuJ = true;}
      if(EwLNFShwBl == RXhppDrAlJ){jmbUywQGHC = true;}
      else if(RXhppDrAlJ == EwLNFShwBl){mhqWgkDXkL = true;}
      if(KynUpESUhl == XcClozZdwX){DJciybTqcM = true;}
      else if(XcClozZdwX == KynUpESUhl){VBOaYishXF = true;}
      if(BpThVFVqUL == AIaYEpBxVC){ZZdTMbJPSq = true;}
      else if(AIaYEpBxVC == BpThVFVqUL){UjqGgoPjtk = true;}
      if(HgAQeGVEKe == rfKzmlPaGp){alRfSNxFoK = true;}
      else if(rfKzmlPaGp == HgAQeGVEKe){pSMjtPMkYG = true;}
      if(MrATFPuXZX == NbCmrBoaWh){nLkVZedGIL = true;}
      else if(NbCmrBoaWh == MrATFPuXZX){QXWtmCaWso = true;}
      if(RBEKTNcdrz == umMRaJxIkz){zGpJZJJIHW = true;}
      if(xqapddxXpi == CteiRrRCyC){lAnPaQrwPk = true;}
      if(hNJDYJoAbF == izWGKYFmfC){dNCRXNnHsg = true;}
      while(umMRaJxIkz == RBEKTNcdrz){towonCqjor = true;}
      while(CteiRrRCyC == CteiRrRCyC){UTdzUCodfI = true;}
      while(izWGKYFmfC == izWGKYFmfC){VHIRnxMxSe = true;}
      if(AZamGESrYd == true){AZamGESrYd = false;}
      if(oHoHnSxSVA == true){oHoHnSxSVA = false;}
      if(jmbUywQGHC == true){jmbUywQGHC = false;}
      if(DJciybTqcM == true){DJciybTqcM = false;}
      if(ZZdTMbJPSq == true){ZZdTMbJPSq = false;}
      if(alRfSNxFoK == true){alRfSNxFoK = false;}
      if(nLkVZedGIL == true){nLkVZedGIL = false;}
      if(zGpJZJJIHW == true){zGpJZJJIHW = false;}
      if(lAnPaQrwPk == true){lAnPaQrwPk = false;}
      if(dNCRXNnHsg == true){dNCRXNnHsg = false;}
      if(xDHGrUwIFI == true){xDHGrUwIFI = false;}
      if(DoxRXmXUuJ == true){DoxRXmXUuJ = false;}
      if(mhqWgkDXkL == true){mhqWgkDXkL = false;}
      if(VBOaYishXF == true){VBOaYishXF = false;}
      if(UjqGgoPjtk == true){UjqGgoPjtk = false;}
      if(pSMjtPMkYG == true){pSMjtPMkYG = false;}
      if(QXWtmCaWso == true){QXWtmCaWso = false;}
      if(towonCqjor == true){towonCqjor = false;}
      if(UTdzUCodfI == true){UTdzUCodfI = false;}
      if(VHIRnxMxSe == true){VHIRnxMxSe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BSOGLCIKGB
{ 
  void WpkkErndaK()
  { 
      bool uFUOajebyA = false;
      bool WPbnsProwE = false;
      bool KMPhpGhcdi = false;
      bool whdKLNPesS = false;
      bool UjQWnKPRGP = false;
      bool EiyzHholYT = false;
      bool BBxODnlRbf = false;
      bool AMwciZustf = false;
      bool qQWNdMidDd = false;
      bool XcGYQTlCTz = false;
      bool YyoaPqmVQh = false;
      bool lwrrckejjp = false;
      bool FHOyHXGWLP = false;
      bool msYtGsbuym = false;
      bool ZNNUrajNwf = false;
      bool nCRctQjqSq = false;
      bool kPJBtrXyki = false;
      bool JikxPqtsFg = false;
      bool msafajbwzo = false;
      bool IjcGBKELZz = false;
      string OzfjSnubRN;
      string uiFdcMUTIX;
      string iVpgzPUczw;
      string DGorCDYDXq;
      string gbwDORCzAM;
      string QHURVsJykC;
      string SScVJotNaY;
      string mQuRpHYmRg;
      string MVclfukWWJ;
      string tZRUXOyasY;
      string WoXaZzMDbN;
      string UWgDoxpppO;
      string DXqReMzZtb;
      string IxssKXyBMf;
      string ihuIoXsOfZ;
      string YIkdVzxFuM;
      string AlJzEWcmsQ;
      string zncJfVchcH;
      string SjtTDAMPWK;
      string ZrFAfwYuER;
      if(OzfjSnubRN == WoXaZzMDbN){uFUOajebyA = true;}
      else if(WoXaZzMDbN == OzfjSnubRN){YyoaPqmVQh = true;}
      if(uiFdcMUTIX == UWgDoxpppO){WPbnsProwE = true;}
      else if(UWgDoxpppO == uiFdcMUTIX){lwrrckejjp = true;}
      if(iVpgzPUczw == DXqReMzZtb){KMPhpGhcdi = true;}
      else if(DXqReMzZtb == iVpgzPUczw){FHOyHXGWLP = true;}
      if(DGorCDYDXq == IxssKXyBMf){whdKLNPesS = true;}
      else if(IxssKXyBMf == DGorCDYDXq){msYtGsbuym = true;}
      if(gbwDORCzAM == ihuIoXsOfZ){UjQWnKPRGP = true;}
      else if(ihuIoXsOfZ == gbwDORCzAM){ZNNUrajNwf = true;}
      if(QHURVsJykC == YIkdVzxFuM){EiyzHholYT = true;}
      else if(YIkdVzxFuM == QHURVsJykC){nCRctQjqSq = true;}
      if(SScVJotNaY == AlJzEWcmsQ){BBxODnlRbf = true;}
      else if(AlJzEWcmsQ == SScVJotNaY){kPJBtrXyki = true;}
      if(mQuRpHYmRg == zncJfVchcH){AMwciZustf = true;}
      if(MVclfukWWJ == SjtTDAMPWK){qQWNdMidDd = true;}
      if(tZRUXOyasY == ZrFAfwYuER){XcGYQTlCTz = true;}
      while(zncJfVchcH == mQuRpHYmRg){JikxPqtsFg = true;}
      while(SjtTDAMPWK == SjtTDAMPWK){msafajbwzo = true;}
      while(ZrFAfwYuER == ZrFAfwYuER){IjcGBKELZz = true;}
      if(uFUOajebyA == true){uFUOajebyA = false;}
      if(WPbnsProwE == true){WPbnsProwE = false;}
      if(KMPhpGhcdi == true){KMPhpGhcdi = false;}
      if(whdKLNPesS == true){whdKLNPesS = false;}
      if(UjQWnKPRGP == true){UjQWnKPRGP = false;}
      if(EiyzHholYT == true){EiyzHholYT = false;}
      if(BBxODnlRbf == true){BBxODnlRbf = false;}
      if(AMwciZustf == true){AMwciZustf = false;}
      if(qQWNdMidDd == true){qQWNdMidDd = false;}
      if(XcGYQTlCTz == true){XcGYQTlCTz = false;}
      if(YyoaPqmVQh == true){YyoaPqmVQh = false;}
      if(lwrrckejjp == true){lwrrckejjp = false;}
      if(FHOyHXGWLP == true){FHOyHXGWLP = false;}
      if(msYtGsbuym == true){msYtGsbuym = false;}
      if(ZNNUrajNwf == true){ZNNUrajNwf = false;}
      if(nCRctQjqSq == true){nCRctQjqSq = false;}
      if(kPJBtrXyki == true){kPJBtrXyki = false;}
      if(JikxPqtsFg == true){JikxPqtsFg = false;}
      if(msafajbwzo == true){msafajbwzo = false;}
      if(IjcGBKELZz == true){IjcGBKELZz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MUYRZCZUKA
{ 
  void mqwRDoFLdP()
  { 
      bool ZWaQiArXSa = false;
      bool wLkAtnuRow = false;
      bool zoCepRuBFZ = false;
      bool OpyRPsFRQQ = false;
      bool KnhusTBrJh = false;
      bool EIFfekTuaQ = false;
      bool FqSsMFNSkB = false;
      bool hcAYkKHHlV = false;
      bool QCrWVGkRme = false;
      bool YJqzKSVoim = false;
      bool GiHIAaHgtT = false;
      bool dLJEzDywWF = false;
      bool UueGbYlBNR = false;
      bool oRYzPhujWt = false;
      bool yDXSBMAlzH = false;
      bool UYRcRKDBfa = false;
      bool oGSbeLmKoR = false;
      bool HOZcHQXLEO = false;
      bool XzBGxPAwgE = false;
      bool etDuEqkuno = false;
      string QxQxLUsbQE;
      string isVWlcqMZj;
      string lKVRMTInYG;
      string XDssUdIZRY;
      string RTddeUOUGp;
      string QtGeSGbzjr;
      string MFCJgeAbBI;
      string TRuASSgqmr;
      string EmhipcxNaO;
      string BIoZMMQTxz;
      string TmTxVyzFMA;
      string YkMrmYXSTD;
      string hlAlGLcUnG;
      string BMtmXRTjNq;
      string drBapQuwln;
      string nyAsQhlgqJ;
      string UdRGzLpYUV;
      string EnFbZokZwp;
      string PgizRDMBWo;
      string flsDWzatbd;
      if(QxQxLUsbQE == TmTxVyzFMA){ZWaQiArXSa = true;}
      else if(TmTxVyzFMA == QxQxLUsbQE){GiHIAaHgtT = true;}
      if(isVWlcqMZj == YkMrmYXSTD){wLkAtnuRow = true;}
      else if(YkMrmYXSTD == isVWlcqMZj){dLJEzDywWF = true;}
      if(lKVRMTInYG == hlAlGLcUnG){zoCepRuBFZ = true;}
      else if(hlAlGLcUnG == lKVRMTInYG){UueGbYlBNR = true;}
      if(XDssUdIZRY == BMtmXRTjNq){OpyRPsFRQQ = true;}
      else if(BMtmXRTjNq == XDssUdIZRY){oRYzPhujWt = true;}
      if(RTddeUOUGp == drBapQuwln){KnhusTBrJh = true;}
      else if(drBapQuwln == RTddeUOUGp){yDXSBMAlzH = true;}
      if(QtGeSGbzjr == nyAsQhlgqJ){EIFfekTuaQ = true;}
      else if(nyAsQhlgqJ == QtGeSGbzjr){UYRcRKDBfa = true;}
      if(MFCJgeAbBI == UdRGzLpYUV){FqSsMFNSkB = true;}
      else if(UdRGzLpYUV == MFCJgeAbBI){oGSbeLmKoR = true;}
      if(TRuASSgqmr == EnFbZokZwp){hcAYkKHHlV = true;}
      if(EmhipcxNaO == PgizRDMBWo){QCrWVGkRme = true;}
      if(BIoZMMQTxz == flsDWzatbd){YJqzKSVoim = true;}
      while(EnFbZokZwp == TRuASSgqmr){HOZcHQXLEO = true;}
      while(PgizRDMBWo == PgizRDMBWo){XzBGxPAwgE = true;}
      while(flsDWzatbd == flsDWzatbd){etDuEqkuno = true;}
      if(ZWaQiArXSa == true){ZWaQiArXSa = false;}
      if(wLkAtnuRow == true){wLkAtnuRow = false;}
      if(zoCepRuBFZ == true){zoCepRuBFZ = false;}
      if(OpyRPsFRQQ == true){OpyRPsFRQQ = false;}
      if(KnhusTBrJh == true){KnhusTBrJh = false;}
      if(EIFfekTuaQ == true){EIFfekTuaQ = false;}
      if(FqSsMFNSkB == true){FqSsMFNSkB = false;}
      if(hcAYkKHHlV == true){hcAYkKHHlV = false;}
      if(QCrWVGkRme == true){QCrWVGkRme = false;}
      if(YJqzKSVoim == true){YJqzKSVoim = false;}
      if(GiHIAaHgtT == true){GiHIAaHgtT = false;}
      if(dLJEzDywWF == true){dLJEzDywWF = false;}
      if(UueGbYlBNR == true){UueGbYlBNR = false;}
      if(oRYzPhujWt == true){oRYzPhujWt = false;}
      if(yDXSBMAlzH == true){yDXSBMAlzH = false;}
      if(UYRcRKDBfa == true){UYRcRKDBfa = false;}
      if(oGSbeLmKoR == true){oGSbeLmKoR = false;}
      if(HOZcHQXLEO == true){HOZcHQXLEO = false;}
      if(XzBGxPAwgE == true){XzBGxPAwgE = false;}
      if(etDuEqkuno == true){etDuEqkuno = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RBQDMSZFRS
{ 
  void BROGdMilZK()
  { 
      bool ZoBcfhEUbe = false;
      bool JuAoUiQSON = false;
      bool oTXggCGYPj = false;
      bool FHHTZJGzyg = false;
      bool jZtqycYdxh = false;
      bool tePDVFRTwc = false;
      bool afFqYCmfOK = false;
      bool bQUhdPYfiD = false;
      bool dcagoZqExU = false;
      bool rmDVFETDSw = false;
      bool orOlGdKhZG = false;
      bool flkrzxEXBz = false;
      bool KiitSnNzbZ = false;
      bool uVNlOkOoaL = false;
      bool OJdqnGBKVH = false;
      bool zwBSkFJUSR = false;
      bool PLGAKJXydN = false;
      bool iOwpjBEqFL = false;
      bool mCcTZzUtBI = false;
      bool btVJuDDdgd = false;
      string iNqujNabIc;
      string PiDFUqPlBw;
      string zOJCBaWgJW;
      string NkaCzEYeqc;
      string jqaoNBQMUU;
      string ZBGOuUEotI;
      string ptRkOdIBFP;
      string RjtXMImTnz;
      string JKMoxMeLGd;
      string AjkhmoYpZi;
      string sPGECoGiBl;
      string hjAtzfgcnC;
      string zwtbPhyMkX;
      string jxtFFtEHlH;
      string GuJUwPipiH;
      string HuBXixVeoT;
      string qtxdCkptoF;
      string bZqfwEmZez;
      string zIcdPnuVWI;
      string YYSRQLyeRL;
      if(iNqujNabIc == sPGECoGiBl){ZoBcfhEUbe = true;}
      else if(sPGECoGiBl == iNqujNabIc){orOlGdKhZG = true;}
      if(PiDFUqPlBw == hjAtzfgcnC){JuAoUiQSON = true;}
      else if(hjAtzfgcnC == PiDFUqPlBw){flkrzxEXBz = true;}
      if(zOJCBaWgJW == zwtbPhyMkX){oTXggCGYPj = true;}
      else if(zwtbPhyMkX == zOJCBaWgJW){KiitSnNzbZ = true;}
      if(NkaCzEYeqc == jxtFFtEHlH){FHHTZJGzyg = true;}
      else if(jxtFFtEHlH == NkaCzEYeqc){uVNlOkOoaL = true;}
      if(jqaoNBQMUU == GuJUwPipiH){jZtqycYdxh = true;}
      else if(GuJUwPipiH == jqaoNBQMUU){OJdqnGBKVH = true;}
      if(ZBGOuUEotI == HuBXixVeoT){tePDVFRTwc = true;}
      else if(HuBXixVeoT == ZBGOuUEotI){zwBSkFJUSR = true;}
      if(ptRkOdIBFP == qtxdCkptoF){afFqYCmfOK = true;}
      else if(qtxdCkptoF == ptRkOdIBFP){PLGAKJXydN = true;}
      if(RjtXMImTnz == bZqfwEmZez){bQUhdPYfiD = true;}
      if(JKMoxMeLGd == zIcdPnuVWI){dcagoZqExU = true;}
      if(AjkhmoYpZi == YYSRQLyeRL){rmDVFETDSw = true;}
      while(bZqfwEmZez == RjtXMImTnz){iOwpjBEqFL = true;}
      while(zIcdPnuVWI == zIcdPnuVWI){mCcTZzUtBI = true;}
      while(YYSRQLyeRL == YYSRQLyeRL){btVJuDDdgd = true;}
      if(ZoBcfhEUbe == true){ZoBcfhEUbe = false;}
      if(JuAoUiQSON == true){JuAoUiQSON = false;}
      if(oTXggCGYPj == true){oTXggCGYPj = false;}
      if(FHHTZJGzyg == true){FHHTZJGzyg = false;}
      if(jZtqycYdxh == true){jZtqycYdxh = false;}
      if(tePDVFRTwc == true){tePDVFRTwc = false;}
      if(afFqYCmfOK == true){afFqYCmfOK = false;}
      if(bQUhdPYfiD == true){bQUhdPYfiD = false;}
      if(dcagoZqExU == true){dcagoZqExU = false;}
      if(rmDVFETDSw == true){rmDVFETDSw = false;}
      if(orOlGdKhZG == true){orOlGdKhZG = false;}
      if(flkrzxEXBz == true){flkrzxEXBz = false;}
      if(KiitSnNzbZ == true){KiitSnNzbZ = false;}
      if(uVNlOkOoaL == true){uVNlOkOoaL = false;}
      if(OJdqnGBKVH == true){OJdqnGBKVH = false;}
      if(zwBSkFJUSR == true){zwBSkFJUSR = false;}
      if(PLGAKJXydN == true){PLGAKJXydN = false;}
      if(iOwpjBEqFL == true){iOwpjBEqFL = false;}
      if(mCcTZzUtBI == true){mCcTZzUtBI = false;}
      if(btVJuDDdgd == true){btVJuDDdgd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UCOFHTPQQO
{ 
  void KTkrhEWLZV()
  { 
      bool RqzccpfCAW = false;
      bool TUidOSFGOq = false;
      bool DhyBKzDQQm = false;
      bool iFbqdKoRTM = false;
      bool yiiGnpPmEe = false;
      bool RMJGpisyRd = false;
      bool lPlMnNsrAD = false;
      bool cnmBrUyrMU = false;
      bool iQLkVVwVAt = false;
      bool QpzoaixVSZ = false;
      bool xXmACdYHNV = false;
      bool kAlxtUDAqR = false;
      bool RWqFllnyaJ = false;
      bool XbZlWLdKir = false;
      bool xnHeIQECtF = false;
      bool eweAHZfgNP = false;
      bool pagDxJazZi = false;
      bool qsQyTdPLwV = false;
      bool gkDVdawYex = false;
      bool ewwTCWGDhz = false;
      string ljxPLdnNSU;
      string YbyLfniCxT;
      string BEfVbxzfXn;
      string XzmZMCWLla;
      string gYPpWFIoiC;
      string cIzRrhEbmN;
      string dIodwYnYzd;
      string QXQVlcnoyW;
      string nKwVKOAUgd;
      string yuYXwXiksx;
      string QjpFlFrcfO;
      string sOcPZTMTTg;
      string YSqUeUyrZO;
      string tDwBGEUqjF;
      string WWoRjjSJyc;
      string LPRLYxsUZQ;
      string HzDQUpRaJV;
      string zUjOKBdYku;
      string CotwCdljcI;
      string KzQSxWSXxt;
      if(ljxPLdnNSU == QjpFlFrcfO){RqzccpfCAW = true;}
      else if(QjpFlFrcfO == ljxPLdnNSU){xXmACdYHNV = true;}
      if(YbyLfniCxT == sOcPZTMTTg){TUidOSFGOq = true;}
      else if(sOcPZTMTTg == YbyLfniCxT){kAlxtUDAqR = true;}
      if(BEfVbxzfXn == YSqUeUyrZO){DhyBKzDQQm = true;}
      else if(YSqUeUyrZO == BEfVbxzfXn){RWqFllnyaJ = true;}
      if(XzmZMCWLla == tDwBGEUqjF){iFbqdKoRTM = true;}
      else if(tDwBGEUqjF == XzmZMCWLla){XbZlWLdKir = true;}
      if(gYPpWFIoiC == WWoRjjSJyc){yiiGnpPmEe = true;}
      else if(WWoRjjSJyc == gYPpWFIoiC){xnHeIQECtF = true;}
      if(cIzRrhEbmN == LPRLYxsUZQ){RMJGpisyRd = true;}
      else if(LPRLYxsUZQ == cIzRrhEbmN){eweAHZfgNP = true;}
      if(dIodwYnYzd == HzDQUpRaJV){lPlMnNsrAD = true;}
      else if(HzDQUpRaJV == dIodwYnYzd){pagDxJazZi = true;}
      if(QXQVlcnoyW == zUjOKBdYku){cnmBrUyrMU = true;}
      if(nKwVKOAUgd == CotwCdljcI){iQLkVVwVAt = true;}
      if(yuYXwXiksx == KzQSxWSXxt){QpzoaixVSZ = true;}
      while(zUjOKBdYku == QXQVlcnoyW){qsQyTdPLwV = true;}
      while(CotwCdljcI == CotwCdljcI){gkDVdawYex = true;}
      while(KzQSxWSXxt == KzQSxWSXxt){ewwTCWGDhz = true;}
      if(RqzccpfCAW == true){RqzccpfCAW = false;}
      if(TUidOSFGOq == true){TUidOSFGOq = false;}
      if(DhyBKzDQQm == true){DhyBKzDQQm = false;}
      if(iFbqdKoRTM == true){iFbqdKoRTM = false;}
      if(yiiGnpPmEe == true){yiiGnpPmEe = false;}
      if(RMJGpisyRd == true){RMJGpisyRd = false;}
      if(lPlMnNsrAD == true){lPlMnNsrAD = false;}
      if(cnmBrUyrMU == true){cnmBrUyrMU = false;}
      if(iQLkVVwVAt == true){iQLkVVwVAt = false;}
      if(QpzoaixVSZ == true){QpzoaixVSZ = false;}
      if(xXmACdYHNV == true){xXmACdYHNV = false;}
      if(kAlxtUDAqR == true){kAlxtUDAqR = false;}
      if(RWqFllnyaJ == true){RWqFllnyaJ = false;}
      if(XbZlWLdKir == true){XbZlWLdKir = false;}
      if(xnHeIQECtF == true){xnHeIQECtF = false;}
      if(eweAHZfgNP == true){eweAHZfgNP = false;}
      if(pagDxJazZi == true){pagDxJazZi = false;}
      if(qsQyTdPLwV == true){qsQyTdPLwV = false;}
      if(gkDVdawYex == true){gkDVdawYex = false;}
      if(ewwTCWGDhz == true){ewwTCWGDhz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YJETFARQNB
{ 
  void osQnNnLSKy()
  { 
      bool omNupXEMUy = false;
      bool aQLPZwVlnM = false;
      bool BLCAeSIyxU = false;
      bool kQLIjaqosz = false;
      bool FCPjkGLMhr = false;
      bool GsyXuckNQW = false;
      bool OFPAMJFDey = false;
      bool FfsnHzwROX = false;
      bool GuQENrSmMH = false;
      bool pdXVjzmPXb = false;
      bool BIhimZAJQu = false;
      bool fTysQToKlT = false;
      bool pRpWIbIWDH = false;
      bool MgtmVyMgRn = false;
      bool rJWQbVeeFQ = false;
      bool JbcDkiRLaK = false;
      bool RcFIiyYLWS = false;
      bool GIrxbgtYtl = false;
      bool xcRmxjDqBl = false;
      bool fBGGWODOhc = false;
      string lwoLFObbPn;
      string IPjSWPHokX;
      string iTAZnTNIwN;
      string iAZfJdVzWK;
      string ewdCrTTqrV;
      string oJMjfSuPWl;
      string ASeBFIoDIs;
      string sHSZrmoyhT;
      string AWQZYZnoAp;
      string hGcjKLQbxj;
      string dPhzbgplrs;
      string dxhCldCEcf;
      string TADRKmeiPf;
      string VNBxpjnQxq;
      string iCxHMkXZxf;
      string JLyQPdMQno;
      string BHVLWPwjMC;
      string pkHlwRDuEY;
      string OFkiWJOjhL;
      string XWcIobfljw;
      if(lwoLFObbPn == dPhzbgplrs){omNupXEMUy = true;}
      else if(dPhzbgplrs == lwoLFObbPn){BIhimZAJQu = true;}
      if(IPjSWPHokX == dxhCldCEcf){aQLPZwVlnM = true;}
      else if(dxhCldCEcf == IPjSWPHokX){fTysQToKlT = true;}
      if(iTAZnTNIwN == TADRKmeiPf){BLCAeSIyxU = true;}
      else if(TADRKmeiPf == iTAZnTNIwN){pRpWIbIWDH = true;}
      if(iAZfJdVzWK == VNBxpjnQxq){kQLIjaqosz = true;}
      else if(VNBxpjnQxq == iAZfJdVzWK){MgtmVyMgRn = true;}
      if(ewdCrTTqrV == iCxHMkXZxf){FCPjkGLMhr = true;}
      else if(iCxHMkXZxf == ewdCrTTqrV){rJWQbVeeFQ = true;}
      if(oJMjfSuPWl == JLyQPdMQno){GsyXuckNQW = true;}
      else if(JLyQPdMQno == oJMjfSuPWl){JbcDkiRLaK = true;}
      if(ASeBFIoDIs == BHVLWPwjMC){OFPAMJFDey = true;}
      else if(BHVLWPwjMC == ASeBFIoDIs){RcFIiyYLWS = true;}
      if(sHSZrmoyhT == pkHlwRDuEY){FfsnHzwROX = true;}
      if(AWQZYZnoAp == OFkiWJOjhL){GuQENrSmMH = true;}
      if(hGcjKLQbxj == XWcIobfljw){pdXVjzmPXb = true;}
      while(pkHlwRDuEY == sHSZrmoyhT){GIrxbgtYtl = true;}
      while(OFkiWJOjhL == OFkiWJOjhL){xcRmxjDqBl = true;}
      while(XWcIobfljw == XWcIobfljw){fBGGWODOhc = true;}
      if(omNupXEMUy == true){omNupXEMUy = false;}
      if(aQLPZwVlnM == true){aQLPZwVlnM = false;}
      if(BLCAeSIyxU == true){BLCAeSIyxU = false;}
      if(kQLIjaqosz == true){kQLIjaqosz = false;}
      if(FCPjkGLMhr == true){FCPjkGLMhr = false;}
      if(GsyXuckNQW == true){GsyXuckNQW = false;}
      if(OFPAMJFDey == true){OFPAMJFDey = false;}
      if(FfsnHzwROX == true){FfsnHzwROX = false;}
      if(GuQENrSmMH == true){GuQENrSmMH = false;}
      if(pdXVjzmPXb == true){pdXVjzmPXb = false;}
      if(BIhimZAJQu == true){BIhimZAJQu = false;}
      if(fTysQToKlT == true){fTysQToKlT = false;}
      if(pRpWIbIWDH == true){pRpWIbIWDH = false;}
      if(MgtmVyMgRn == true){MgtmVyMgRn = false;}
      if(rJWQbVeeFQ == true){rJWQbVeeFQ = false;}
      if(JbcDkiRLaK == true){JbcDkiRLaK = false;}
      if(RcFIiyYLWS == true){RcFIiyYLWS = false;}
      if(GIrxbgtYtl == true){GIrxbgtYtl = false;}
      if(xcRmxjDqBl == true){xcRmxjDqBl = false;}
      if(fBGGWODOhc == true){fBGGWODOhc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PKQASJZNPJ
{ 
  void HFapdDWRLO()
  { 
      bool daNUGLujeA = false;
      bool tixsdQhqYJ = false;
      bool qPQqGmKtdw = false;
      bool udLXGhXAmr = false;
      bool dNfGTEdsKG = false;
      bool GnRkMOJWOT = false;
      bool AAGiEQXYmw = false;
      bool shDffciJWB = false;
      bool IRjWoHLaLq = false;
      bool XAkSrcHxCD = false;
      bool fEGNNYjlEi = false;
      bool eAAMKiPisx = false;
      bool ExNuXrwknD = false;
      bool VlHTVELVJP = false;
      bool WZVcMXAXHI = false;
      bool NXDFyipKpV = false;
      bool bzxBMsrfqF = false;
      bool XtYcZeakiz = false;
      bool qYceoxhBVl = false;
      bool OAfnnWbUPS = false;
      string xEsyLtBTHI;
      string auxjcdUruR;
      string XXehjgiAhw;
      string wQtwsgPOSu;
      string bcYHeDblPJ;
      string NNzhgYZKNd;
      string LBkKMKAApw;
      string NFYzptyWDf;
      string FwkYihRMPT;
      string HFBnjaqNuU;
      string DauSuHoArl;
      string aCGhYOQMbe;
      string rnjVoHEkOO;
      string DjbicdARca;
      string sQTdsEySPU;
      string KelQbOpZdn;
      string CDkcipaXFt;
      string NyjBkbUGbj;
      string JlVtEYyLGN;
      string namoXofpuD;
      if(xEsyLtBTHI == DauSuHoArl){daNUGLujeA = true;}
      else if(DauSuHoArl == xEsyLtBTHI){fEGNNYjlEi = true;}
      if(auxjcdUruR == aCGhYOQMbe){tixsdQhqYJ = true;}
      else if(aCGhYOQMbe == auxjcdUruR){eAAMKiPisx = true;}
      if(XXehjgiAhw == rnjVoHEkOO){qPQqGmKtdw = true;}
      else if(rnjVoHEkOO == XXehjgiAhw){ExNuXrwknD = true;}
      if(wQtwsgPOSu == DjbicdARca){udLXGhXAmr = true;}
      else if(DjbicdARca == wQtwsgPOSu){VlHTVELVJP = true;}
      if(bcYHeDblPJ == sQTdsEySPU){dNfGTEdsKG = true;}
      else if(sQTdsEySPU == bcYHeDblPJ){WZVcMXAXHI = true;}
      if(NNzhgYZKNd == KelQbOpZdn){GnRkMOJWOT = true;}
      else if(KelQbOpZdn == NNzhgYZKNd){NXDFyipKpV = true;}
      if(LBkKMKAApw == CDkcipaXFt){AAGiEQXYmw = true;}
      else if(CDkcipaXFt == LBkKMKAApw){bzxBMsrfqF = true;}
      if(NFYzptyWDf == NyjBkbUGbj){shDffciJWB = true;}
      if(FwkYihRMPT == JlVtEYyLGN){IRjWoHLaLq = true;}
      if(HFBnjaqNuU == namoXofpuD){XAkSrcHxCD = true;}
      while(NyjBkbUGbj == NFYzptyWDf){XtYcZeakiz = true;}
      while(JlVtEYyLGN == JlVtEYyLGN){qYceoxhBVl = true;}
      while(namoXofpuD == namoXofpuD){OAfnnWbUPS = true;}
      if(daNUGLujeA == true){daNUGLujeA = false;}
      if(tixsdQhqYJ == true){tixsdQhqYJ = false;}
      if(qPQqGmKtdw == true){qPQqGmKtdw = false;}
      if(udLXGhXAmr == true){udLXGhXAmr = false;}
      if(dNfGTEdsKG == true){dNfGTEdsKG = false;}
      if(GnRkMOJWOT == true){GnRkMOJWOT = false;}
      if(AAGiEQXYmw == true){AAGiEQXYmw = false;}
      if(shDffciJWB == true){shDffciJWB = false;}
      if(IRjWoHLaLq == true){IRjWoHLaLq = false;}
      if(XAkSrcHxCD == true){XAkSrcHxCD = false;}
      if(fEGNNYjlEi == true){fEGNNYjlEi = false;}
      if(eAAMKiPisx == true){eAAMKiPisx = false;}
      if(ExNuXrwknD == true){ExNuXrwknD = false;}
      if(VlHTVELVJP == true){VlHTVELVJP = false;}
      if(WZVcMXAXHI == true){WZVcMXAXHI = false;}
      if(NXDFyipKpV == true){NXDFyipKpV = false;}
      if(bzxBMsrfqF == true){bzxBMsrfqF = false;}
      if(XtYcZeakiz == true){XtYcZeakiz = false;}
      if(qYceoxhBVl == true){qYceoxhBVl = false;}
      if(OAfnnWbUPS == true){OAfnnWbUPS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MUHTCMRIJK
{ 
  void NhFaTkfHMj()
  { 
      bool SooOxgQZBq = false;
      bool hVPMUPpmky = false;
      bool DkraGUWRLY = false;
      bool XMNVuKnRQx = false;
      bool gBeXCaNksI = false;
      bool GykmqgxNIb = false;
      bool KyOcSVHsQK = false;
      bool aXVnRBGUYe = false;
      bool HkYCRZaaeO = false;
      bool NaJnlhlBEg = false;
      bool TTUihhbxUo = false;
      bool ZbocUZZMKr = false;
      bool tfNBwzOhBi = false;
      bool PEPrLmQoUj = false;
      bool ycFpnkkgTM = false;
      bool IZqMlnGSlh = false;
      bool SsulZVMwQg = false;
      bool IjqatRZXML = false;
      bool NhNnZoNDFX = false;
      bool gxClQEgTQS = false;
      string KxfoJYZFwf;
      string FRZlCUJQzR;
      string pxIxlVkfmG;
      string AOKWEuCQbo;
      string ogAqlwDFgj;
      string QKTZzNnhDn;
      string msSpepHDhG;
      string RrnKbIwQLp;
      string SfzCopukWp;
      string HTNnBuNKMu;
      string nAWZSXWJxp;
      string QWWMVGtQAo;
      string uWfqzxezIg;
      string tkEPBRJYjQ;
      string TlmgZkxASt;
      string BxJXAQUfaU;
      string sWAQueSHLD;
      string xUgiVJizQO;
      string AdgNexbgOj;
      string xcyOXwkWkt;
      if(KxfoJYZFwf == nAWZSXWJxp){SooOxgQZBq = true;}
      else if(nAWZSXWJxp == KxfoJYZFwf){TTUihhbxUo = true;}
      if(FRZlCUJQzR == QWWMVGtQAo){hVPMUPpmky = true;}
      else if(QWWMVGtQAo == FRZlCUJQzR){ZbocUZZMKr = true;}
      if(pxIxlVkfmG == uWfqzxezIg){DkraGUWRLY = true;}
      else if(uWfqzxezIg == pxIxlVkfmG){tfNBwzOhBi = true;}
      if(AOKWEuCQbo == tkEPBRJYjQ){XMNVuKnRQx = true;}
      else if(tkEPBRJYjQ == AOKWEuCQbo){PEPrLmQoUj = true;}
      if(ogAqlwDFgj == TlmgZkxASt){gBeXCaNksI = true;}
      else if(TlmgZkxASt == ogAqlwDFgj){ycFpnkkgTM = true;}
      if(QKTZzNnhDn == BxJXAQUfaU){GykmqgxNIb = true;}
      else if(BxJXAQUfaU == QKTZzNnhDn){IZqMlnGSlh = true;}
      if(msSpepHDhG == sWAQueSHLD){KyOcSVHsQK = true;}
      else if(sWAQueSHLD == msSpepHDhG){SsulZVMwQg = true;}
      if(RrnKbIwQLp == xUgiVJizQO){aXVnRBGUYe = true;}
      if(SfzCopukWp == AdgNexbgOj){HkYCRZaaeO = true;}
      if(HTNnBuNKMu == xcyOXwkWkt){NaJnlhlBEg = true;}
      while(xUgiVJizQO == RrnKbIwQLp){IjqatRZXML = true;}
      while(AdgNexbgOj == AdgNexbgOj){NhNnZoNDFX = true;}
      while(xcyOXwkWkt == xcyOXwkWkt){gxClQEgTQS = true;}
      if(SooOxgQZBq == true){SooOxgQZBq = false;}
      if(hVPMUPpmky == true){hVPMUPpmky = false;}
      if(DkraGUWRLY == true){DkraGUWRLY = false;}
      if(XMNVuKnRQx == true){XMNVuKnRQx = false;}
      if(gBeXCaNksI == true){gBeXCaNksI = false;}
      if(GykmqgxNIb == true){GykmqgxNIb = false;}
      if(KyOcSVHsQK == true){KyOcSVHsQK = false;}
      if(aXVnRBGUYe == true){aXVnRBGUYe = false;}
      if(HkYCRZaaeO == true){HkYCRZaaeO = false;}
      if(NaJnlhlBEg == true){NaJnlhlBEg = false;}
      if(TTUihhbxUo == true){TTUihhbxUo = false;}
      if(ZbocUZZMKr == true){ZbocUZZMKr = false;}
      if(tfNBwzOhBi == true){tfNBwzOhBi = false;}
      if(PEPrLmQoUj == true){PEPrLmQoUj = false;}
      if(ycFpnkkgTM == true){ycFpnkkgTM = false;}
      if(IZqMlnGSlh == true){IZqMlnGSlh = false;}
      if(SsulZVMwQg == true){SsulZVMwQg = false;}
      if(IjqatRZXML == true){IjqatRZXML = false;}
      if(NhNnZoNDFX == true){NhNnZoNDFX = false;}
      if(gxClQEgTQS == true){gxClQEgTQS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DZLFXBIITZ
{ 
  void APFJzHlnWA()
  { 
      bool LJnfjpfsBP = false;
      bool iNGYVIFlTS = false;
      bool mQJVyjQUtV = false;
      bool lAioTbucfk = false;
      bool LzNHkhKrHh = false;
      bool ZmFmAuzRhz = false;
      bool ZLzecieRFs = false;
      bool McGhGmDCkP = false;
      bool YXMMwzuuoX = false;
      bool bdtfLAAhdC = false;
      bool DsmNUwgWgM = false;
      bool qxgRaBymhd = false;
      bool nicgRihZWF = false;
      bool mUXiMfkzqA = false;
      bool RXObgexbFK = false;
      bool CNAANUQFwk = false;
      bool NHAaZWYyFu = false;
      bool PzuhMPTAAl = false;
      bool lGUMDZUXUr = false;
      bool OeCFlKXnCJ = false;
      string HGCMTKmSXZ;
      string rMKSLbUWKJ;
      string DfuCGSckLM;
      string oXwJyzYGUU;
      string eTJgkWJymn;
      string VzscNaKzFB;
      string gCGKjTQATL;
      string DOCoeiWNRj;
      string IexWppeTuX;
      string nZATmcNywQ;
      string FIebkHxDxe;
      string ExhVsCEjTt;
      string nwFuWcOiBB;
      string UXQAdzAVki;
      string sbVPTqqeZk;
      string YddCcJDiEH;
      string rNCwJffhqt;
      string BhykBMZobe;
      string ljIPbqsOXU;
      string wRRdpeZZFm;
      if(HGCMTKmSXZ == FIebkHxDxe){LJnfjpfsBP = true;}
      else if(FIebkHxDxe == HGCMTKmSXZ){DsmNUwgWgM = true;}
      if(rMKSLbUWKJ == ExhVsCEjTt){iNGYVIFlTS = true;}
      else if(ExhVsCEjTt == rMKSLbUWKJ){qxgRaBymhd = true;}
      if(DfuCGSckLM == nwFuWcOiBB){mQJVyjQUtV = true;}
      else if(nwFuWcOiBB == DfuCGSckLM){nicgRihZWF = true;}
      if(oXwJyzYGUU == UXQAdzAVki){lAioTbucfk = true;}
      else if(UXQAdzAVki == oXwJyzYGUU){mUXiMfkzqA = true;}
      if(eTJgkWJymn == sbVPTqqeZk){LzNHkhKrHh = true;}
      else if(sbVPTqqeZk == eTJgkWJymn){RXObgexbFK = true;}
      if(VzscNaKzFB == YddCcJDiEH){ZmFmAuzRhz = true;}
      else if(YddCcJDiEH == VzscNaKzFB){CNAANUQFwk = true;}
      if(gCGKjTQATL == rNCwJffhqt){ZLzecieRFs = true;}
      else if(rNCwJffhqt == gCGKjTQATL){NHAaZWYyFu = true;}
      if(DOCoeiWNRj == BhykBMZobe){McGhGmDCkP = true;}
      if(IexWppeTuX == ljIPbqsOXU){YXMMwzuuoX = true;}
      if(nZATmcNywQ == wRRdpeZZFm){bdtfLAAhdC = true;}
      while(BhykBMZobe == DOCoeiWNRj){PzuhMPTAAl = true;}
      while(ljIPbqsOXU == ljIPbqsOXU){lGUMDZUXUr = true;}
      while(wRRdpeZZFm == wRRdpeZZFm){OeCFlKXnCJ = true;}
      if(LJnfjpfsBP == true){LJnfjpfsBP = false;}
      if(iNGYVIFlTS == true){iNGYVIFlTS = false;}
      if(mQJVyjQUtV == true){mQJVyjQUtV = false;}
      if(lAioTbucfk == true){lAioTbucfk = false;}
      if(LzNHkhKrHh == true){LzNHkhKrHh = false;}
      if(ZmFmAuzRhz == true){ZmFmAuzRhz = false;}
      if(ZLzecieRFs == true){ZLzecieRFs = false;}
      if(McGhGmDCkP == true){McGhGmDCkP = false;}
      if(YXMMwzuuoX == true){YXMMwzuuoX = false;}
      if(bdtfLAAhdC == true){bdtfLAAhdC = false;}
      if(DsmNUwgWgM == true){DsmNUwgWgM = false;}
      if(qxgRaBymhd == true){qxgRaBymhd = false;}
      if(nicgRihZWF == true){nicgRihZWF = false;}
      if(mUXiMfkzqA == true){mUXiMfkzqA = false;}
      if(RXObgexbFK == true){RXObgexbFK = false;}
      if(CNAANUQFwk == true){CNAANUQFwk = false;}
      if(NHAaZWYyFu == true){NHAaZWYyFu = false;}
      if(PzuhMPTAAl == true){PzuhMPTAAl = false;}
      if(lGUMDZUXUr == true){lGUMDZUXUr = false;}
      if(OeCFlKXnCJ == true){OeCFlKXnCJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CKXOCNSMWC
{ 
  void nbzkpGUyEE()
  { 
      bool VjeLYArPok = false;
      bool HJczrPkXPj = false;
      bool wASBDwSZkS = false;
      bool QFFkoUmZAS = false;
      bool niCxIeGuol = false;
      bool UEpKEUQdIb = false;
      bool gSdVTBDrKX = false;
      bool cKWxuMxVrX = false;
      bool pXkyyMPtfT = false;
      bool DIRXkVFfcH = false;
      bool bNkRJJgsOr = false;
      bool WuljlJgeXH = false;
      bool lJMpTUrzLZ = false;
      bool GRrkwclTon = false;
      bool eVenwHVhlb = false;
      bool naIVURAtxn = false;
      bool RhhokFDAmJ = false;
      bool WNpicPdYtL = false;
      bool yKNjkfIVPd = false;
      bool brVAWwnpCp = false;
      string tAtteELkBi;
      string LrADUFTuay;
      string zQeYlRaGms;
      string qqjaYiSGJM;
      string khkIQeucTX;
      string KApcPdVUIN;
      string cEyHrTIYdl;
      string mNlNMQeWIG;
      string VymOhpBGLu;
      string MxyujfjcPw;
      string WWyhkxNbQy;
      string yzMBqeRjYB;
      string SCMiYoOpdg;
      string gCGLjyzEck;
      string cZpMxNkwEE;
      string NzDOdTJPJw;
      string hWxiRurAQr;
      string fNGnYsMAlK;
      string ESlZSgzrhP;
      string oitgCFbGuB;
      if(tAtteELkBi == WWyhkxNbQy){VjeLYArPok = true;}
      else if(WWyhkxNbQy == tAtteELkBi){bNkRJJgsOr = true;}
      if(LrADUFTuay == yzMBqeRjYB){HJczrPkXPj = true;}
      else if(yzMBqeRjYB == LrADUFTuay){WuljlJgeXH = true;}
      if(zQeYlRaGms == SCMiYoOpdg){wASBDwSZkS = true;}
      else if(SCMiYoOpdg == zQeYlRaGms){lJMpTUrzLZ = true;}
      if(qqjaYiSGJM == gCGLjyzEck){QFFkoUmZAS = true;}
      else if(gCGLjyzEck == qqjaYiSGJM){GRrkwclTon = true;}
      if(khkIQeucTX == cZpMxNkwEE){niCxIeGuol = true;}
      else if(cZpMxNkwEE == khkIQeucTX){eVenwHVhlb = true;}
      if(KApcPdVUIN == NzDOdTJPJw){UEpKEUQdIb = true;}
      else if(NzDOdTJPJw == KApcPdVUIN){naIVURAtxn = true;}
      if(cEyHrTIYdl == hWxiRurAQr){gSdVTBDrKX = true;}
      else if(hWxiRurAQr == cEyHrTIYdl){RhhokFDAmJ = true;}
      if(mNlNMQeWIG == fNGnYsMAlK){cKWxuMxVrX = true;}
      if(VymOhpBGLu == ESlZSgzrhP){pXkyyMPtfT = true;}
      if(MxyujfjcPw == oitgCFbGuB){DIRXkVFfcH = true;}
      while(fNGnYsMAlK == mNlNMQeWIG){WNpicPdYtL = true;}
      while(ESlZSgzrhP == ESlZSgzrhP){yKNjkfIVPd = true;}
      while(oitgCFbGuB == oitgCFbGuB){brVAWwnpCp = true;}
      if(VjeLYArPok == true){VjeLYArPok = false;}
      if(HJczrPkXPj == true){HJczrPkXPj = false;}
      if(wASBDwSZkS == true){wASBDwSZkS = false;}
      if(QFFkoUmZAS == true){QFFkoUmZAS = false;}
      if(niCxIeGuol == true){niCxIeGuol = false;}
      if(UEpKEUQdIb == true){UEpKEUQdIb = false;}
      if(gSdVTBDrKX == true){gSdVTBDrKX = false;}
      if(cKWxuMxVrX == true){cKWxuMxVrX = false;}
      if(pXkyyMPtfT == true){pXkyyMPtfT = false;}
      if(DIRXkVFfcH == true){DIRXkVFfcH = false;}
      if(bNkRJJgsOr == true){bNkRJJgsOr = false;}
      if(WuljlJgeXH == true){WuljlJgeXH = false;}
      if(lJMpTUrzLZ == true){lJMpTUrzLZ = false;}
      if(GRrkwclTon == true){GRrkwclTon = false;}
      if(eVenwHVhlb == true){eVenwHVhlb = false;}
      if(naIVURAtxn == true){naIVURAtxn = false;}
      if(RhhokFDAmJ == true){RhhokFDAmJ = false;}
      if(WNpicPdYtL == true){WNpicPdYtL = false;}
      if(yKNjkfIVPd == true){yKNjkfIVPd = false;}
      if(brVAWwnpCp == true){brVAWwnpCp = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UXBAPQWYSD
{ 
  void jXCZoXXwWp()
  { 
      bool TKKiHhattw = false;
      bool DSPUhaakjI = false;
      bool oCYjuWZpJO = false;
      bool aVnbLzwxyI = false;
      bool kkJJolyWNy = false;
      bool KHgAYRlamj = false;
      bool IkjfUhemJi = false;
      bool edmaXDrOtI = false;
      bool XCbKDTomCs = false;
      bool MUJNAzAdnH = false;
      bool GLMdSgTFcU = false;
      bool YXfeMFIZbB = false;
      bool rqjPWJTYaX = false;
      bool DOZmCMoyqW = false;
      bool zjKhUnoeBt = false;
      bool qaCmPPaxHJ = false;
      bool MudERKysbA = false;
      bool PKhECHfCth = false;
      bool UoWggbLlXl = false;
      bool krphJDGGPK = false;
      string botiGcFajS;
      string pMeGUlOQAR;
      string CWrwPQVwUI;
      string HLYcReqTuE;
      string SruBBJyUwm;
      string XqkQDExxSp;
      string GJGbKenCno;
      string tuUwzJbptG;
      string SBzgkPZIYN;
      string hsAFyIkTpW;
      string VugHscWoQs;
      string cGDMWFJYpt;
      string FedaGAtOyp;
      string YIAXjWUrnH;
      string sjqejapmir;
      string zMcuQqRGAr;
      string hHqaUUGecA;
      string kQkiBUmypf;
      string KGfFqLeEGs;
      string FOquZlsQJO;
      if(botiGcFajS == VugHscWoQs){TKKiHhattw = true;}
      else if(VugHscWoQs == botiGcFajS){GLMdSgTFcU = true;}
      if(pMeGUlOQAR == cGDMWFJYpt){DSPUhaakjI = true;}
      else if(cGDMWFJYpt == pMeGUlOQAR){YXfeMFIZbB = true;}
      if(CWrwPQVwUI == FedaGAtOyp){oCYjuWZpJO = true;}
      else if(FedaGAtOyp == CWrwPQVwUI){rqjPWJTYaX = true;}
      if(HLYcReqTuE == YIAXjWUrnH){aVnbLzwxyI = true;}
      else if(YIAXjWUrnH == HLYcReqTuE){DOZmCMoyqW = true;}
      if(SruBBJyUwm == sjqejapmir){kkJJolyWNy = true;}
      else if(sjqejapmir == SruBBJyUwm){zjKhUnoeBt = true;}
      if(XqkQDExxSp == zMcuQqRGAr){KHgAYRlamj = true;}
      else if(zMcuQqRGAr == XqkQDExxSp){qaCmPPaxHJ = true;}
      if(GJGbKenCno == hHqaUUGecA){IkjfUhemJi = true;}
      else if(hHqaUUGecA == GJGbKenCno){MudERKysbA = true;}
      if(tuUwzJbptG == kQkiBUmypf){edmaXDrOtI = true;}
      if(SBzgkPZIYN == KGfFqLeEGs){XCbKDTomCs = true;}
      if(hsAFyIkTpW == FOquZlsQJO){MUJNAzAdnH = true;}
      while(kQkiBUmypf == tuUwzJbptG){PKhECHfCth = true;}
      while(KGfFqLeEGs == KGfFqLeEGs){UoWggbLlXl = true;}
      while(FOquZlsQJO == FOquZlsQJO){krphJDGGPK = true;}
      if(TKKiHhattw == true){TKKiHhattw = false;}
      if(DSPUhaakjI == true){DSPUhaakjI = false;}
      if(oCYjuWZpJO == true){oCYjuWZpJO = false;}
      if(aVnbLzwxyI == true){aVnbLzwxyI = false;}
      if(kkJJolyWNy == true){kkJJolyWNy = false;}
      if(KHgAYRlamj == true){KHgAYRlamj = false;}
      if(IkjfUhemJi == true){IkjfUhemJi = false;}
      if(edmaXDrOtI == true){edmaXDrOtI = false;}
      if(XCbKDTomCs == true){XCbKDTomCs = false;}
      if(MUJNAzAdnH == true){MUJNAzAdnH = false;}
      if(GLMdSgTFcU == true){GLMdSgTFcU = false;}
      if(YXfeMFIZbB == true){YXfeMFIZbB = false;}
      if(rqjPWJTYaX == true){rqjPWJTYaX = false;}
      if(DOZmCMoyqW == true){DOZmCMoyqW = false;}
      if(zjKhUnoeBt == true){zjKhUnoeBt = false;}
      if(qaCmPPaxHJ == true){qaCmPPaxHJ = false;}
      if(MudERKysbA == true){MudERKysbA = false;}
      if(PKhECHfCth == true){PKhECHfCth = false;}
      if(UoWggbLlXl == true){UoWggbLlXl = false;}
      if(krphJDGGPK == true){krphJDGGPK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VPZSCNGLSY
{ 
  void iCQrpADZMu()
  { 
      bool MrFuYJXQnY = false;
      bool bCuXApsSBp = false;
      bool pxUMJkEuwE = false;
      bool SLiVOEifKd = false;
      bool lpJpGEKUcd = false;
      bool oZrFlOXsln = false;
      bool FbUjGPSYJu = false;
      bool HNHWEAmeFE = false;
      bool ajSSOsTdiz = false;
      bool gpsWYJUZjp = false;
      bool kkLgWbRUBl = false;
      bool armgMVJCUN = false;
      bool wopJORTkyt = false;
      bool eNuPToYkum = false;
      bool azcxYnCMJC = false;
      bool ASEWtwuBSE = false;
      bool HfVGHNxXMX = false;
      bool xYGhlxMrjZ = false;
      bool OGbwtLIYVX = false;
      bool tWSqetcVpK = false;
      string rjVMoYwQRc;
      string NEBWMgHVTG;
      string HSSqPnSdOj;
      string YIwTlwrHRM;
      string OBeYydpCAN;
      string SLJbOZQdwB;
      string GSCgEFIgjP;
      string ozDGqGmtsk;
      string xQcARMyTFM;
      string eNLQGwyxTr;
      string VJRjYSIDkI;
      string lWkKEpoBPr;
      string HnbXkanmAu;
      string HoZAPnzrby;
      string EuiwZIcALQ;
      string Gxhbkggszy;
      string dUCoAxwjuQ;
      string qwYQmLFeyb;
      string xnajnKHyPh;
      string GFrdTirswb;
      if(rjVMoYwQRc == VJRjYSIDkI){MrFuYJXQnY = true;}
      else if(VJRjYSIDkI == rjVMoYwQRc){kkLgWbRUBl = true;}
      if(NEBWMgHVTG == lWkKEpoBPr){bCuXApsSBp = true;}
      else if(lWkKEpoBPr == NEBWMgHVTG){armgMVJCUN = true;}
      if(HSSqPnSdOj == HnbXkanmAu){pxUMJkEuwE = true;}
      else if(HnbXkanmAu == HSSqPnSdOj){wopJORTkyt = true;}
      if(YIwTlwrHRM == HoZAPnzrby){SLiVOEifKd = true;}
      else if(HoZAPnzrby == YIwTlwrHRM){eNuPToYkum = true;}
      if(OBeYydpCAN == EuiwZIcALQ){lpJpGEKUcd = true;}
      else if(EuiwZIcALQ == OBeYydpCAN){azcxYnCMJC = true;}
      if(SLJbOZQdwB == Gxhbkggszy){oZrFlOXsln = true;}
      else if(Gxhbkggszy == SLJbOZQdwB){ASEWtwuBSE = true;}
      if(GSCgEFIgjP == dUCoAxwjuQ){FbUjGPSYJu = true;}
      else if(dUCoAxwjuQ == GSCgEFIgjP){HfVGHNxXMX = true;}
      if(ozDGqGmtsk == qwYQmLFeyb){HNHWEAmeFE = true;}
      if(xQcARMyTFM == xnajnKHyPh){ajSSOsTdiz = true;}
      if(eNLQGwyxTr == GFrdTirswb){gpsWYJUZjp = true;}
      while(qwYQmLFeyb == ozDGqGmtsk){xYGhlxMrjZ = true;}
      while(xnajnKHyPh == xnajnKHyPh){OGbwtLIYVX = true;}
      while(GFrdTirswb == GFrdTirswb){tWSqetcVpK = true;}
      if(MrFuYJXQnY == true){MrFuYJXQnY = false;}
      if(bCuXApsSBp == true){bCuXApsSBp = false;}
      if(pxUMJkEuwE == true){pxUMJkEuwE = false;}
      if(SLiVOEifKd == true){SLiVOEifKd = false;}
      if(lpJpGEKUcd == true){lpJpGEKUcd = false;}
      if(oZrFlOXsln == true){oZrFlOXsln = false;}
      if(FbUjGPSYJu == true){FbUjGPSYJu = false;}
      if(HNHWEAmeFE == true){HNHWEAmeFE = false;}
      if(ajSSOsTdiz == true){ajSSOsTdiz = false;}
      if(gpsWYJUZjp == true){gpsWYJUZjp = false;}
      if(kkLgWbRUBl == true){kkLgWbRUBl = false;}
      if(armgMVJCUN == true){armgMVJCUN = false;}
      if(wopJORTkyt == true){wopJORTkyt = false;}
      if(eNuPToYkum == true){eNuPToYkum = false;}
      if(azcxYnCMJC == true){azcxYnCMJC = false;}
      if(ASEWtwuBSE == true){ASEWtwuBSE = false;}
      if(HfVGHNxXMX == true){HfVGHNxXMX = false;}
      if(xYGhlxMrjZ == true){xYGhlxMrjZ = false;}
      if(OGbwtLIYVX == true){OGbwtLIYVX = false;}
      if(tWSqetcVpK == true){tWSqetcVpK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CWKBSWONCH
{ 
  void JPpxPPUEGa()
  { 
      bool KZRlPqSjJo = false;
      bool gYHPFpXOzd = false;
      bool CRPegadGpk = false;
      bool xeKSRnliBG = false;
      bool KGfXFPlQes = false;
      bool iYEDarJmEo = false;
      bool NLsXrBOoxR = false;
      bool EoMrTwlrYR = false;
      bool oMbxCXZHqZ = false;
      bool dzdLryMHfD = false;
      bool BxSNosLMid = false;
      bool FRNmAAjwVT = false;
      bool bHxXnFKBzV = false;
      bool jeMVPrsOIJ = false;
      bool MNXNTLkqLr = false;
      bool NQlLbszuZi = false;
      bool iKMNtaFrWI = false;
      bool HUmLGNjjEb = false;
      bool TSoHHSrAsW = false;
      bool ryoYaBBJSr = false;
      string CffLHNSiwU;
      string mZsJUgLkob;
      string etsyoXbSCg;
      string HinZZFzXUm;
      string mkeHglRHDQ;
      string UkimoIlgVi;
      string kzfPNDkSjO;
      string hnPCnsOlQC;
      string FUOlLVzAkQ;
      string SRBlBmEOtd;
      string AzWoigVgyu;
      string sAPScQktEq;
      string DkhTuPlcBA;
      string nArmCGxZDd;
      string zSBoWSCiIj;
      string CKKCZPRDsS;
      string trexHsHcQW;
      string TKqZrkirwc;
      string NDUQycppMl;
      string osgjjGmEhd;
      if(CffLHNSiwU == AzWoigVgyu){KZRlPqSjJo = true;}
      else if(AzWoigVgyu == CffLHNSiwU){BxSNosLMid = true;}
      if(mZsJUgLkob == sAPScQktEq){gYHPFpXOzd = true;}
      else if(sAPScQktEq == mZsJUgLkob){FRNmAAjwVT = true;}
      if(etsyoXbSCg == DkhTuPlcBA){CRPegadGpk = true;}
      else if(DkhTuPlcBA == etsyoXbSCg){bHxXnFKBzV = true;}
      if(HinZZFzXUm == nArmCGxZDd){xeKSRnliBG = true;}
      else if(nArmCGxZDd == HinZZFzXUm){jeMVPrsOIJ = true;}
      if(mkeHglRHDQ == zSBoWSCiIj){KGfXFPlQes = true;}
      else if(zSBoWSCiIj == mkeHglRHDQ){MNXNTLkqLr = true;}
      if(UkimoIlgVi == CKKCZPRDsS){iYEDarJmEo = true;}
      else if(CKKCZPRDsS == UkimoIlgVi){NQlLbszuZi = true;}
      if(kzfPNDkSjO == trexHsHcQW){NLsXrBOoxR = true;}
      else if(trexHsHcQW == kzfPNDkSjO){iKMNtaFrWI = true;}
      if(hnPCnsOlQC == TKqZrkirwc){EoMrTwlrYR = true;}
      if(FUOlLVzAkQ == NDUQycppMl){oMbxCXZHqZ = true;}
      if(SRBlBmEOtd == osgjjGmEhd){dzdLryMHfD = true;}
      while(TKqZrkirwc == hnPCnsOlQC){HUmLGNjjEb = true;}
      while(NDUQycppMl == NDUQycppMl){TSoHHSrAsW = true;}
      while(osgjjGmEhd == osgjjGmEhd){ryoYaBBJSr = true;}
      if(KZRlPqSjJo == true){KZRlPqSjJo = false;}
      if(gYHPFpXOzd == true){gYHPFpXOzd = false;}
      if(CRPegadGpk == true){CRPegadGpk = false;}
      if(xeKSRnliBG == true){xeKSRnliBG = false;}
      if(KGfXFPlQes == true){KGfXFPlQes = false;}
      if(iYEDarJmEo == true){iYEDarJmEo = false;}
      if(NLsXrBOoxR == true){NLsXrBOoxR = false;}
      if(EoMrTwlrYR == true){EoMrTwlrYR = false;}
      if(oMbxCXZHqZ == true){oMbxCXZHqZ = false;}
      if(dzdLryMHfD == true){dzdLryMHfD = false;}
      if(BxSNosLMid == true){BxSNosLMid = false;}
      if(FRNmAAjwVT == true){FRNmAAjwVT = false;}
      if(bHxXnFKBzV == true){bHxXnFKBzV = false;}
      if(jeMVPrsOIJ == true){jeMVPrsOIJ = false;}
      if(MNXNTLkqLr == true){MNXNTLkqLr = false;}
      if(NQlLbszuZi == true){NQlLbszuZi = false;}
      if(iKMNtaFrWI == true){iKMNtaFrWI = false;}
      if(HUmLGNjjEb == true){HUmLGNjjEb = false;}
      if(TSoHHSrAsW == true){TSoHHSrAsW = false;}
      if(ryoYaBBJSr == true){ryoYaBBJSr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BINABGHFSP
{ 
  void yxTqTiQDzu()
  { 
      bool FrGVbfyrOD = false;
      bool fameGpyjTw = false;
      bool AbwPAbelwx = false;
      bool fGgxXdeBLY = false;
      bool RPwYOPhEBc = false;
      bool TJsCXdwqVA = false;
      bool BMwRXZtoGc = false;
      bool KxYtekfWAk = false;
      bool tajTdWMeYt = false;
      bool hmfLXETeBo = false;
      bool UrxYRWkyWm = false;
      bool yZqjGZbcnB = false;
      bool MOPMtWRMVZ = false;
      bool QBUKPBdLBH = false;
      bool lOHlwIwuFy = false;
      bool cpqAcGbjdt = false;
      bool EXbxVwEfUE = false;
      bool uQagPxHdQl = false;
      bool aVHXxaKThJ = false;
      bool YsgHCNlKPz = false;
      string lAMZUrfxKi;
      string xMWdMAxfgn;
      string yeBAXeWyKc;
      string lAssPULXsU;
      string jMydUYecYG;
      string DsxeIGzMCp;
      string UglhwwTcOr;
      string nYmMEkgWey;
      string AsYEYsDNjI;
      string UzVCACaQgX;
      string EBOgSeFkaX;
      string ZbgEpaHgID;
      string sFsFWnDjMB;
      string KPoHsydrlt;
      string mSlMisRQMY;
      string usHddzHJJd;
      string bVXZNJmYKq;
      string wOtOPaSgWd;
      string WlWmFQIKct;
      string pUiISRtMzY;
      if(lAMZUrfxKi == EBOgSeFkaX){FrGVbfyrOD = true;}
      else if(EBOgSeFkaX == lAMZUrfxKi){UrxYRWkyWm = true;}
      if(xMWdMAxfgn == ZbgEpaHgID){fameGpyjTw = true;}
      else if(ZbgEpaHgID == xMWdMAxfgn){yZqjGZbcnB = true;}
      if(yeBAXeWyKc == sFsFWnDjMB){AbwPAbelwx = true;}
      else if(sFsFWnDjMB == yeBAXeWyKc){MOPMtWRMVZ = true;}
      if(lAssPULXsU == KPoHsydrlt){fGgxXdeBLY = true;}
      else if(KPoHsydrlt == lAssPULXsU){QBUKPBdLBH = true;}
      if(jMydUYecYG == mSlMisRQMY){RPwYOPhEBc = true;}
      else if(mSlMisRQMY == jMydUYecYG){lOHlwIwuFy = true;}
      if(DsxeIGzMCp == usHddzHJJd){TJsCXdwqVA = true;}
      else if(usHddzHJJd == DsxeIGzMCp){cpqAcGbjdt = true;}
      if(UglhwwTcOr == bVXZNJmYKq){BMwRXZtoGc = true;}
      else if(bVXZNJmYKq == UglhwwTcOr){EXbxVwEfUE = true;}
      if(nYmMEkgWey == wOtOPaSgWd){KxYtekfWAk = true;}
      if(AsYEYsDNjI == WlWmFQIKct){tajTdWMeYt = true;}
      if(UzVCACaQgX == pUiISRtMzY){hmfLXETeBo = true;}
      while(wOtOPaSgWd == nYmMEkgWey){uQagPxHdQl = true;}
      while(WlWmFQIKct == WlWmFQIKct){aVHXxaKThJ = true;}
      while(pUiISRtMzY == pUiISRtMzY){YsgHCNlKPz = true;}
      if(FrGVbfyrOD == true){FrGVbfyrOD = false;}
      if(fameGpyjTw == true){fameGpyjTw = false;}
      if(AbwPAbelwx == true){AbwPAbelwx = false;}
      if(fGgxXdeBLY == true){fGgxXdeBLY = false;}
      if(RPwYOPhEBc == true){RPwYOPhEBc = false;}
      if(TJsCXdwqVA == true){TJsCXdwqVA = false;}
      if(BMwRXZtoGc == true){BMwRXZtoGc = false;}
      if(KxYtekfWAk == true){KxYtekfWAk = false;}
      if(tajTdWMeYt == true){tajTdWMeYt = false;}
      if(hmfLXETeBo == true){hmfLXETeBo = false;}
      if(UrxYRWkyWm == true){UrxYRWkyWm = false;}
      if(yZqjGZbcnB == true){yZqjGZbcnB = false;}
      if(MOPMtWRMVZ == true){MOPMtWRMVZ = false;}
      if(QBUKPBdLBH == true){QBUKPBdLBH = false;}
      if(lOHlwIwuFy == true){lOHlwIwuFy = false;}
      if(cpqAcGbjdt == true){cpqAcGbjdt = false;}
      if(EXbxVwEfUE == true){EXbxVwEfUE = false;}
      if(uQagPxHdQl == true){uQagPxHdQl = false;}
      if(aVHXxaKThJ == true){aVHXxaKThJ = false;}
      if(YsgHCNlKPz == true){YsgHCNlKPz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CLGGDEKNJW
{ 
  void XxXHjcRExL()
  { 
      bool rxumIILIPa = false;
      bool fbMDwEUdyO = false;
      bool diZrnxCCnP = false;
      bool fqOpnNTOtA = false;
      bool GilOBjdmCC = false;
      bool pJEshJXCyK = false;
      bool FJTAtnNWOi = false;
      bool jFiyItwKqE = false;
      bool TPzlZWmVHe = false;
      bool oQQitwPlTM = false;
      bool VHXTRjaFjn = false;
      bool MMLsqoJgZX = false;
      bool ArOitToADM = false;
      bool eJpdiMqfTo = false;
      bool uglTRbFXrE = false;
      bool lhLuMdroZy = false;
      bool tZCXIDVgKe = false;
      bool ydNjcSFtQR = false;
      bool OQIyZxxuRE = false;
      bool TCGVtUeOLO = false;
      string lFfIdCXDeu;
      string eyYWpdIawp;
      string SzmFptxhHa;
      string ujVMlYpUjA;
      string LRdxgGXmMT;
      string tCewLlSaRi;
      string FpwDAtcMkR;
      string nnpweAVoYj;
      string WJVmgsEZAz;
      string GPCAtVVZxl;
      string cKCufFLiQL;
      string QcCgMwTIWH;
      string pDLlKXkoBJ;
      string eurlHLHaHa;
      string gVIohcNVny;
      string BMDpwZzPjH;
      string TWgGgVkPZB;
      string osBuZUtWDg;
      string KPBDIRqCpg;
      string MqpzGKnCHM;
      if(lFfIdCXDeu == cKCufFLiQL){rxumIILIPa = true;}
      else if(cKCufFLiQL == lFfIdCXDeu){VHXTRjaFjn = true;}
      if(eyYWpdIawp == QcCgMwTIWH){fbMDwEUdyO = true;}
      else if(QcCgMwTIWH == eyYWpdIawp){MMLsqoJgZX = true;}
      if(SzmFptxhHa == pDLlKXkoBJ){diZrnxCCnP = true;}
      else if(pDLlKXkoBJ == SzmFptxhHa){ArOitToADM = true;}
      if(ujVMlYpUjA == eurlHLHaHa){fqOpnNTOtA = true;}
      else if(eurlHLHaHa == ujVMlYpUjA){eJpdiMqfTo = true;}
      if(LRdxgGXmMT == gVIohcNVny){GilOBjdmCC = true;}
      else if(gVIohcNVny == LRdxgGXmMT){uglTRbFXrE = true;}
      if(tCewLlSaRi == BMDpwZzPjH){pJEshJXCyK = true;}
      else if(BMDpwZzPjH == tCewLlSaRi){lhLuMdroZy = true;}
      if(FpwDAtcMkR == TWgGgVkPZB){FJTAtnNWOi = true;}
      else if(TWgGgVkPZB == FpwDAtcMkR){tZCXIDVgKe = true;}
      if(nnpweAVoYj == osBuZUtWDg){jFiyItwKqE = true;}
      if(WJVmgsEZAz == KPBDIRqCpg){TPzlZWmVHe = true;}
      if(GPCAtVVZxl == MqpzGKnCHM){oQQitwPlTM = true;}
      while(osBuZUtWDg == nnpweAVoYj){ydNjcSFtQR = true;}
      while(KPBDIRqCpg == KPBDIRqCpg){OQIyZxxuRE = true;}
      while(MqpzGKnCHM == MqpzGKnCHM){TCGVtUeOLO = true;}
      if(rxumIILIPa == true){rxumIILIPa = false;}
      if(fbMDwEUdyO == true){fbMDwEUdyO = false;}
      if(diZrnxCCnP == true){diZrnxCCnP = false;}
      if(fqOpnNTOtA == true){fqOpnNTOtA = false;}
      if(GilOBjdmCC == true){GilOBjdmCC = false;}
      if(pJEshJXCyK == true){pJEshJXCyK = false;}
      if(FJTAtnNWOi == true){FJTAtnNWOi = false;}
      if(jFiyItwKqE == true){jFiyItwKqE = false;}
      if(TPzlZWmVHe == true){TPzlZWmVHe = false;}
      if(oQQitwPlTM == true){oQQitwPlTM = false;}
      if(VHXTRjaFjn == true){VHXTRjaFjn = false;}
      if(MMLsqoJgZX == true){MMLsqoJgZX = false;}
      if(ArOitToADM == true){ArOitToADM = false;}
      if(eJpdiMqfTo == true){eJpdiMqfTo = false;}
      if(uglTRbFXrE == true){uglTRbFXrE = false;}
      if(lhLuMdroZy == true){lhLuMdroZy = false;}
      if(tZCXIDVgKe == true){tZCXIDVgKe = false;}
      if(ydNjcSFtQR == true){ydNjcSFtQR = false;}
      if(OQIyZxxuRE == true){OQIyZxxuRE = false;}
      if(TCGVtUeOLO == true){TCGVtUeOLO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FGFAZAQHSX
{ 
  void iIGFpPPpLQ()
  { 
      bool ljUemAKArV = false;
      bool NLbVgrwNpB = false;
      bool HFcegEEzZJ = false;
      bool ZJXZIBahBf = false;
      bool JaJmJCqFaD = false;
      bool XMJcHgGoyh = false;
      bool uLZhwEnCNe = false;
      bool RNLJqfupUd = false;
      bool MzXWlNztRa = false;
      bool ukjTjyBbqF = false;
      bool OoqulJBAlb = false;
      bool lQUUUwmCui = false;
      bool EskmjfdrYr = false;
      bool dSyPPXHckc = false;
      bool LzHoodQPyn = false;
      bool KygIAUEELF = false;
      bool eJasxSSEEx = false;
      bool oKRSWEkuxF = false;
      bool POskgflpPt = false;
      bool TDRexCXOrV = false;
      string LwgogVoIhp;
      string PsPOVeJruu;
      string UAOkpTTSYl;
      string GGHDrHiUIX;
      string ciYMHpbbTW;
      string ufaBtKhuyN;
      string BWiwiIlLVO;
      string uniWSlbCdA;
      string qqyBpafFJh;
      string ZmVuVFzcdy;
      string EPFFaGxDiI;
      string ifRyVKYhFH;
      string cfCnnHSmAM;
      string lzXewakrai;
      string dcVAztVWZL;
      string fRWVVXNRQS;
      string nJCVViHlCt;
      string YfGVdAJCot;
      string luNofUbCJX;
      string NTbHuTZzeP;
      if(LwgogVoIhp == EPFFaGxDiI){ljUemAKArV = true;}
      else if(EPFFaGxDiI == LwgogVoIhp){OoqulJBAlb = true;}
      if(PsPOVeJruu == ifRyVKYhFH){NLbVgrwNpB = true;}
      else if(ifRyVKYhFH == PsPOVeJruu){lQUUUwmCui = true;}
      if(UAOkpTTSYl == cfCnnHSmAM){HFcegEEzZJ = true;}
      else if(cfCnnHSmAM == UAOkpTTSYl){EskmjfdrYr = true;}
      if(GGHDrHiUIX == lzXewakrai){ZJXZIBahBf = true;}
      else if(lzXewakrai == GGHDrHiUIX){dSyPPXHckc = true;}
      if(ciYMHpbbTW == dcVAztVWZL){JaJmJCqFaD = true;}
      else if(dcVAztVWZL == ciYMHpbbTW){LzHoodQPyn = true;}
      if(ufaBtKhuyN == fRWVVXNRQS){XMJcHgGoyh = true;}
      else if(fRWVVXNRQS == ufaBtKhuyN){KygIAUEELF = true;}
      if(BWiwiIlLVO == nJCVViHlCt){uLZhwEnCNe = true;}
      else if(nJCVViHlCt == BWiwiIlLVO){eJasxSSEEx = true;}
      if(uniWSlbCdA == YfGVdAJCot){RNLJqfupUd = true;}
      if(qqyBpafFJh == luNofUbCJX){MzXWlNztRa = true;}
      if(ZmVuVFzcdy == NTbHuTZzeP){ukjTjyBbqF = true;}
      while(YfGVdAJCot == uniWSlbCdA){oKRSWEkuxF = true;}
      while(luNofUbCJX == luNofUbCJX){POskgflpPt = true;}
      while(NTbHuTZzeP == NTbHuTZzeP){TDRexCXOrV = true;}
      if(ljUemAKArV == true){ljUemAKArV = false;}
      if(NLbVgrwNpB == true){NLbVgrwNpB = false;}
      if(HFcegEEzZJ == true){HFcegEEzZJ = false;}
      if(ZJXZIBahBf == true){ZJXZIBahBf = false;}
      if(JaJmJCqFaD == true){JaJmJCqFaD = false;}
      if(XMJcHgGoyh == true){XMJcHgGoyh = false;}
      if(uLZhwEnCNe == true){uLZhwEnCNe = false;}
      if(RNLJqfupUd == true){RNLJqfupUd = false;}
      if(MzXWlNztRa == true){MzXWlNztRa = false;}
      if(ukjTjyBbqF == true){ukjTjyBbqF = false;}
      if(OoqulJBAlb == true){OoqulJBAlb = false;}
      if(lQUUUwmCui == true){lQUUUwmCui = false;}
      if(EskmjfdrYr == true){EskmjfdrYr = false;}
      if(dSyPPXHckc == true){dSyPPXHckc = false;}
      if(LzHoodQPyn == true){LzHoodQPyn = false;}
      if(KygIAUEELF == true){KygIAUEELF = false;}
      if(eJasxSSEEx == true){eJasxSSEEx = false;}
      if(oKRSWEkuxF == true){oKRSWEkuxF = false;}
      if(POskgflpPt == true){POskgflpPt = false;}
      if(TDRexCXOrV == true){TDRexCXOrV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BOLKAAJRFF
{ 
  void ONmyPxjFNf()
  { 
      bool jzLqnlPFdG = false;
      bool dEBRzGqhCq = false;
      bool InGQKTCzht = false;
      bool ausrWeFsro = false;
      bool xbGzfrVJdy = false;
      bool NDRrxurtfT = false;
      bool jQWieiKJuT = false;
      bool BxpMsfhGjS = false;
      bool yrUjgWdAua = false;
      bool mKNYFgEXTx = false;
      bool gLOJwQNEzQ = false;
      bool NkKNUqumCI = false;
      bool FJOTgqjQGL = false;
      bool bKmlklcKmP = false;
      bool IYBdNCAzDN = false;
      bool lRULrXxtcF = false;
      bool gxynoDyXch = false;
      bool NjzlAfIjGe = false;
      bool pjMrZJobtN = false;
      bool oihhiLZNoK = false;
      string TSdaDCWEPo;
      string ALMdaiVMdL;
      string iOFippWynx;
      string RIfwTLsBuY;
      string VwcgZiOFFK;
      string tafMxEMukM;
      string fYQSWqePKP;
      string oTQEpllYIG;
      string bRrdCpcTyP;
      string yyQhsxfJJi;
      string DytpUZKaWK;
      string SnldjNflQf;
      string OLQuQpChLr;
      string GbOZCdWBIb;
      string fBxnwJLlye;
      string QssdTrgzYd;
      string WbOdEqZMaG;
      string qednzNOcyG;
      string eQjgbNhkFK;
      string wWbdFElTiF;
      if(TSdaDCWEPo == DytpUZKaWK){jzLqnlPFdG = true;}
      else if(DytpUZKaWK == TSdaDCWEPo){gLOJwQNEzQ = true;}
      if(ALMdaiVMdL == SnldjNflQf){dEBRzGqhCq = true;}
      else if(SnldjNflQf == ALMdaiVMdL){NkKNUqumCI = true;}
      if(iOFippWynx == OLQuQpChLr){InGQKTCzht = true;}
      else if(OLQuQpChLr == iOFippWynx){FJOTgqjQGL = true;}
      if(RIfwTLsBuY == GbOZCdWBIb){ausrWeFsro = true;}
      else if(GbOZCdWBIb == RIfwTLsBuY){bKmlklcKmP = true;}
      if(VwcgZiOFFK == fBxnwJLlye){xbGzfrVJdy = true;}
      else if(fBxnwJLlye == VwcgZiOFFK){IYBdNCAzDN = true;}
      if(tafMxEMukM == QssdTrgzYd){NDRrxurtfT = true;}
      else if(QssdTrgzYd == tafMxEMukM){lRULrXxtcF = true;}
      if(fYQSWqePKP == WbOdEqZMaG){jQWieiKJuT = true;}
      else if(WbOdEqZMaG == fYQSWqePKP){gxynoDyXch = true;}
      if(oTQEpllYIG == qednzNOcyG){BxpMsfhGjS = true;}
      if(bRrdCpcTyP == eQjgbNhkFK){yrUjgWdAua = true;}
      if(yyQhsxfJJi == wWbdFElTiF){mKNYFgEXTx = true;}
      while(qednzNOcyG == oTQEpllYIG){NjzlAfIjGe = true;}
      while(eQjgbNhkFK == eQjgbNhkFK){pjMrZJobtN = true;}
      while(wWbdFElTiF == wWbdFElTiF){oihhiLZNoK = true;}
      if(jzLqnlPFdG == true){jzLqnlPFdG = false;}
      if(dEBRzGqhCq == true){dEBRzGqhCq = false;}
      if(InGQKTCzht == true){InGQKTCzht = false;}
      if(ausrWeFsro == true){ausrWeFsro = false;}
      if(xbGzfrVJdy == true){xbGzfrVJdy = false;}
      if(NDRrxurtfT == true){NDRrxurtfT = false;}
      if(jQWieiKJuT == true){jQWieiKJuT = false;}
      if(BxpMsfhGjS == true){BxpMsfhGjS = false;}
      if(yrUjgWdAua == true){yrUjgWdAua = false;}
      if(mKNYFgEXTx == true){mKNYFgEXTx = false;}
      if(gLOJwQNEzQ == true){gLOJwQNEzQ = false;}
      if(NkKNUqumCI == true){NkKNUqumCI = false;}
      if(FJOTgqjQGL == true){FJOTgqjQGL = false;}
      if(bKmlklcKmP == true){bKmlklcKmP = false;}
      if(IYBdNCAzDN == true){IYBdNCAzDN = false;}
      if(lRULrXxtcF == true){lRULrXxtcF = false;}
      if(gxynoDyXch == true){gxynoDyXch = false;}
      if(NjzlAfIjGe == true){NjzlAfIjGe = false;}
      if(pjMrZJobtN == true){pjMrZJobtN = false;}
      if(oihhiLZNoK == true){oihhiLZNoK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PZJTCEDWSM
{ 
  void kbStUVjLqb()
  { 
      bool yZeeFBiBca = false;
      bool aUBoEnNKIW = false;
      bool HjYhhwySKm = false;
      bool eWQsaMHhQO = false;
      bool MUVYStLanN = false;
      bool MVxNtOBwwJ = false;
      bool DPppuEDHOr = false;
      bool ffstJlNzPn = false;
      bool jWexZJYCyL = false;
      bool LKkGKxdfcO = false;
      bool pEMwggbDEH = false;
      bool ABgIZgksPl = false;
      bool fBJCfUtljM = false;
      bool IhWByfJhPs = false;
      bool gVypKJdcgc = false;
      bool LkPphnuKPM = false;
      bool moSqzSefJt = false;
      bool WDcWEKDSqV = false;
      bool qQFGAXRfSJ = false;
      bool kzRDFqCGqr = false;
      string dGcICadCWH;
      string ccgUuPszDq;
      string saiRVJalom;
      string bVgLyeUTCh;
      string BHllSJwskY;
      string sLnuwDWKls;
      string ArNieGuUVP;
      string uZxUoknxzL;
      string MgAPtPglOu;
      string IJgIhIsDGM;
      string NbWVdSOTFK;
      string OZLZgSrynO;
      string YJFaHksnlg;
      string qkpmyjJdkb;
      string BNfEfhyGbP;
      string PuOqmEVwNr;
      string ZagYSdiYbw;
      string JNMMNLcBuY;
      string mlMWQwWzAs;
      string ONPsBafGkB;
      if(dGcICadCWH == NbWVdSOTFK){yZeeFBiBca = true;}
      else if(NbWVdSOTFK == dGcICadCWH){pEMwggbDEH = true;}
      if(ccgUuPszDq == OZLZgSrynO){aUBoEnNKIW = true;}
      else if(OZLZgSrynO == ccgUuPszDq){ABgIZgksPl = true;}
      if(saiRVJalom == YJFaHksnlg){HjYhhwySKm = true;}
      else if(YJFaHksnlg == saiRVJalom){fBJCfUtljM = true;}
      if(bVgLyeUTCh == qkpmyjJdkb){eWQsaMHhQO = true;}
      else if(qkpmyjJdkb == bVgLyeUTCh){IhWByfJhPs = true;}
      if(BHllSJwskY == BNfEfhyGbP){MUVYStLanN = true;}
      else if(BNfEfhyGbP == BHllSJwskY){gVypKJdcgc = true;}
      if(sLnuwDWKls == PuOqmEVwNr){MVxNtOBwwJ = true;}
      else if(PuOqmEVwNr == sLnuwDWKls){LkPphnuKPM = true;}
      if(ArNieGuUVP == ZagYSdiYbw){DPppuEDHOr = true;}
      else if(ZagYSdiYbw == ArNieGuUVP){moSqzSefJt = true;}
      if(uZxUoknxzL == JNMMNLcBuY){ffstJlNzPn = true;}
      if(MgAPtPglOu == mlMWQwWzAs){jWexZJYCyL = true;}
      if(IJgIhIsDGM == ONPsBafGkB){LKkGKxdfcO = true;}
      while(JNMMNLcBuY == uZxUoknxzL){WDcWEKDSqV = true;}
      while(mlMWQwWzAs == mlMWQwWzAs){qQFGAXRfSJ = true;}
      while(ONPsBafGkB == ONPsBafGkB){kzRDFqCGqr = true;}
      if(yZeeFBiBca == true){yZeeFBiBca = false;}
      if(aUBoEnNKIW == true){aUBoEnNKIW = false;}
      if(HjYhhwySKm == true){HjYhhwySKm = false;}
      if(eWQsaMHhQO == true){eWQsaMHhQO = false;}
      if(MUVYStLanN == true){MUVYStLanN = false;}
      if(MVxNtOBwwJ == true){MVxNtOBwwJ = false;}
      if(DPppuEDHOr == true){DPppuEDHOr = false;}
      if(ffstJlNzPn == true){ffstJlNzPn = false;}
      if(jWexZJYCyL == true){jWexZJYCyL = false;}
      if(LKkGKxdfcO == true){LKkGKxdfcO = false;}
      if(pEMwggbDEH == true){pEMwggbDEH = false;}
      if(ABgIZgksPl == true){ABgIZgksPl = false;}
      if(fBJCfUtljM == true){fBJCfUtljM = false;}
      if(IhWByfJhPs == true){IhWByfJhPs = false;}
      if(gVypKJdcgc == true){gVypKJdcgc = false;}
      if(LkPphnuKPM == true){LkPphnuKPM = false;}
      if(moSqzSefJt == true){moSqzSefJt = false;}
      if(WDcWEKDSqV == true){WDcWEKDSqV = false;}
      if(qQFGAXRfSJ == true){qQFGAXRfSJ = false;}
      if(kzRDFqCGqr == true){kzRDFqCGqr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KSUMHJKKSZ
{ 
  void ZtFCjWPdGQ()
  { 
      bool ptULUdhuVB = false;
      bool GfGhPixWuZ = false;
      bool KdpYKDQoiQ = false;
      bool htdmBcsigT = false;
      bool AaMVAghYap = false;
      bool jrGptMEwAR = false;
      bool MsMFSLBSfl = false;
      bool pOpaYjRTYd = false;
      bool UqNsfpmnro = false;
      bool YwhOHGOHDn = false;
      bool eOsozVCfQK = false;
      bool ujXVxVfSYc = false;
      bool XNaqNtfbTS = false;
      bool UJQjzFyXsK = false;
      bool nHWgtnYMQl = false;
      bool auhaPQnCbF = false;
      bool paLypkTmWr = false;
      bool WHZNoQqsxY = false;
      bool USKhrYapTh = false;
      bool QwwnUNuXqI = false;
      string bHGcKSuFNE;
      string qWBljOlFzX;
      string GJBMhUUlty;
      string maoFMXniip;
      string QjzICcoDiu;
      string OrxAbRGttp;
      string xIKORQhwZM;
      string ZDWIHVKrJF;
      string fRDzitcodt;
      string QsossHYjpl;
      string usdVULVnsF;
      string HNknKfFKfW;
      string mrSeugOcPz;
      string xZBWJmmgeW;
      string YnuXlDIAiN;
      string BODTzDZyZn;
      string xrYsIgMNeX;
      string rXfIxywtrr;
      string XNtCyhVhXG;
      string MoHqZgPCPF;
      if(bHGcKSuFNE == usdVULVnsF){ptULUdhuVB = true;}
      else if(usdVULVnsF == bHGcKSuFNE){eOsozVCfQK = true;}
      if(qWBljOlFzX == HNknKfFKfW){GfGhPixWuZ = true;}
      else if(HNknKfFKfW == qWBljOlFzX){ujXVxVfSYc = true;}
      if(GJBMhUUlty == mrSeugOcPz){KdpYKDQoiQ = true;}
      else if(mrSeugOcPz == GJBMhUUlty){XNaqNtfbTS = true;}
      if(maoFMXniip == xZBWJmmgeW){htdmBcsigT = true;}
      else if(xZBWJmmgeW == maoFMXniip){UJQjzFyXsK = true;}
      if(QjzICcoDiu == YnuXlDIAiN){AaMVAghYap = true;}
      else if(YnuXlDIAiN == QjzICcoDiu){nHWgtnYMQl = true;}
      if(OrxAbRGttp == BODTzDZyZn){jrGptMEwAR = true;}
      else if(BODTzDZyZn == OrxAbRGttp){auhaPQnCbF = true;}
      if(xIKORQhwZM == xrYsIgMNeX){MsMFSLBSfl = true;}
      else if(xrYsIgMNeX == xIKORQhwZM){paLypkTmWr = true;}
      if(ZDWIHVKrJF == rXfIxywtrr){pOpaYjRTYd = true;}
      if(fRDzitcodt == XNtCyhVhXG){UqNsfpmnro = true;}
      if(QsossHYjpl == MoHqZgPCPF){YwhOHGOHDn = true;}
      while(rXfIxywtrr == ZDWIHVKrJF){WHZNoQqsxY = true;}
      while(XNtCyhVhXG == XNtCyhVhXG){USKhrYapTh = true;}
      while(MoHqZgPCPF == MoHqZgPCPF){QwwnUNuXqI = true;}
      if(ptULUdhuVB == true){ptULUdhuVB = false;}
      if(GfGhPixWuZ == true){GfGhPixWuZ = false;}
      if(KdpYKDQoiQ == true){KdpYKDQoiQ = false;}
      if(htdmBcsigT == true){htdmBcsigT = false;}
      if(AaMVAghYap == true){AaMVAghYap = false;}
      if(jrGptMEwAR == true){jrGptMEwAR = false;}
      if(MsMFSLBSfl == true){MsMFSLBSfl = false;}
      if(pOpaYjRTYd == true){pOpaYjRTYd = false;}
      if(UqNsfpmnro == true){UqNsfpmnro = false;}
      if(YwhOHGOHDn == true){YwhOHGOHDn = false;}
      if(eOsozVCfQK == true){eOsozVCfQK = false;}
      if(ujXVxVfSYc == true){ujXVxVfSYc = false;}
      if(XNaqNtfbTS == true){XNaqNtfbTS = false;}
      if(UJQjzFyXsK == true){UJQjzFyXsK = false;}
      if(nHWgtnYMQl == true){nHWgtnYMQl = false;}
      if(auhaPQnCbF == true){auhaPQnCbF = false;}
      if(paLypkTmWr == true){paLypkTmWr = false;}
      if(WHZNoQqsxY == true){WHZNoQqsxY = false;}
      if(USKhrYapTh == true){USKhrYapTh = false;}
      if(QwwnUNuXqI == true){QwwnUNuXqI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QOVKQQEWZD
{ 
  void YZVUECVqLu()
  { 
      bool WOHAGDSuSX = false;
      bool PAEWCjzkjE = false;
      bool sTFGEdSWYN = false;
      bool VgWQScRCRY = false;
      bool ZYEijDjYXj = false;
      bool VpuKdDJAwb = false;
      bool juVEBhcmDY = false;
      bool RxTLeTOVVB = false;
      bool ssynrfkMmP = false;
      bool TRGoodCzhB = false;
      bool ZwVzSYWjfJ = false;
      bool nmduipxhMV = false;
      bool xQAnFApdVA = false;
      bool QPnUawXyVT = false;
      bool IAfdbfmLGy = false;
      bool fUmMEJwhtR = false;
      bool hsOerYzJTl = false;
      bool RzMsMxjVLY = false;
      bool NeWWTwcaPR = false;
      bool zpEDkPlAkD = false;
      string ZfwqOxapII;
      string wxfhOgZEpZ;
      string aggUyjybmI;
      string RSrNAdtpkV;
      string okLzjmFzBC;
      string gfaruryasd;
      string yyCzqcFSgj;
      string mjSTkpVhjK;
      string ezZjbkrzSH;
      string eSwHXphMIG;
      string hmRRPxNGHF;
      string WQpizzXSXG;
      string kFzZEUmLwt;
      string HIYcbqgIUd;
      string XlrMkObNIX;
      string SPnBVbmrMt;
      string JCJMxNqMGH;
      string ROxsiyMmnp;
      string rZCXbrTXHz;
      string DnwaUUbfPW;
      if(ZfwqOxapII == hmRRPxNGHF){WOHAGDSuSX = true;}
      else if(hmRRPxNGHF == ZfwqOxapII){ZwVzSYWjfJ = true;}
      if(wxfhOgZEpZ == WQpizzXSXG){PAEWCjzkjE = true;}
      else if(WQpizzXSXG == wxfhOgZEpZ){nmduipxhMV = true;}
      if(aggUyjybmI == kFzZEUmLwt){sTFGEdSWYN = true;}
      else if(kFzZEUmLwt == aggUyjybmI){xQAnFApdVA = true;}
      if(RSrNAdtpkV == HIYcbqgIUd){VgWQScRCRY = true;}
      else if(HIYcbqgIUd == RSrNAdtpkV){QPnUawXyVT = true;}
      if(okLzjmFzBC == XlrMkObNIX){ZYEijDjYXj = true;}
      else if(XlrMkObNIX == okLzjmFzBC){IAfdbfmLGy = true;}
      if(gfaruryasd == SPnBVbmrMt){VpuKdDJAwb = true;}
      else if(SPnBVbmrMt == gfaruryasd){fUmMEJwhtR = true;}
      if(yyCzqcFSgj == JCJMxNqMGH){juVEBhcmDY = true;}
      else if(JCJMxNqMGH == yyCzqcFSgj){hsOerYzJTl = true;}
      if(mjSTkpVhjK == ROxsiyMmnp){RxTLeTOVVB = true;}
      if(ezZjbkrzSH == rZCXbrTXHz){ssynrfkMmP = true;}
      if(eSwHXphMIG == DnwaUUbfPW){TRGoodCzhB = true;}
      while(ROxsiyMmnp == mjSTkpVhjK){RzMsMxjVLY = true;}
      while(rZCXbrTXHz == rZCXbrTXHz){NeWWTwcaPR = true;}
      while(DnwaUUbfPW == DnwaUUbfPW){zpEDkPlAkD = true;}
      if(WOHAGDSuSX == true){WOHAGDSuSX = false;}
      if(PAEWCjzkjE == true){PAEWCjzkjE = false;}
      if(sTFGEdSWYN == true){sTFGEdSWYN = false;}
      if(VgWQScRCRY == true){VgWQScRCRY = false;}
      if(ZYEijDjYXj == true){ZYEijDjYXj = false;}
      if(VpuKdDJAwb == true){VpuKdDJAwb = false;}
      if(juVEBhcmDY == true){juVEBhcmDY = false;}
      if(RxTLeTOVVB == true){RxTLeTOVVB = false;}
      if(ssynrfkMmP == true){ssynrfkMmP = false;}
      if(TRGoodCzhB == true){TRGoodCzhB = false;}
      if(ZwVzSYWjfJ == true){ZwVzSYWjfJ = false;}
      if(nmduipxhMV == true){nmduipxhMV = false;}
      if(xQAnFApdVA == true){xQAnFApdVA = false;}
      if(QPnUawXyVT == true){QPnUawXyVT = false;}
      if(IAfdbfmLGy == true){IAfdbfmLGy = false;}
      if(fUmMEJwhtR == true){fUmMEJwhtR = false;}
      if(hsOerYzJTl == true){hsOerYzJTl = false;}
      if(RzMsMxjVLY == true){RzMsMxjVLY = false;}
      if(NeWWTwcaPR == true){NeWWTwcaPR = false;}
      if(zpEDkPlAkD == true){zpEDkPlAkD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YFEYOAVTSB
{ 
  void ZzjTqrYVnx()
  { 
      bool AiLVyGzzzi = false;
      bool BFSlRWDmms = false;
      bool bEPXhhXrFr = false;
      bool tfzRwGFypU = false;
      bool ZUfKrhZaxu = false;
      bool OwwCUwSmmP = false;
      bool hGsGoVFAoK = false;
      bool QFxRTGYFeA = false;
      bool IxtKdxixwA = false;
      bool nRKamSbYVW = false;
      bool bYkmdJydhO = false;
      bool XNHlGkMicq = false;
      bool mIDoWytWeA = false;
      bool eziIXiMHfT = false;
      bool yFOhcaJXTC = false;
      bool GswdjTUwPe = false;
      bool ObyhiXdzOW = false;
      bool YECIrgsVLu = false;
      bool nVCctlDEYE = false;
      bool OPqAFVGgyF = false;
      string HRbsAZgmHx;
      string VLFQjXmfiY;
      string yVnDVmFftj;
      string WdOfVohgQu;
      string cfZltdklOg;
      string ryceRlcYWN;
      string UBsWZqxYtu;
      string pGOnwSuCJr;
      string BSRNrYBiNj;
      string LIJDZblCLC;
      string fMrTpkioiz;
      string paPeLBjeeB;
      string pOKJihwLNT;
      string fspGLoayXG;
      string pLzOtkrerY;
      string rGXKMRBhjH;
      string nWbPSxjJSV;
      string alHXKdPuPx;
      string pRTnbYUfst;
      string mrNPRSjjwx;
      if(HRbsAZgmHx == fMrTpkioiz){AiLVyGzzzi = true;}
      else if(fMrTpkioiz == HRbsAZgmHx){bYkmdJydhO = true;}
      if(VLFQjXmfiY == paPeLBjeeB){BFSlRWDmms = true;}
      else if(paPeLBjeeB == VLFQjXmfiY){XNHlGkMicq = true;}
      if(yVnDVmFftj == pOKJihwLNT){bEPXhhXrFr = true;}
      else if(pOKJihwLNT == yVnDVmFftj){mIDoWytWeA = true;}
      if(WdOfVohgQu == fspGLoayXG){tfzRwGFypU = true;}
      else if(fspGLoayXG == WdOfVohgQu){eziIXiMHfT = true;}
      if(cfZltdklOg == pLzOtkrerY){ZUfKrhZaxu = true;}
      else if(pLzOtkrerY == cfZltdklOg){yFOhcaJXTC = true;}
      if(ryceRlcYWN == rGXKMRBhjH){OwwCUwSmmP = true;}
      else if(rGXKMRBhjH == ryceRlcYWN){GswdjTUwPe = true;}
      if(UBsWZqxYtu == nWbPSxjJSV){hGsGoVFAoK = true;}
      else if(nWbPSxjJSV == UBsWZqxYtu){ObyhiXdzOW = true;}
      if(pGOnwSuCJr == alHXKdPuPx){QFxRTGYFeA = true;}
      if(BSRNrYBiNj == pRTnbYUfst){IxtKdxixwA = true;}
      if(LIJDZblCLC == mrNPRSjjwx){nRKamSbYVW = true;}
      while(alHXKdPuPx == pGOnwSuCJr){YECIrgsVLu = true;}
      while(pRTnbYUfst == pRTnbYUfst){nVCctlDEYE = true;}
      while(mrNPRSjjwx == mrNPRSjjwx){OPqAFVGgyF = true;}
      if(AiLVyGzzzi == true){AiLVyGzzzi = false;}
      if(BFSlRWDmms == true){BFSlRWDmms = false;}
      if(bEPXhhXrFr == true){bEPXhhXrFr = false;}
      if(tfzRwGFypU == true){tfzRwGFypU = false;}
      if(ZUfKrhZaxu == true){ZUfKrhZaxu = false;}
      if(OwwCUwSmmP == true){OwwCUwSmmP = false;}
      if(hGsGoVFAoK == true){hGsGoVFAoK = false;}
      if(QFxRTGYFeA == true){QFxRTGYFeA = false;}
      if(IxtKdxixwA == true){IxtKdxixwA = false;}
      if(nRKamSbYVW == true){nRKamSbYVW = false;}
      if(bYkmdJydhO == true){bYkmdJydhO = false;}
      if(XNHlGkMicq == true){XNHlGkMicq = false;}
      if(mIDoWytWeA == true){mIDoWytWeA = false;}
      if(eziIXiMHfT == true){eziIXiMHfT = false;}
      if(yFOhcaJXTC == true){yFOhcaJXTC = false;}
      if(GswdjTUwPe == true){GswdjTUwPe = false;}
      if(ObyhiXdzOW == true){ObyhiXdzOW = false;}
      if(YECIrgsVLu == true){YECIrgsVLu = false;}
      if(nVCctlDEYE == true){nVCctlDEYE = false;}
      if(OPqAFVGgyF == true){OPqAFVGgyF = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TFSFXHBHHH
{ 
  void VELKahjZYL()
  { 
      bool CkNdABPoeZ = false;
      bool XfNXRMnWuq = false;
      bool VDReHTaVJx = false;
      bool refyUhjKAd = false;
      bool wRiGOhEkYU = false;
      bool yLtAByShVf = false;
      bool ZteggrzPCi = false;
      bool bdWQtDTLPY = false;
      bool PpxoyfkfxV = false;
      bool PBnTEBlBYV = false;
      bool iohorUodGx = false;
      bool eHRwQixpiA = false;
      bool HTRgTZoDMr = false;
      bool QRlspZIHwd = false;
      bool qHYzqHjqCU = false;
      bool EwSYOPWBVR = false;
      bool fkQNgMVXGq = false;
      bool apsEJztSoL = false;
      bool pxAqnUzNea = false;
      bool CGkeyFpLSe = false;
      string wfssusOijQ;
      string qnpPHhktnx;
      string XesxlSwicU;
      string grrieBMhQY;
      string FUdAgadorL;
      string oxajwQmwHc;
      string weFOjyuGjF;
      string UCdPqDBrPK;
      string oHpRejRImO;
      string OQanbZOOPh;
      string eWFlpmSKrr;
      string OtfwnonmiA;
      string glSUqEbomQ;
      string nlcgeRCEbr;
      string QGQuzTjdAm;
      string kbfOAsDjuk;
      string KdXaDbRyQV;
      string ljLqjaVXVW;
      string TxoAeMGNfl;
      string NSIpgmipqW;
      if(wfssusOijQ == eWFlpmSKrr){CkNdABPoeZ = true;}
      else if(eWFlpmSKrr == wfssusOijQ){iohorUodGx = true;}
      if(qnpPHhktnx == OtfwnonmiA){XfNXRMnWuq = true;}
      else if(OtfwnonmiA == qnpPHhktnx){eHRwQixpiA = true;}
      if(XesxlSwicU == glSUqEbomQ){VDReHTaVJx = true;}
      else if(glSUqEbomQ == XesxlSwicU){HTRgTZoDMr = true;}
      if(grrieBMhQY == nlcgeRCEbr){refyUhjKAd = true;}
      else if(nlcgeRCEbr == grrieBMhQY){QRlspZIHwd = true;}
      if(FUdAgadorL == QGQuzTjdAm){wRiGOhEkYU = true;}
      else if(QGQuzTjdAm == FUdAgadorL){qHYzqHjqCU = true;}
      if(oxajwQmwHc == kbfOAsDjuk){yLtAByShVf = true;}
      else if(kbfOAsDjuk == oxajwQmwHc){EwSYOPWBVR = true;}
      if(weFOjyuGjF == KdXaDbRyQV){ZteggrzPCi = true;}
      else if(KdXaDbRyQV == weFOjyuGjF){fkQNgMVXGq = true;}
      if(UCdPqDBrPK == ljLqjaVXVW){bdWQtDTLPY = true;}
      if(oHpRejRImO == TxoAeMGNfl){PpxoyfkfxV = true;}
      if(OQanbZOOPh == NSIpgmipqW){PBnTEBlBYV = true;}
      while(ljLqjaVXVW == UCdPqDBrPK){apsEJztSoL = true;}
      while(TxoAeMGNfl == TxoAeMGNfl){pxAqnUzNea = true;}
      while(NSIpgmipqW == NSIpgmipqW){CGkeyFpLSe = true;}
      if(CkNdABPoeZ == true){CkNdABPoeZ = false;}
      if(XfNXRMnWuq == true){XfNXRMnWuq = false;}
      if(VDReHTaVJx == true){VDReHTaVJx = false;}
      if(refyUhjKAd == true){refyUhjKAd = false;}
      if(wRiGOhEkYU == true){wRiGOhEkYU = false;}
      if(yLtAByShVf == true){yLtAByShVf = false;}
      if(ZteggrzPCi == true){ZteggrzPCi = false;}
      if(bdWQtDTLPY == true){bdWQtDTLPY = false;}
      if(PpxoyfkfxV == true){PpxoyfkfxV = false;}
      if(PBnTEBlBYV == true){PBnTEBlBYV = false;}
      if(iohorUodGx == true){iohorUodGx = false;}
      if(eHRwQixpiA == true){eHRwQixpiA = false;}
      if(HTRgTZoDMr == true){HTRgTZoDMr = false;}
      if(QRlspZIHwd == true){QRlspZIHwd = false;}
      if(qHYzqHjqCU == true){qHYzqHjqCU = false;}
      if(EwSYOPWBVR == true){EwSYOPWBVR = false;}
      if(fkQNgMVXGq == true){fkQNgMVXGq = false;}
      if(apsEJztSoL == true){apsEJztSoL = false;}
      if(pxAqnUzNea == true){pxAqnUzNea = false;}
      if(CGkeyFpLSe == true){CGkeyFpLSe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DQNFQWXFYM
{ 
  void fqrRZfudKt()
  { 
      bool AJVOdCxmtj = false;
      bool cRdgWUouTZ = false;
      bool nmoVfrIsDb = false;
      bool rcYIZdnHzY = false;
      bool KMLfRQsLhB = false;
      bool chRYXMUAil = false;
      bool sDiRqArUGs = false;
      bool ghutObaIDO = false;
      bool yxKByKKAjM = false;
      bool JmYNfHcsnQ = false;
      bool YYEnUtlXKw = false;
      bool lLwTnurgTg = false;
      bool pqjsjsJhOH = false;
      bool ygUlpYrWgE = false;
      bool XmpnaxqEYj = false;
      bool OwZGahnLEb = false;
      bool kEICxEpJdO = false;
      bool DVAPJCBwSl = false;
      bool CEIerCHNKI = false;
      bool kgSESTdmpV = false;
      string GdHDjPaTEh;
      string PVlcawPiOO;
      string OBjnVfkAcx;
      string SwRQFaGZTL;
      string eAycssOraB;
      string SbzKzEtThD;
      string eapHPSDVWA;
      string ooPDjNDHtc;
      string QrZbepxHRB;
      string fmrPfEKpMZ;
      string osUaLcLuZk;
      string AmYEllAOcY;
      string VwEVTBxzWe;
      string DMtTnXWzPC;
      string eBgfAsOmcs;
      string DuNnErozGK;
      string eLYugmsEQO;
      string PByatUOXVZ;
      string mKVdaHzFPu;
      string hXrFWSclyM;
      if(GdHDjPaTEh == osUaLcLuZk){AJVOdCxmtj = true;}
      else if(osUaLcLuZk == GdHDjPaTEh){YYEnUtlXKw = true;}
      if(PVlcawPiOO == AmYEllAOcY){cRdgWUouTZ = true;}
      else if(AmYEllAOcY == PVlcawPiOO){lLwTnurgTg = true;}
      if(OBjnVfkAcx == VwEVTBxzWe){nmoVfrIsDb = true;}
      else if(VwEVTBxzWe == OBjnVfkAcx){pqjsjsJhOH = true;}
      if(SwRQFaGZTL == DMtTnXWzPC){rcYIZdnHzY = true;}
      else if(DMtTnXWzPC == SwRQFaGZTL){ygUlpYrWgE = true;}
      if(eAycssOraB == eBgfAsOmcs){KMLfRQsLhB = true;}
      else if(eBgfAsOmcs == eAycssOraB){XmpnaxqEYj = true;}
      if(SbzKzEtThD == DuNnErozGK){chRYXMUAil = true;}
      else if(DuNnErozGK == SbzKzEtThD){OwZGahnLEb = true;}
      if(eapHPSDVWA == eLYugmsEQO){sDiRqArUGs = true;}
      else if(eLYugmsEQO == eapHPSDVWA){kEICxEpJdO = true;}
      if(ooPDjNDHtc == PByatUOXVZ){ghutObaIDO = true;}
      if(QrZbepxHRB == mKVdaHzFPu){yxKByKKAjM = true;}
      if(fmrPfEKpMZ == hXrFWSclyM){JmYNfHcsnQ = true;}
      while(PByatUOXVZ == ooPDjNDHtc){DVAPJCBwSl = true;}
      while(mKVdaHzFPu == mKVdaHzFPu){CEIerCHNKI = true;}
      while(hXrFWSclyM == hXrFWSclyM){kgSESTdmpV = true;}
      if(AJVOdCxmtj == true){AJVOdCxmtj = false;}
      if(cRdgWUouTZ == true){cRdgWUouTZ = false;}
      if(nmoVfrIsDb == true){nmoVfrIsDb = false;}
      if(rcYIZdnHzY == true){rcYIZdnHzY = false;}
      if(KMLfRQsLhB == true){KMLfRQsLhB = false;}
      if(chRYXMUAil == true){chRYXMUAil = false;}
      if(sDiRqArUGs == true){sDiRqArUGs = false;}
      if(ghutObaIDO == true){ghutObaIDO = false;}
      if(yxKByKKAjM == true){yxKByKKAjM = false;}
      if(JmYNfHcsnQ == true){JmYNfHcsnQ = false;}
      if(YYEnUtlXKw == true){YYEnUtlXKw = false;}
      if(lLwTnurgTg == true){lLwTnurgTg = false;}
      if(pqjsjsJhOH == true){pqjsjsJhOH = false;}
      if(ygUlpYrWgE == true){ygUlpYrWgE = false;}
      if(XmpnaxqEYj == true){XmpnaxqEYj = false;}
      if(OwZGahnLEb == true){OwZGahnLEb = false;}
      if(kEICxEpJdO == true){kEICxEpJdO = false;}
      if(DVAPJCBwSl == true){DVAPJCBwSl = false;}
      if(CEIerCHNKI == true){CEIerCHNKI = false;}
      if(kgSESTdmpV == true){kgSESTdmpV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VNWABKXTSY
{ 
  void CKIleMUztE()
  { 
      bool IzhAAjHRlC = false;
      bool TdKRpJydhR = false;
      bool qklJSfSmVp = false;
      bool ZwhbGmYBij = false;
      bool nuJMUDMlGQ = false;
      bool NPXIhXMYgU = false;
      bool RTuwcXwhLD = false;
      bool jxuxcuaZoP = false;
      bool TffFcJXYti = false;
      bool aATNSTANiw = false;
      bool GdZDfQUZfl = false;
      bool wmiRAkmpTS = false;
      bool zXnbbicgdJ = false;
      bool uMpsJoFcZL = false;
      bool nysIlmRCNh = false;
      bool gKtQxhpyaw = false;
      bool mnuVpixpIL = false;
      bool eKPHeBeeoo = false;
      bool RaTwbkSZua = false;
      bool zHBcXACWRD = false;
      string gicOwXAhWa;
      string TtRRdbSUfd;
      string AUaKPXLVej;
      string tkzgYQDnuC;
      string qHokdZTjzn;
      string kcdPsMIRyY;
      string MdDmfhtjrN;
      string XeZcmreogp;
      string DaWqyQHjnM;
      string IkoVpMECEw;
      string firlPGDAbA;
      string hSkIUlGMnP;
      string ZJwfEloDmZ;
      string mbkjKUikaq;
      string MGHmepzgBD;
      string FrOoVVmLgc;
      string RAMCMFgAyA;
      string DVkwCSBIBO;
      string lHCiTuNwOn;
      string hWDxZMsFaY;
      if(gicOwXAhWa == firlPGDAbA){IzhAAjHRlC = true;}
      else if(firlPGDAbA == gicOwXAhWa){GdZDfQUZfl = true;}
      if(TtRRdbSUfd == hSkIUlGMnP){TdKRpJydhR = true;}
      else if(hSkIUlGMnP == TtRRdbSUfd){wmiRAkmpTS = true;}
      if(AUaKPXLVej == ZJwfEloDmZ){qklJSfSmVp = true;}
      else if(ZJwfEloDmZ == AUaKPXLVej){zXnbbicgdJ = true;}
      if(tkzgYQDnuC == mbkjKUikaq){ZwhbGmYBij = true;}
      else if(mbkjKUikaq == tkzgYQDnuC){uMpsJoFcZL = true;}
      if(qHokdZTjzn == MGHmepzgBD){nuJMUDMlGQ = true;}
      else if(MGHmepzgBD == qHokdZTjzn){nysIlmRCNh = true;}
      if(kcdPsMIRyY == FrOoVVmLgc){NPXIhXMYgU = true;}
      else if(FrOoVVmLgc == kcdPsMIRyY){gKtQxhpyaw = true;}
      if(MdDmfhtjrN == RAMCMFgAyA){RTuwcXwhLD = true;}
      else if(RAMCMFgAyA == MdDmfhtjrN){mnuVpixpIL = true;}
      if(XeZcmreogp == DVkwCSBIBO){jxuxcuaZoP = true;}
      if(DaWqyQHjnM == lHCiTuNwOn){TffFcJXYti = true;}
      if(IkoVpMECEw == hWDxZMsFaY){aATNSTANiw = true;}
      while(DVkwCSBIBO == XeZcmreogp){eKPHeBeeoo = true;}
      while(lHCiTuNwOn == lHCiTuNwOn){RaTwbkSZua = true;}
      while(hWDxZMsFaY == hWDxZMsFaY){zHBcXACWRD = true;}
      if(IzhAAjHRlC == true){IzhAAjHRlC = false;}
      if(TdKRpJydhR == true){TdKRpJydhR = false;}
      if(qklJSfSmVp == true){qklJSfSmVp = false;}
      if(ZwhbGmYBij == true){ZwhbGmYBij = false;}
      if(nuJMUDMlGQ == true){nuJMUDMlGQ = false;}
      if(NPXIhXMYgU == true){NPXIhXMYgU = false;}
      if(RTuwcXwhLD == true){RTuwcXwhLD = false;}
      if(jxuxcuaZoP == true){jxuxcuaZoP = false;}
      if(TffFcJXYti == true){TffFcJXYti = false;}
      if(aATNSTANiw == true){aATNSTANiw = false;}
      if(GdZDfQUZfl == true){GdZDfQUZfl = false;}
      if(wmiRAkmpTS == true){wmiRAkmpTS = false;}
      if(zXnbbicgdJ == true){zXnbbicgdJ = false;}
      if(uMpsJoFcZL == true){uMpsJoFcZL = false;}
      if(nysIlmRCNh == true){nysIlmRCNh = false;}
      if(gKtQxhpyaw == true){gKtQxhpyaw = false;}
      if(mnuVpixpIL == true){mnuVpixpIL = false;}
      if(eKPHeBeeoo == true){eKPHeBeeoo = false;}
      if(RaTwbkSZua == true){RaTwbkSZua = false;}
      if(zHBcXACWRD == true){zHBcXACWRD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GNAIDMTMJL
{ 
  void JCpGTgdTcj()
  { 
      bool MSXDTiPoyY = false;
      bool mKOdWIKVdc = false;
      bool hClJBhgNCI = false;
      bool qkKkPQPoAI = false;
      bool jkeZfVnBmj = false;
      bool HnhiBJgOCl = false;
      bool BCmpgBAcNF = false;
      bool sVUUrxWLce = false;
      bool tfZSACDajF = false;
      bool jAlOGhxnsd = false;
      bool VKngpkTMpE = false;
      bool lUlWqVbTpd = false;
      bool yYsIUrjGOJ = false;
      bool ziSyJjrsbP = false;
      bool ipkeSPOOXj = false;
      bool GdyOexQfMx = false;
      bool BMyfFDBmNH = false;
      bool dhqZuImOoF = false;
      bool dUhdCVkESM = false;
      bool SLfdNVrEZP = false;
      string YGQBRVuItz;
      string GDxsigCtpn;
      string iTxCCPLkKh;
      string ykifRlcgim;
      string FXjWziJZYy;
      string GtSraEJOzB;
      string JKyMBUUQax;
      string DVLCiLjZDS;
      string GlLLaERpFN;
      string bOggctYXVd;
      string OryaukhQJP;
      string UjKguXymlw;
      string DdLNUnndZn;
      string lMDZDNmkPo;
      string VBgorrEAlE;
      string wllZajgjFG;
      string wDpUcIfkch;
      string kMWwaBxVVb;
      string jmrUqAadyT;
      string fCeJsSKLLj;
      if(YGQBRVuItz == OryaukhQJP){MSXDTiPoyY = true;}
      else if(OryaukhQJP == YGQBRVuItz){VKngpkTMpE = true;}
      if(GDxsigCtpn == UjKguXymlw){mKOdWIKVdc = true;}
      else if(UjKguXymlw == GDxsigCtpn){lUlWqVbTpd = true;}
      if(iTxCCPLkKh == DdLNUnndZn){hClJBhgNCI = true;}
      else if(DdLNUnndZn == iTxCCPLkKh){yYsIUrjGOJ = true;}
      if(ykifRlcgim == lMDZDNmkPo){qkKkPQPoAI = true;}
      else if(lMDZDNmkPo == ykifRlcgim){ziSyJjrsbP = true;}
      if(FXjWziJZYy == VBgorrEAlE){jkeZfVnBmj = true;}
      else if(VBgorrEAlE == FXjWziJZYy){ipkeSPOOXj = true;}
      if(GtSraEJOzB == wllZajgjFG){HnhiBJgOCl = true;}
      else if(wllZajgjFG == GtSraEJOzB){GdyOexQfMx = true;}
      if(JKyMBUUQax == wDpUcIfkch){BCmpgBAcNF = true;}
      else if(wDpUcIfkch == JKyMBUUQax){BMyfFDBmNH = true;}
      if(DVLCiLjZDS == kMWwaBxVVb){sVUUrxWLce = true;}
      if(GlLLaERpFN == jmrUqAadyT){tfZSACDajF = true;}
      if(bOggctYXVd == fCeJsSKLLj){jAlOGhxnsd = true;}
      while(kMWwaBxVVb == DVLCiLjZDS){dhqZuImOoF = true;}
      while(jmrUqAadyT == jmrUqAadyT){dUhdCVkESM = true;}
      while(fCeJsSKLLj == fCeJsSKLLj){SLfdNVrEZP = true;}
      if(MSXDTiPoyY == true){MSXDTiPoyY = false;}
      if(mKOdWIKVdc == true){mKOdWIKVdc = false;}
      if(hClJBhgNCI == true){hClJBhgNCI = false;}
      if(qkKkPQPoAI == true){qkKkPQPoAI = false;}
      if(jkeZfVnBmj == true){jkeZfVnBmj = false;}
      if(HnhiBJgOCl == true){HnhiBJgOCl = false;}
      if(BCmpgBAcNF == true){BCmpgBAcNF = false;}
      if(sVUUrxWLce == true){sVUUrxWLce = false;}
      if(tfZSACDajF == true){tfZSACDajF = false;}
      if(jAlOGhxnsd == true){jAlOGhxnsd = false;}
      if(VKngpkTMpE == true){VKngpkTMpE = false;}
      if(lUlWqVbTpd == true){lUlWqVbTpd = false;}
      if(yYsIUrjGOJ == true){yYsIUrjGOJ = false;}
      if(ziSyJjrsbP == true){ziSyJjrsbP = false;}
      if(ipkeSPOOXj == true){ipkeSPOOXj = false;}
      if(GdyOexQfMx == true){GdyOexQfMx = false;}
      if(BMyfFDBmNH == true){BMyfFDBmNH = false;}
      if(dhqZuImOoF == true){dhqZuImOoF = false;}
      if(dUhdCVkESM == true){dUhdCVkESM = false;}
      if(SLfdNVrEZP == true){SLfdNVrEZP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LVMRNNGVHH
{ 
  void GtWTeUAHgL()
  { 
      bool OHLdbcKlAQ = false;
      bool PzzumjtVtQ = false;
      bool XmgfNZDpPZ = false;
      bool qOtTbOuaaQ = false;
      bool LsTICLKkfA = false;
      bool LxZhpbaeQC = false;
      bool ZUWZYBEeTY = false;
      bool nTtcFeTXdB = false;
      bool ORKPOtnMcA = false;
      bool mQKYqqWsiA = false;
      bool HfRGLrDxcL = false;
      bool HCBPmHJGLe = false;
      bool NrBTlXIUlO = false;
      bool HaocsiADEV = false;
      bool pqiRrjPuli = false;
      bool gPgDmPdyUn = false;
      bool ihKODgNXVD = false;
      bool HGConWIjaB = false;
      bool INhDgrtkpd = false;
      bool FBhQmIfUNK = false;
      string QTcUsLpbyN;
      string JbjozFsVwW;
      string PKqsupGPRl;
      string LZeqwGCAhs;
      string uRbWOmPdsu;
      string BwrYbFcjYC;
      string FaJMCjxssr;
      string CrmDbyhUdH;
      string ZcVWjPgFOp;
      string dSaenTHPTd;
      string pWkBsIIowc;
      string WHFQbbFmbm;
      string CqdCNiRIPI;
      string qSAARDrUcb;
      string GrIkSWNKCL;
      string NUmgayrhmJ;
      string qAYnlEHFzY;
      string jkJAfKisiq;
      string VGFDtVfTYS;
      string LJgTjGSTyO;
      if(QTcUsLpbyN == pWkBsIIowc){OHLdbcKlAQ = true;}
      else if(pWkBsIIowc == QTcUsLpbyN){HfRGLrDxcL = true;}
      if(JbjozFsVwW == WHFQbbFmbm){PzzumjtVtQ = true;}
      else if(WHFQbbFmbm == JbjozFsVwW){HCBPmHJGLe = true;}
      if(PKqsupGPRl == CqdCNiRIPI){XmgfNZDpPZ = true;}
      else if(CqdCNiRIPI == PKqsupGPRl){NrBTlXIUlO = true;}
      if(LZeqwGCAhs == qSAARDrUcb){qOtTbOuaaQ = true;}
      else if(qSAARDrUcb == LZeqwGCAhs){HaocsiADEV = true;}
      if(uRbWOmPdsu == GrIkSWNKCL){LsTICLKkfA = true;}
      else if(GrIkSWNKCL == uRbWOmPdsu){pqiRrjPuli = true;}
      if(BwrYbFcjYC == NUmgayrhmJ){LxZhpbaeQC = true;}
      else if(NUmgayrhmJ == BwrYbFcjYC){gPgDmPdyUn = true;}
      if(FaJMCjxssr == qAYnlEHFzY){ZUWZYBEeTY = true;}
      else if(qAYnlEHFzY == FaJMCjxssr){ihKODgNXVD = true;}
      if(CrmDbyhUdH == jkJAfKisiq){nTtcFeTXdB = true;}
      if(ZcVWjPgFOp == VGFDtVfTYS){ORKPOtnMcA = true;}
      if(dSaenTHPTd == LJgTjGSTyO){mQKYqqWsiA = true;}
      while(jkJAfKisiq == CrmDbyhUdH){HGConWIjaB = true;}
      while(VGFDtVfTYS == VGFDtVfTYS){INhDgrtkpd = true;}
      while(LJgTjGSTyO == LJgTjGSTyO){FBhQmIfUNK = true;}
      if(OHLdbcKlAQ == true){OHLdbcKlAQ = false;}
      if(PzzumjtVtQ == true){PzzumjtVtQ = false;}
      if(XmgfNZDpPZ == true){XmgfNZDpPZ = false;}
      if(qOtTbOuaaQ == true){qOtTbOuaaQ = false;}
      if(LsTICLKkfA == true){LsTICLKkfA = false;}
      if(LxZhpbaeQC == true){LxZhpbaeQC = false;}
      if(ZUWZYBEeTY == true){ZUWZYBEeTY = false;}
      if(nTtcFeTXdB == true){nTtcFeTXdB = false;}
      if(ORKPOtnMcA == true){ORKPOtnMcA = false;}
      if(mQKYqqWsiA == true){mQKYqqWsiA = false;}
      if(HfRGLrDxcL == true){HfRGLrDxcL = false;}
      if(HCBPmHJGLe == true){HCBPmHJGLe = false;}
      if(NrBTlXIUlO == true){NrBTlXIUlO = false;}
      if(HaocsiADEV == true){HaocsiADEV = false;}
      if(pqiRrjPuli == true){pqiRrjPuli = false;}
      if(gPgDmPdyUn == true){gPgDmPdyUn = false;}
      if(ihKODgNXVD == true){ihKODgNXVD = false;}
      if(HGConWIjaB == true){HGConWIjaB = false;}
      if(INhDgrtkpd == true){INhDgrtkpd = false;}
      if(FBhQmIfUNK == true){FBhQmIfUNK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KZCNWROXFH
{ 
  void WlBCEIWmPi()
  { 
      bool tTybcqksAp = false;
      bool lSWGgKgInY = false;
      bool ECGTtoXbcy = false;
      bool BmfHnrWMks = false;
      bool gUPKJIhTyR = false;
      bool UcOCcrMegm = false;
      bool TIPBnymORo = false;
      bool nTXtrjmuJk = false;
      bool HDdLdYYdew = false;
      bool eqfxYXENWL = false;
      bool VSBIcAFTJs = false;
      bool ERVufQeEXP = false;
      bool ZJiVJxnZUT = false;
      bool eYYyPnBUhc = false;
      bool SETlqNVjRI = false;
      bool sApGkMifzf = false;
      bool MezpNerWfq = false;
      bool OJWtwVoxew = false;
      bool aDILZeiRMD = false;
      bool FUVBQnEdXL = false;
      string kriQTOAYOy;
      string EzsESIyBeA;
      string mdRqHtlina;
      string uqGAkXFLtL;
      string VGCblnsTgi;
      string BPaegGZqIY;
      string cQjnDMssSt;
      string ClHTefbpET;
      string BRCFYeQaSd;
      string yODUhEWOYZ;
      string tzAlgPHHzA;
      string ejloXLOePX;
      string wRZnwnilka;
      string JMQeYBQPXI;
      string WIbSzNbNxW;
      string pqzAdcZMos;
      string fVWYqRSHPV;
      string tyGGAmpHoi;
      string LUnNkRNQBw;
      string bEaecBlCag;
      if(kriQTOAYOy == tzAlgPHHzA){tTybcqksAp = true;}
      else if(tzAlgPHHzA == kriQTOAYOy){VSBIcAFTJs = true;}
      if(EzsESIyBeA == ejloXLOePX){lSWGgKgInY = true;}
      else if(ejloXLOePX == EzsESIyBeA){ERVufQeEXP = true;}
      if(mdRqHtlina == wRZnwnilka){ECGTtoXbcy = true;}
      else if(wRZnwnilka == mdRqHtlina){ZJiVJxnZUT = true;}
      if(uqGAkXFLtL == JMQeYBQPXI){BmfHnrWMks = true;}
      else if(JMQeYBQPXI == uqGAkXFLtL){eYYyPnBUhc = true;}
      if(VGCblnsTgi == WIbSzNbNxW){gUPKJIhTyR = true;}
      else if(WIbSzNbNxW == VGCblnsTgi){SETlqNVjRI = true;}
      if(BPaegGZqIY == pqzAdcZMos){UcOCcrMegm = true;}
      else if(pqzAdcZMos == BPaegGZqIY){sApGkMifzf = true;}
      if(cQjnDMssSt == fVWYqRSHPV){TIPBnymORo = true;}
      else if(fVWYqRSHPV == cQjnDMssSt){MezpNerWfq = true;}
      if(ClHTefbpET == tyGGAmpHoi){nTXtrjmuJk = true;}
      if(BRCFYeQaSd == LUnNkRNQBw){HDdLdYYdew = true;}
      if(yODUhEWOYZ == bEaecBlCag){eqfxYXENWL = true;}
      while(tyGGAmpHoi == ClHTefbpET){OJWtwVoxew = true;}
      while(LUnNkRNQBw == LUnNkRNQBw){aDILZeiRMD = true;}
      while(bEaecBlCag == bEaecBlCag){FUVBQnEdXL = true;}
      if(tTybcqksAp == true){tTybcqksAp = false;}
      if(lSWGgKgInY == true){lSWGgKgInY = false;}
      if(ECGTtoXbcy == true){ECGTtoXbcy = false;}
      if(BmfHnrWMks == true){BmfHnrWMks = false;}
      if(gUPKJIhTyR == true){gUPKJIhTyR = false;}
      if(UcOCcrMegm == true){UcOCcrMegm = false;}
      if(TIPBnymORo == true){TIPBnymORo = false;}
      if(nTXtrjmuJk == true){nTXtrjmuJk = false;}
      if(HDdLdYYdew == true){HDdLdYYdew = false;}
      if(eqfxYXENWL == true){eqfxYXENWL = false;}
      if(VSBIcAFTJs == true){VSBIcAFTJs = false;}
      if(ERVufQeEXP == true){ERVufQeEXP = false;}
      if(ZJiVJxnZUT == true){ZJiVJxnZUT = false;}
      if(eYYyPnBUhc == true){eYYyPnBUhc = false;}
      if(SETlqNVjRI == true){SETlqNVjRI = false;}
      if(sApGkMifzf == true){sApGkMifzf = false;}
      if(MezpNerWfq == true){MezpNerWfq = false;}
      if(OJWtwVoxew == true){OJWtwVoxew = false;}
      if(aDILZeiRMD == true){aDILZeiRMD = false;}
      if(FUVBQnEdXL == true){FUVBQnEdXL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ITYPFZMCGQ
{ 
  void tKKmYlZmlY()
  { 
      bool aFQZkIsuMC = false;
      bool XsLDqeJigy = false;
      bool hydTMPXfsP = false;
      bool fVIOREWsuz = false;
      bool YktqAtkWEz = false;
      bool oLbKTbbxNV = false;
      bool kMBwWBkxll = false;
      bool HQngYxDlSR = false;
      bool dAbtZTsxup = false;
      bool cZfYOLixUz = false;
      bool JnkRqYWtlW = false;
      bool XoduqjkGWC = false;
      bool HmBwJgkOSM = false;
      bool NGRTHoPJDB = false;
      bool wyLYRnwjgd = false;
      bool acaSqWmnHJ = false;
      bool nHbYJRXJnC = false;
      bool TaDVFJbbdU = false;
      bool kBhnljfzck = false;
      bool CaBsNnWmPA = false;
      string DoQxZVSxiS;
      string HhiizJeoZj;
      string mECmVZzEkC;
      string gQBNaGAEYI;
      string qFSCDNXQMy;
      string pkLxjymdbA;
      string VNLflpgVIr;
      string JZQQsgkzjH;
      string BirlMuFDdb;
      string TWcXlctlQN;
      string gCIUoPUtED;
      string MmsTymVYTp;
      string kdrfYmxreP;
      string myhVayZWxw;
      string aKyZPfcJZy;
      string HUSacCKbwB;
      string SbPoQNFWrA;
      string NybXOZeRQr;
      string KnSfIRfGsX;
      string mxaudztJJB;
      if(DoQxZVSxiS == gCIUoPUtED){aFQZkIsuMC = true;}
      else if(gCIUoPUtED == DoQxZVSxiS){JnkRqYWtlW = true;}
      if(HhiizJeoZj == MmsTymVYTp){XsLDqeJigy = true;}
      else if(MmsTymVYTp == HhiizJeoZj){XoduqjkGWC = true;}
      if(mECmVZzEkC == kdrfYmxreP){hydTMPXfsP = true;}
      else if(kdrfYmxreP == mECmVZzEkC){HmBwJgkOSM = true;}
      if(gQBNaGAEYI == myhVayZWxw){fVIOREWsuz = true;}
      else if(myhVayZWxw == gQBNaGAEYI){NGRTHoPJDB = true;}
      if(qFSCDNXQMy == aKyZPfcJZy){YktqAtkWEz = true;}
      else if(aKyZPfcJZy == qFSCDNXQMy){wyLYRnwjgd = true;}
      if(pkLxjymdbA == HUSacCKbwB){oLbKTbbxNV = true;}
      else if(HUSacCKbwB == pkLxjymdbA){acaSqWmnHJ = true;}
      if(VNLflpgVIr == SbPoQNFWrA){kMBwWBkxll = true;}
      else if(SbPoQNFWrA == VNLflpgVIr){nHbYJRXJnC = true;}
      if(JZQQsgkzjH == NybXOZeRQr){HQngYxDlSR = true;}
      if(BirlMuFDdb == KnSfIRfGsX){dAbtZTsxup = true;}
      if(TWcXlctlQN == mxaudztJJB){cZfYOLixUz = true;}
      while(NybXOZeRQr == JZQQsgkzjH){TaDVFJbbdU = true;}
      while(KnSfIRfGsX == KnSfIRfGsX){kBhnljfzck = true;}
      while(mxaudztJJB == mxaudztJJB){CaBsNnWmPA = true;}
      if(aFQZkIsuMC == true){aFQZkIsuMC = false;}
      if(XsLDqeJigy == true){XsLDqeJigy = false;}
      if(hydTMPXfsP == true){hydTMPXfsP = false;}
      if(fVIOREWsuz == true){fVIOREWsuz = false;}
      if(YktqAtkWEz == true){YktqAtkWEz = false;}
      if(oLbKTbbxNV == true){oLbKTbbxNV = false;}
      if(kMBwWBkxll == true){kMBwWBkxll = false;}
      if(HQngYxDlSR == true){HQngYxDlSR = false;}
      if(dAbtZTsxup == true){dAbtZTsxup = false;}
      if(cZfYOLixUz == true){cZfYOLixUz = false;}
      if(JnkRqYWtlW == true){JnkRqYWtlW = false;}
      if(XoduqjkGWC == true){XoduqjkGWC = false;}
      if(HmBwJgkOSM == true){HmBwJgkOSM = false;}
      if(NGRTHoPJDB == true){NGRTHoPJDB = false;}
      if(wyLYRnwjgd == true){wyLYRnwjgd = false;}
      if(acaSqWmnHJ == true){acaSqWmnHJ = false;}
      if(nHbYJRXJnC == true){nHbYJRXJnC = false;}
      if(TaDVFJbbdU == true){TaDVFJbbdU = false;}
      if(kBhnljfzck == true){kBhnljfzck = false;}
      if(CaBsNnWmPA == true){CaBsNnWmPA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HLBGSPEDAD
{ 
  void PjwzerBZee()
  { 
      bool LmmqOnUwkr = false;
      bool yVyUSLwIok = false;
      bool oHlmJFbExn = false;
      bool jmfQzpDEAO = false;
      bool lkexOroyZU = false;
      bool WudzDEBzhJ = false;
      bool MQzxIKpgVb = false;
      bool IrIazDcnYQ = false;
      bool PddcQnJgNu = false;
      bool oDVqCVlgoE = false;
      bool CHcyMemDAz = false;
      bool aMaQsilWfT = false;
      bool BJSZmEVRYc = false;
      bool RuzTOHBfPl = false;
      bool BFfOTbcFxT = false;
      bool jPBUCroCKH = false;
      bool IVeXgfNGBk = false;
      bool hFGsSucEFK = false;
      bool bQTFrCtdQV = false;
      bool SyUIdfaIgn = false;
      string rNNaUHowFx;
      string LppFfkyAdR;
      string IMLxSKoRcd;
      string QciyeSeNzN;
      string BaOqedZJow;
      string PDIsAfBQHK;
      string SWTafiGrZM;
      string GjAPPgxURo;
      string jebfONToxz;
      string QNGotdGnLu;
      string WhibpfCrdT;
      string hgySZnunMS;
      string ORtausdKUl;
      string ZJJWRBuzph;
      string KaLdQjPmPG;
      string iSasfEmIgw;
      string BqWrsXXbAX;
      string ruydHnGQJL;
      string dpuOCHhQpM;
      string OmSsgSESks;
      if(rNNaUHowFx == WhibpfCrdT){LmmqOnUwkr = true;}
      else if(WhibpfCrdT == rNNaUHowFx){CHcyMemDAz = true;}
      if(LppFfkyAdR == hgySZnunMS){yVyUSLwIok = true;}
      else if(hgySZnunMS == LppFfkyAdR){aMaQsilWfT = true;}
      if(IMLxSKoRcd == ORtausdKUl){oHlmJFbExn = true;}
      else if(ORtausdKUl == IMLxSKoRcd){BJSZmEVRYc = true;}
      if(QciyeSeNzN == ZJJWRBuzph){jmfQzpDEAO = true;}
      else if(ZJJWRBuzph == QciyeSeNzN){RuzTOHBfPl = true;}
      if(BaOqedZJow == KaLdQjPmPG){lkexOroyZU = true;}
      else if(KaLdQjPmPG == BaOqedZJow){BFfOTbcFxT = true;}
      if(PDIsAfBQHK == iSasfEmIgw){WudzDEBzhJ = true;}
      else if(iSasfEmIgw == PDIsAfBQHK){jPBUCroCKH = true;}
      if(SWTafiGrZM == BqWrsXXbAX){MQzxIKpgVb = true;}
      else if(BqWrsXXbAX == SWTafiGrZM){IVeXgfNGBk = true;}
      if(GjAPPgxURo == ruydHnGQJL){IrIazDcnYQ = true;}
      if(jebfONToxz == dpuOCHhQpM){PddcQnJgNu = true;}
      if(QNGotdGnLu == OmSsgSESks){oDVqCVlgoE = true;}
      while(ruydHnGQJL == GjAPPgxURo){hFGsSucEFK = true;}
      while(dpuOCHhQpM == dpuOCHhQpM){bQTFrCtdQV = true;}
      while(OmSsgSESks == OmSsgSESks){SyUIdfaIgn = true;}
      if(LmmqOnUwkr == true){LmmqOnUwkr = false;}
      if(yVyUSLwIok == true){yVyUSLwIok = false;}
      if(oHlmJFbExn == true){oHlmJFbExn = false;}
      if(jmfQzpDEAO == true){jmfQzpDEAO = false;}
      if(lkexOroyZU == true){lkexOroyZU = false;}
      if(WudzDEBzhJ == true){WudzDEBzhJ = false;}
      if(MQzxIKpgVb == true){MQzxIKpgVb = false;}
      if(IrIazDcnYQ == true){IrIazDcnYQ = false;}
      if(PddcQnJgNu == true){PddcQnJgNu = false;}
      if(oDVqCVlgoE == true){oDVqCVlgoE = false;}
      if(CHcyMemDAz == true){CHcyMemDAz = false;}
      if(aMaQsilWfT == true){aMaQsilWfT = false;}
      if(BJSZmEVRYc == true){BJSZmEVRYc = false;}
      if(RuzTOHBfPl == true){RuzTOHBfPl = false;}
      if(BFfOTbcFxT == true){BFfOTbcFxT = false;}
      if(jPBUCroCKH == true){jPBUCroCKH = false;}
      if(IVeXgfNGBk == true){IVeXgfNGBk = false;}
      if(hFGsSucEFK == true){hFGsSucEFK = false;}
      if(bQTFrCtdQV == true){bQTFrCtdQV = false;}
      if(SyUIdfaIgn == true){SyUIdfaIgn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CLUKRXEOVO
{ 
  void VjzZyIdsOV()
  { 
      bool BjqRbMHxUg = false;
      bool sZEoNeWOiA = false;
      bool xWVdHIplZb = false;
      bool syLXATomdI = false;
      bool IQSGOaofoo = false;
      bool GDSwPeONKT = false;
      bool VUWrntOfns = false;
      bool pijZqfDPOf = false;
      bool TKxkUoeOIV = false;
      bool FbcKmntZST = false;
      bool MIquzWSrFj = false;
      bool ayzdzLXkWy = false;
      bool OzXSnOGZmD = false;
      bool kdxtTKSBly = false;
      bool VmIXtGcyKa = false;
      bool PICTFMjYHj = false;
      bool PsFByxBEJn = false;
      bool sVzYbbpAYg = false;
      bool lHOLmcbcoH = false;
      bool attBhcgHZw = false;
      string PTMIYAPdYg;
      string eSshuecjem;
      string PXBASxgVju;
      string LqDihzwEpg;
      string syxFdQMzBz;
      string sABusrSLoa;
      string SssdVnPKzu;
      string iYtxtQZwVC;
      string khTjBLpHop;
      string gjPPNzETip;
      string BCzGQUGDAb;
      string RVpmpkZIbJ;
      string xHUJNyIdiu;
      string LGMfBhiNTg;
      string FlHMXLtcYc;
      string YwrYrshorR;
      string LbTFPLJJkH;
      string BMnuELAHkC;
      string EfEFVPNmmH;
      string AsVCUcRBwM;
      if(PTMIYAPdYg == BCzGQUGDAb){BjqRbMHxUg = true;}
      else if(BCzGQUGDAb == PTMIYAPdYg){MIquzWSrFj = true;}
      if(eSshuecjem == RVpmpkZIbJ){sZEoNeWOiA = true;}
      else if(RVpmpkZIbJ == eSshuecjem){ayzdzLXkWy = true;}
      if(PXBASxgVju == xHUJNyIdiu){xWVdHIplZb = true;}
      else if(xHUJNyIdiu == PXBASxgVju){OzXSnOGZmD = true;}
      if(LqDihzwEpg == LGMfBhiNTg){syLXATomdI = true;}
      else if(LGMfBhiNTg == LqDihzwEpg){kdxtTKSBly = true;}
      if(syxFdQMzBz == FlHMXLtcYc){IQSGOaofoo = true;}
      else if(FlHMXLtcYc == syxFdQMzBz){VmIXtGcyKa = true;}
      if(sABusrSLoa == YwrYrshorR){GDSwPeONKT = true;}
      else if(YwrYrshorR == sABusrSLoa){PICTFMjYHj = true;}
      if(SssdVnPKzu == LbTFPLJJkH){VUWrntOfns = true;}
      else if(LbTFPLJJkH == SssdVnPKzu){PsFByxBEJn = true;}
      if(iYtxtQZwVC == BMnuELAHkC){pijZqfDPOf = true;}
      if(khTjBLpHop == EfEFVPNmmH){TKxkUoeOIV = true;}
      if(gjPPNzETip == AsVCUcRBwM){FbcKmntZST = true;}
      while(BMnuELAHkC == iYtxtQZwVC){sVzYbbpAYg = true;}
      while(EfEFVPNmmH == EfEFVPNmmH){lHOLmcbcoH = true;}
      while(AsVCUcRBwM == AsVCUcRBwM){attBhcgHZw = true;}
      if(BjqRbMHxUg == true){BjqRbMHxUg = false;}
      if(sZEoNeWOiA == true){sZEoNeWOiA = false;}
      if(xWVdHIplZb == true){xWVdHIplZb = false;}
      if(syLXATomdI == true){syLXATomdI = false;}
      if(IQSGOaofoo == true){IQSGOaofoo = false;}
      if(GDSwPeONKT == true){GDSwPeONKT = false;}
      if(VUWrntOfns == true){VUWrntOfns = false;}
      if(pijZqfDPOf == true){pijZqfDPOf = false;}
      if(TKxkUoeOIV == true){TKxkUoeOIV = false;}
      if(FbcKmntZST == true){FbcKmntZST = false;}
      if(MIquzWSrFj == true){MIquzWSrFj = false;}
      if(ayzdzLXkWy == true){ayzdzLXkWy = false;}
      if(OzXSnOGZmD == true){OzXSnOGZmD = false;}
      if(kdxtTKSBly == true){kdxtTKSBly = false;}
      if(VmIXtGcyKa == true){VmIXtGcyKa = false;}
      if(PICTFMjYHj == true){PICTFMjYHj = false;}
      if(PsFByxBEJn == true){PsFByxBEJn = false;}
      if(sVzYbbpAYg == true){sVzYbbpAYg = false;}
      if(lHOLmcbcoH == true){lHOLmcbcoH = false;}
      if(attBhcgHZw == true){attBhcgHZw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NMONNXHITG
{ 
  void GlETdeaBOL()
  { 
      bool HMCzDzKwpX = false;
      bool kCOFGuGwqC = false;
      bool gPrOMbZiUe = false;
      bool OQBRDrHVFk = false;
      bool USLMgsgPbr = false;
      bool utEEerTnlb = false;
      bool ZbbLJTBwBO = false;
      bool mjqGhODfgT = false;
      bool tJGciTMwmL = false;
      bool YTjdLrpzCM = false;
      bool qjONELDxWQ = false;
      bool nmHRgPVzco = false;
      bool HFJpoTzMuo = false;
      bool XEcTbKurXW = false;
      bool POZVWkMSxu = false;
      bool WpWMFlZXXk = false;
      bool LpBgrjJmpV = false;
      bool UcbFYsfnTE = false;
      bool GgPASggPOo = false;
      bool jWUlLYMrdm = false;
      string KmptxAIjRp;
      string qDCbQjIPAy;
      string sVmIxLQoaX;
      string tDoVOPLeLZ;
      string txyeRxrtCc;
      string dKRKmozgxk;
      string jTJsumOgNA;
      string XcEaEwXIqz;
      string TVmysjldhZ;
      string PehZoanEid;
      string iLgBEAFpFq;
      string QqpxRNfLAL;
      string Bkzmjsudkd;
      string ITZMQIolsr;
      string dwXpJYojIb;
      string tcEXcxLsQK;
      string FbFbiwRcgJ;
      string XtRHgFMIdL;
      string EKUIjfPBjS;
      string wwUimCpwHM;
      if(KmptxAIjRp == iLgBEAFpFq){HMCzDzKwpX = true;}
      else if(iLgBEAFpFq == KmptxAIjRp){qjONELDxWQ = true;}
      if(qDCbQjIPAy == QqpxRNfLAL){kCOFGuGwqC = true;}
      else if(QqpxRNfLAL == qDCbQjIPAy){nmHRgPVzco = true;}
      if(sVmIxLQoaX == Bkzmjsudkd){gPrOMbZiUe = true;}
      else if(Bkzmjsudkd == sVmIxLQoaX){HFJpoTzMuo = true;}
      if(tDoVOPLeLZ == ITZMQIolsr){OQBRDrHVFk = true;}
      else if(ITZMQIolsr == tDoVOPLeLZ){XEcTbKurXW = true;}
      if(txyeRxrtCc == dwXpJYojIb){USLMgsgPbr = true;}
      else if(dwXpJYojIb == txyeRxrtCc){POZVWkMSxu = true;}
      if(dKRKmozgxk == tcEXcxLsQK){utEEerTnlb = true;}
      else if(tcEXcxLsQK == dKRKmozgxk){WpWMFlZXXk = true;}
      if(jTJsumOgNA == FbFbiwRcgJ){ZbbLJTBwBO = true;}
      else if(FbFbiwRcgJ == jTJsumOgNA){LpBgrjJmpV = true;}
      if(XcEaEwXIqz == XtRHgFMIdL){mjqGhODfgT = true;}
      if(TVmysjldhZ == EKUIjfPBjS){tJGciTMwmL = true;}
      if(PehZoanEid == wwUimCpwHM){YTjdLrpzCM = true;}
      while(XtRHgFMIdL == XcEaEwXIqz){UcbFYsfnTE = true;}
      while(EKUIjfPBjS == EKUIjfPBjS){GgPASggPOo = true;}
      while(wwUimCpwHM == wwUimCpwHM){jWUlLYMrdm = true;}
      if(HMCzDzKwpX == true){HMCzDzKwpX = false;}
      if(kCOFGuGwqC == true){kCOFGuGwqC = false;}
      if(gPrOMbZiUe == true){gPrOMbZiUe = false;}
      if(OQBRDrHVFk == true){OQBRDrHVFk = false;}
      if(USLMgsgPbr == true){USLMgsgPbr = false;}
      if(utEEerTnlb == true){utEEerTnlb = false;}
      if(ZbbLJTBwBO == true){ZbbLJTBwBO = false;}
      if(mjqGhODfgT == true){mjqGhODfgT = false;}
      if(tJGciTMwmL == true){tJGciTMwmL = false;}
      if(YTjdLrpzCM == true){YTjdLrpzCM = false;}
      if(qjONELDxWQ == true){qjONELDxWQ = false;}
      if(nmHRgPVzco == true){nmHRgPVzco = false;}
      if(HFJpoTzMuo == true){HFJpoTzMuo = false;}
      if(XEcTbKurXW == true){XEcTbKurXW = false;}
      if(POZVWkMSxu == true){POZVWkMSxu = false;}
      if(WpWMFlZXXk == true){WpWMFlZXXk = false;}
      if(LpBgrjJmpV == true){LpBgrjJmpV = false;}
      if(UcbFYsfnTE == true){UcbFYsfnTE = false;}
      if(GgPASggPOo == true){GgPASggPOo = false;}
      if(jWUlLYMrdm == true){jWUlLYMrdm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HFNAFGGCST
{ 
  void MSzmtIAeDA()
  { 
      bool xwzlCaWDzZ = false;
      bool yOlisiDHRT = false;
      bool hMVyqjuRZq = false;
      bool uplQoZtIHF = false;
      bool ZxZgVMTUdC = false;
      bool RkysGkZBtS = false;
      bool fFdHeIbVtS = false;
      bool ntAMsjlnbd = false;
      bool PZmWZJrsKE = false;
      bool AsNnYUTSCN = false;
      bool kyaKOWklzV = false;
      bool zkYQCGRHhU = false;
      bool QiKpStfLmY = false;
      bool QSlOsRycHC = false;
      bool XFsoEOUxlS = false;
      bool CLjgxBnkqU = false;
      bool qZzZEfzJWk = false;
      bool HozexKoVkE = false;
      bool ZhYTWEiERq = false;
      bool cNkmLxGSjX = false;
      string cNNNoJtGzU;
      string WVPgodjKkH;
      string RXNdRCxweU;
      string ZpghdJIJyw;
      string zQSktPXaaf;
      string QJJNbFAjwl;
      string MxbdtfohDf;
      string UaMnIAEbjB;
      string dWVyEPfTFY;
      string IYMMjBonpN;
      string aoWtLpxVEb;
      string AXztHpacZe;
      string zVcdXuNzDG;
      string KQlVanzbmO;
      string SKOdYjZSEs;
      string ZMOLOFMrWP;
      string cxRyQjlcTS;
      string qzoLLnGyFm;
      string VttJfxBZHU;
      string bGEXkUsQMn;
      if(cNNNoJtGzU == aoWtLpxVEb){xwzlCaWDzZ = true;}
      else if(aoWtLpxVEb == cNNNoJtGzU){kyaKOWklzV = true;}
      if(WVPgodjKkH == AXztHpacZe){yOlisiDHRT = true;}
      else if(AXztHpacZe == WVPgodjKkH){zkYQCGRHhU = true;}
      if(RXNdRCxweU == zVcdXuNzDG){hMVyqjuRZq = true;}
      else if(zVcdXuNzDG == RXNdRCxweU){QiKpStfLmY = true;}
      if(ZpghdJIJyw == KQlVanzbmO){uplQoZtIHF = true;}
      else if(KQlVanzbmO == ZpghdJIJyw){QSlOsRycHC = true;}
      if(zQSktPXaaf == SKOdYjZSEs){ZxZgVMTUdC = true;}
      else if(SKOdYjZSEs == zQSktPXaaf){XFsoEOUxlS = true;}
      if(QJJNbFAjwl == ZMOLOFMrWP){RkysGkZBtS = true;}
      else if(ZMOLOFMrWP == QJJNbFAjwl){CLjgxBnkqU = true;}
      if(MxbdtfohDf == cxRyQjlcTS){fFdHeIbVtS = true;}
      else if(cxRyQjlcTS == MxbdtfohDf){qZzZEfzJWk = true;}
      if(UaMnIAEbjB == qzoLLnGyFm){ntAMsjlnbd = true;}
      if(dWVyEPfTFY == VttJfxBZHU){PZmWZJrsKE = true;}
      if(IYMMjBonpN == bGEXkUsQMn){AsNnYUTSCN = true;}
      while(qzoLLnGyFm == UaMnIAEbjB){HozexKoVkE = true;}
      while(VttJfxBZHU == VttJfxBZHU){ZhYTWEiERq = true;}
      while(bGEXkUsQMn == bGEXkUsQMn){cNkmLxGSjX = true;}
      if(xwzlCaWDzZ == true){xwzlCaWDzZ = false;}
      if(yOlisiDHRT == true){yOlisiDHRT = false;}
      if(hMVyqjuRZq == true){hMVyqjuRZq = false;}
      if(uplQoZtIHF == true){uplQoZtIHF = false;}
      if(ZxZgVMTUdC == true){ZxZgVMTUdC = false;}
      if(RkysGkZBtS == true){RkysGkZBtS = false;}
      if(fFdHeIbVtS == true){fFdHeIbVtS = false;}
      if(ntAMsjlnbd == true){ntAMsjlnbd = false;}
      if(PZmWZJrsKE == true){PZmWZJrsKE = false;}
      if(AsNnYUTSCN == true){AsNnYUTSCN = false;}
      if(kyaKOWklzV == true){kyaKOWklzV = false;}
      if(zkYQCGRHhU == true){zkYQCGRHhU = false;}
      if(QiKpStfLmY == true){QiKpStfLmY = false;}
      if(QSlOsRycHC == true){QSlOsRycHC = false;}
      if(XFsoEOUxlS == true){XFsoEOUxlS = false;}
      if(CLjgxBnkqU == true){CLjgxBnkqU = false;}
      if(qZzZEfzJWk == true){qZzZEfzJWk = false;}
      if(HozexKoVkE == true){HozexKoVkE = false;}
      if(ZhYTWEiERq == true){ZhYTWEiERq = false;}
      if(cNkmLxGSjX == true){cNkmLxGSjX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GVUDHVKULN
{ 
  void NbKfRlnrHF()
  { 
      bool ImfdwnPXms = false;
      bool zcPFFGbiMn = false;
      bool aRkunFWhVf = false;
      bool cSJkzbMiMu = false;
      bool PqsIunsaNF = false;
      bool eXZreqBkpq = false;
      bool QhCHQfTdGc = false;
      bool tsWQYjXcAz = false;
      bool LuTPBEsQFH = false;
      bool abrVdhnlrP = false;
      bool UOTjKlBITu = false;
      bool SYjpfVNlFL = false;
      bool lCYzCSABmp = false;
      bool wkkkdwajFP = false;
      bool CkxKnBqBoI = false;
      bool UBffcSpUgr = false;
      bool kAVczhnzLi = false;
      bool tcgpIBENKB = false;
      bool ayLdVtFEHL = false;
      bool GGCMdCPTKG = false;
      string LYXUQxwfjI;
      string qrDeSkZqtc;
      string BixLjZEaqR;
      string qowpipDxSi;
      string LwQcQpZqEq;
      string qJZoWcbLjq;
      string rUoSRBuMuc;
      string gbXLRERTtt;
      string zNWzZgPGTG;
      string OqMbppHxLP;
      string tNwbKOkOXR;
      string KHRpreyprj;
      string losnOoPwUa;
      string xTIeAlQtnK;
      string wBiBwGZWiK;
      string rQFacFTHNU;
      string zoswUpympF;
      string ODBgnjFqrH;
      string PrMJifOgBt;
      string RJgTLHHlgG;
      if(LYXUQxwfjI == tNwbKOkOXR){ImfdwnPXms = true;}
      else if(tNwbKOkOXR == LYXUQxwfjI){UOTjKlBITu = true;}
      if(qrDeSkZqtc == KHRpreyprj){zcPFFGbiMn = true;}
      else if(KHRpreyprj == qrDeSkZqtc){SYjpfVNlFL = true;}
      if(BixLjZEaqR == losnOoPwUa){aRkunFWhVf = true;}
      else if(losnOoPwUa == BixLjZEaqR){lCYzCSABmp = true;}
      if(qowpipDxSi == xTIeAlQtnK){cSJkzbMiMu = true;}
      else if(xTIeAlQtnK == qowpipDxSi){wkkkdwajFP = true;}
      if(LwQcQpZqEq == wBiBwGZWiK){PqsIunsaNF = true;}
      else if(wBiBwGZWiK == LwQcQpZqEq){CkxKnBqBoI = true;}
      if(qJZoWcbLjq == rQFacFTHNU){eXZreqBkpq = true;}
      else if(rQFacFTHNU == qJZoWcbLjq){UBffcSpUgr = true;}
      if(rUoSRBuMuc == zoswUpympF){QhCHQfTdGc = true;}
      else if(zoswUpympF == rUoSRBuMuc){kAVczhnzLi = true;}
      if(gbXLRERTtt == ODBgnjFqrH){tsWQYjXcAz = true;}
      if(zNWzZgPGTG == PrMJifOgBt){LuTPBEsQFH = true;}
      if(OqMbppHxLP == RJgTLHHlgG){abrVdhnlrP = true;}
      while(ODBgnjFqrH == gbXLRERTtt){tcgpIBENKB = true;}
      while(PrMJifOgBt == PrMJifOgBt){ayLdVtFEHL = true;}
      while(RJgTLHHlgG == RJgTLHHlgG){GGCMdCPTKG = true;}
      if(ImfdwnPXms == true){ImfdwnPXms = false;}
      if(zcPFFGbiMn == true){zcPFFGbiMn = false;}
      if(aRkunFWhVf == true){aRkunFWhVf = false;}
      if(cSJkzbMiMu == true){cSJkzbMiMu = false;}
      if(PqsIunsaNF == true){PqsIunsaNF = false;}
      if(eXZreqBkpq == true){eXZreqBkpq = false;}
      if(QhCHQfTdGc == true){QhCHQfTdGc = false;}
      if(tsWQYjXcAz == true){tsWQYjXcAz = false;}
      if(LuTPBEsQFH == true){LuTPBEsQFH = false;}
      if(abrVdhnlrP == true){abrVdhnlrP = false;}
      if(UOTjKlBITu == true){UOTjKlBITu = false;}
      if(SYjpfVNlFL == true){SYjpfVNlFL = false;}
      if(lCYzCSABmp == true){lCYzCSABmp = false;}
      if(wkkkdwajFP == true){wkkkdwajFP = false;}
      if(CkxKnBqBoI == true){CkxKnBqBoI = false;}
      if(UBffcSpUgr == true){UBffcSpUgr = false;}
      if(kAVczhnzLi == true){kAVczhnzLi = false;}
      if(tcgpIBENKB == true){tcgpIBENKB = false;}
      if(ayLdVtFEHL == true){ayLdVtFEHL = false;}
      if(GGCMdCPTKG == true){GGCMdCPTKG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QJGELWAMWI
{ 
  void TUNkOasJZH()
  { 
      bool AXkMncaGRq = false;
      bool WZSSsKcxYj = false;
      bool ZxVObOiEoO = false;
      bool pNiODVpgXV = false;
      bool UEXNciiilx = false;
      bool TPgbdCBVkZ = false;
      bool sWRDBIpDPX = false;
      bool UwYDOUZIls = false;
      bool eARqtZfHkA = false;
      bool UUwybymTLT = false;
      bool rfSOcXpdMI = false;
      bool crjqtsEZIa = false;
      bool biSOCzAabQ = false;
      bool gFwSBzgDNW = false;
      bool iCtbXbjEEh = false;
      bool tNKjqIMELd = false;
      bool WPCwbWwQpu = false;
      bool LALlqZoqqa = false;
      bool OwVsVTtMar = false;
      bool mcqjzTzyAA = false;
      string qsaQSAgANO;
      string jlOHTuZPUz;
      string oXGZeRRlxH;
      string zGcsKgbrMr;
      string RRGaByenAy;
      string wQFQLTmAlA;
      string huCFPcJVxl;
      string ShCbhWReYf;
      string ldlyKhBLgp;
      string taFLJVOALX;
      string oIqNeONcin;
      string CHbFNEIDtg;
      string bxnGqrbqSL;
      string HQTAqdmXSw;
      string UXcsnmyzxF;
      string NLFduqUjWm;
      string mWKWgkmaOj;
      string QjjyMDTuRD;
      string sbsMywLoFz;
      string ICBfIxNGLD;
      if(qsaQSAgANO == oIqNeONcin){AXkMncaGRq = true;}
      else if(oIqNeONcin == qsaQSAgANO){rfSOcXpdMI = true;}
      if(jlOHTuZPUz == CHbFNEIDtg){WZSSsKcxYj = true;}
      else if(CHbFNEIDtg == jlOHTuZPUz){crjqtsEZIa = true;}
      if(oXGZeRRlxH == bxnGqrbqSL){ZxVObOiEoO = true;}
      else if(bxnGqrbqSL == oXGZeRRlxH){biSOCzAabQ = true;}
      if(zGcsKgbrMr == HQTAqdmXSw){pNiODVpgXV = true;}
      else if(HQTAqdmXSw == zGcsKgbrMr){gFwSBzgDNW = true;}
      if(RRGaByenAy == UXcsnmyzxF){UEXNciiilx = true;}
      else if(UXcsnmyzxF == RRGaByenAy){iCtbXbjEEh = true;}
      if(wQFQLTmAlA == NLFduqUjWm){TPgbdCBVkZ = true;}
      else if(NLFduqUjWm == wQFQLTmAlA){tNKjqIMELd = true;}
      if(huCFPcJVxl == mWKWgkmaOj){sWRDBIpDPX = true;}
      else if(mWKWgkmaOj == huCFPcJVxl){WPCwbWwQpu = true;}
      if(ShCbhWReYf == QjjyMDTuRD){UwYDOUZIls = true;}
      if(ldlyKhBLgp == sbsMywLoFz){eARqtZfHkA = true;}
      if(taFLJVOALX == ICBfIxNGLD){UUwybymTLT = true;}
      while(QjjyMDTuRD == ShCbhWReYf){LALlqZoqqa = true;}
      while(sbsMywLoFz == sbsMywLoFz){OwVsVTtMar = true;}
      while(ICBfIxNGLD == ICBfIxNGLD){mcqjzTzyAA = true;}
      if(AXkMncaGRq == true){AXkMncaGRq = false;}
      if(WZSSsKcxYj == true){WZSSsKcxYj = false;}
      if(ZxVObOiEoO == true){ZxVObOiEoO = false;}
      if(pNiODVpgXV == true){pNiODVpgXV = false;}
      if(UEXNciiilx == true){UEXNciiilx = false;}
      if(TPgbdCBVkZ == true){TPgbdCBVkZ = false;}
      if(sWRDBIpDPX == true){sWRDBIpDPX = false;}
      if(UwYDOUZIls == true){UwYDOUZIls = false;}
      if(eARqtZfHkA == true){eARqtZfHkA = false;}
      if(UUwybymTLT == true){UUwybymTLT = false;}
      if(rfSOcXpdMI == true){rfSOcXpdMI = false;}
      if(crjqtsEZIa == true){crjqtsEZIa = false;}
      if(biSOCzAabQ == true){biSOCzAabQ = false;}
      if(gFwSBzgDNW == true){gFwSBzgDNW = false;}
      if(iCtbXbjEEh == true){iCtbXbjEEh = false;}
      if(tNKjqIMELd == true){tNKjqIMELd = false;}
      if(WPCwbWwQpu == true){WPCwbWwQpu = false;}
      if(LALlqZoqqa == true){LALlqZoqqa = false;}
      if(OwVsVTtMar == true){OwVsVTtMar = false;}
      if(mcqjzTzyAA == true){mcqjzTzyAA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IAVPZWXQNA
{ 
  void IEAuNDMxYK()
  { 
      bool bHrjHfwCSD = false;
      bool wGTcsizcUq = false;
      bool IZhbFBLCax = false;
      bool nnwnZuhLVL = false;
      bool RgPrTBnkMV = false;
      bool BlqgwCZQeG = false;
      bool TsaUOIJfDy = false;
      bool BnHJZaKpID = false;
      bool MapEwBwOrW = false;
      bool VRgOqXFucL = false;
      bool qIgOUMdhHU = false;
      bool qiHatsDWYM = false;
      bool LunfbiHsRB = false;
      bool VPNxLDsbii = false;
      bool XNGWelKFUy = false;
      bool UhVPzwUAll = false;
      bool FkQFybUJOp = false;
      bool zampoxnQAp = false;
      bool XBYSgmsxyP = false;
      bool irpxcmxBYD = false;
      string UYbQzPoXwz;
      string RjLGpkltmg;
      string nNFOYpPIcM;
      string RSWHMLeiEu;
      string HIPtfletxq;
      string ZeskEHSNrI;
      string zgujhzMDte;
      string FCpqYfxLhl;
      string KVLlmlYlDF;
      string rQquwaNfMg;
      string YbARkWoCLY;
      string JHeFxdUIwu;
      string ZaAnloLtuy;
      string SRzBNiUtBQ;
      string wWZewjlkyR;
      string GEiQGxaMew;
      string KuGMFqIdrN;
      string JaHfyDBUrB;
      string DaudrNihuP;
      string HIOFFBPMYE;
      if(UYbQzPoXwz == YbARkWoCLY){bHrjHfwCSD = true;}
      else if(YbARkWoCLY == UYbQzPoXwz){qIgOUMdhHU = true;}
      if(RjLGpkltmg == JHeFxdUIwu){wGTcsizcUq = true;}
      else if(JHeFxdUIwu == RjLGpkltmg){qiHatsDWYM = true;}
      if(nNFOYpPIcM == ZaAnloLtuy){IZhbFBLCax = true;}
      else if(ZaAnloLtuy == nNFOYpPIcM){LunfbiHsRB = true;}
      if(RSWHMLeiEu == SRzBNiUtBQ){nnwnZuhLVL = true;}
      else if(SRzBNiUtBQ == RSWHMLeiEu){VPNxLDsbii = true;}
      if(HIPtfletxq == wWZewjlkyR){RgPrTBnkMV = true;}
      else if(wWZewjlkyR == HIPtfletxq){XNGWelKFUy = true;}
      if(ZeskEHSNrI == GEiQGxaMew){BlqgwCZQeG = true;}
      else if(GEiQGxaMew == ZeskEHSNrI){UhVPzwUAll = true;}
      if(zgujhzMDte == KuGMFqIdrN){TsaUOIJfDy = true;}
      else if(KuGMFqIdrN == zgujhzMDte){FkQFybUJOp = true;}
      if(FCpqYfxLhl == JaHfyDBUrB){BnHJZaKpID = true;}
      if(KVLlmlYlDF == DaudrNihuP){MapEwBwOrW = true;}
      if(rQquwaNfMg == HIOFFBPMYE){VRgOqXFucL = true;}
      while(JaHfyDBUrB == FCpqYfxLhl){zampoxnQAp = true;}
      while(DaudrNihuP == DaudrNihuP){XBYSgmsxyP = true;}
      while(HIOFFBPMYE == HIOFFBPMYE){irpxcmxBYD = true;}
      if(bHrjHfwCSD == true){bHrjHfwCSD = false;}
      if(wGTcsizcUq == true){wGTcsizcUq = false;}
      if(IZhbFBLCax == true){IZhbFBLCax = false;}
      if(nnwnZuhLVL == true){nnwnZuhLVL = false;}
      if(RgPrTBnkMV == true){RgPrTBnkMV = false;}
      if(BlqgwCZQeG == true){BlqgwCZQeG = false;}
      if(TsaUOIJfDy == true){TsaUOIJfDy = false;}
      if(BnHJZaKpID == true){BnHJZaKpID = false;}
      if(MapEwBwOrW == true){MapEwBwOrW = false;}
      if(VRgOqXFucL == true){VRgOqXFucL = false;}
      if(qIgOUMdhHU == true){qIgOUMdhHU = false;}
      if(qiHatsDWYM == true){qiHatsDWYM = false;}
      if(LunfbiHsRB == true){LunfbiHsRB = false;}
      if(VPNxLDsbii == true){VPNxLDsbii = false;}
      if(XNGWelKFUy == true){XNGWelKFUy = false;}
      if(UhVPzwUAll == true){UhVPzwUAll = false;}
      if(FkQFybUJOp == true){FkQFybUJOp = false;}
      if(zampoxnQAp == true){zampoxnQAp = false;}
      if(XBYSgmsxyP == true){XBYSgmsxyP = false;}
      if(irpxcmxBYD == true){irpxcmxBYD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QSWXDGGIRI
{ 
  void fVPTrCYmZy()
  { 
      bool CEfMFJYiRt = false;
      bool aCxdDaRaZJ = false;
      bool DNEDsLbnSC = false;
      bool UAxbhadrhr = false;
      bool bZOwMHCQMQ = false;
      bool iYZJUnzbSs = false;
      bool gkQPYaFUwc = false;
      bool xYsggczIqk = false;
      bool JKaBTwZnHs = false;
      bool bBZnKpJzWl = false;
      bool CgUjTJfrbq = false;
      bool opkgjWJOuc = false;
      bool AQNDAuzFWq = false;
      bool TzkFibpqhJ = false;
      bool QgLPDkQrDn = false;
      bool ftDtkkTQiX = false;
      bool JZVkXoEeyI = false;
      bool gSflLTFyAP = false;
      bool eHBiKYwqUx = false;
      bool IZHKzpLaqN = false;
      string nVdNntMfFO;
      string uGmjGpVouz;
      string VqeWlQDYKZ;
      string ZerZyhJHVQ;
      string CFJiQRZVTu;
      string fKxGGxpHbX;
      string pmUeoQNMlX;
      string gNSdxPguFT;
      string pgFOtOgkTB;
      string qBfxZOjdtz;
      string EMiuOSqSJp;
      string PuBHgJhFkZ;
      string jbskfehUEh;
      string fCAGPdxKAb;
      string WObckwzXPQ;
      string gesUPbSxOi;
      string jWOaKMturD;
      string LXFJCUAQAj;
      string eJEmBBJfpn;
      string KzuQqAAASX;
      if(nVdNntMfFO == EMiuOSqSJp){CEfMFJYiRt = true;}
      else if(EMiuOSqSJp == nVdNntMfFO){CgUjTJfrbq = true;}
      if(uGmjGpVouz == PuBHgJhFkZ){aCxdDaRaZJ = true;}
      else if(PuBHgJhFkZ == uGmjGpVouz){opkgjWJOuc = true;}
      if(VqeWlQDYKZ == jbskfehUEh){DNEDsLbnSC = true;}
      else if(jbskfehUEh == VqeWlQDYKZ){AQNDAuzFWq = true;}
      if(ZerZyhJHVQ == fCAGPdxKAb){UAxbhadrhr = true;}
      else if(fCAGPdxKAb == ZerZyhJHVQ){TzkFibpqhJ = true;}
      if(CFJiQRZVTu == WObckwzXPQ){bZOwMHCQMQ = true;}
      else if(WObckwzXPQ == CFJiQRZVTu){QgLPDkQrDn = true;}
      if(fKxGGxpHbX == gesUPbSxOi){iYZJUnzbSs = true;}
      else if(gesUPbSxOi == fKxGGxpHbX){ftDtkkTQiX = true;}
      if(pmUeoQNMlX == jWOaKMturD){gkQPYaFUwc = true;}
      else if(jWOaKMturD == pmUeoQNMlX){JZVkXoEeyI = true;}
      if(gNSdxPguFT == LXFJCUAQAj){xYsggczIqk = true;}
      if(pgFOtOgkTB == eJEmBBJfpn){JKaBTwZnHs = true;}
      if(qBfxZOjdtz == KzuQqAAASX){bBZnKpJzWl = true;}
      while(LXFJCUAQAj == gNSdxPguFT){gSflLTFyAP = true;}
      while(eJEmBBJfpn == eJEmBBJfpn){eHBiKYwqUx = true;}
      while(KzuQqAAASX == KzuQqAAASX){IZHKzpLaqN = true;}
      if(CEfMFJYiRt == true){CEfMFJYiRt = false;}
      if(aCxdDaRaZJ == true){aCxdDaRaZJ = false;}
      if(DNEDsLbnSC == true){DNEDsLbnSC = false;}
      if(UAxbhadrhr == true){UAxbhadrhr = false;}
      if(bZOwMHCQMQ == true){bZOwMHCQMQ = false;}
      if(iYZJUnzbSs == true){iYZJUnzbSs = false;}
      if(gkQPYaFUwc == true){gkQPYaFUwc = false;}
      if(xYsggczIqk == true){xYsggczIqk = false;}
      if(JKaBTwZnHs == true){JKaBTwZnHs = false;}
      if(bBZnKpJzWl == true){bBZnKpJzWl = false;}
      if(CgUjTJfrbq == true){CgUjTJfrbq = false;}
      if(opkgjWJOuc == true){opkgjWJOuc = false;}
      if(AQNDAuzFWq == true){AQNDAuzFWq = false;}
      if(TzkFibpqhJ == true){TzkFibpqhJ = false;}
      if(QgLPDkQrDn == true){QgLPDkQrDn = false;}
      if(ftDtkkTQiX == true){ftDtkkTQiX = false;}
      if(JZVkXoEeyI == true){JZVkXoEeyI = false;}
      if(gSflLTFyAP == true){gSflLTFyAP = false;}
      if(eHBiKYwqUx == true){eHBiKYwqUx = false;}
      if(IZHKzpLaqN == true){IZHKzpLaqN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GQXVDASMZD
{ 
  void dWVEsryRVD()
  { 
      bool zjJfcpffFr = false;
      bool gSaQkQGVtU = false;
      bool PaTwYcwFxP = false;
      bool kxGCJewWHs = false;
      bool yJsVMJfYzi = false;
      bool uRziNYQNHz = false;
      bool tZhoHjMXGG = false;
      bool UiNBWGgxiO = false;
      bool xWGNUSyWnk = false;
      bool OhOIyYaDOb = false;
      bool xISrOLExMH = false;
      bool qSdsCXSTTo = false;
      bool koiVrlUFDy = false;
      bool ediHRnDNzJ = false;
      bool OAKgtwDVFZ = false;
      bool FGiZqHYxAb = false;
      bool qytNbNiWaN = false;
      bool TEUJZjdCbw = false;
      bool iyAcrYtJPJ = false;
      bool PGNWSPGwEf = false;
      string XmrhRSKWMX;
      string dZQSsbgbHy;
      string zjQOUIgjXq;
      string iMxAupKTzx;
      string GzSqAQIuLB;
      string pZOhynVcMI;
      string kazUnQcPWC;
      string QNcJrzQZOE;
      string AlYVAJyOBG;
      string UhHpLezyCi;
      string mrXIjcwEpy;
      string XwOHIyCcUe;
      string SONPhiGlxM;
      string ASWeRHOQlW;
      string MtXCiYTzhM;
      string cWcXQXdHZB;
      string WKGdyFYioB;
      string jlqxRXqKgJ;
      string PGDkidoyOs;
      string cGpkABmsEd;
      if(XmrhRSKWMX == mrXIjcwEpy){zjJfcpffFr = true;}
      else if(mrXIjcwEpy == XmrhRSKWMX){xISrOLExMH = true;}
      if(dZQSsbgbHy == XwOHIyCcUe){gSaQkQGVtU = true;}
      else if(XwOHIyCcUe == dZQSsbgbHy){qSdsCXSTTo = true;}
      if(zjQOUIgjXq == SONPhiGlxM){PaTwYcwFxP = true;}
      else if(SONPhiGlxM == zjQOUIgjXq){koiVrlUFDy = true;}
      if(iMxAupKTzx == ASWeRHOQlW){kxGCJewWHs = true;}
      else if(ASWeRHOQlW == iMxAupKTzx){ediHRnDNzJ = true;}
      if(GzSqAQIuLB == MtXCiYTzhM){yJsVMJfYzi = true;}
      else if(MtXCiYTzhM == GzSqAQIuLB){OAKgtwDVFZ = true;}
      if(pZOhynVcMI == cWcXQXdHZB){uRziNYQNHz = true;}
      else if(cWcXQXdHZB == pZOhynVcMI){FGiZqHYxAb = true;}
      if(kazUnQcPWC == WKGdyFYioB){tZhoHjMXGG = true;}
      else if(WKGdyFYioB == kazUnQcPWC){qytNbNiWaN = true;}
      if(QNcJrzQZOE == jlqxRXqKgJ){UiNBWGgxiO = true;}
      if(AlYVAJyOBG == PGDkidoyOs){xWGNUSyWnk = true;}
      if(UhHpLezyCi == cGpkABmsEd){OhOIyYaDOb = true;}
      while(jlqxRXqKgJ == QNcJrzQZOE){TEUJZjdCbw = true;}
      while(PGDkidoyOs == PGDkidoyOs){iyAcrYtJPJ = true;}
      while(cGpkABmsEd == cGpkABmsEd){PGNWSPGwEf = true;}
      if(zjJfcpffFr == true){zjJfcpffFr = false;}
      if(gSaQkQGVtU == true){gSaQkQGVtU = false;}
      if(PaTwYcwFxP == true){PaTwYcwFxP = false;}
      if(kxGCJewWHs == true){kxGCJewWHs = false;}
      if(yJsVMJfYzi == true){yJsVMJfYzi = false;}
      if(uRziNYQNHz == true){uRziNYQNHz = false;}
      if(tZhoHjMXGG == true){tZhoHjMXGG = false;}
      if(UiNBWGgxiO == true){UiNBWGgxiO = false;}
      if(xWGNUSyWnk == true){xWGNUSyWnk = false;}
      if(OhOIyYaDOb == true){OhOIyYaDOb = false;}
      if(xISrOLExMH == true){xISrOLExMH = false;}
      if(qSdsCXSTTo == true){qSdsCXSTTo = false;}
      if(koiVrlUFDy == true){koiVrlUFDy = false;}
      if(ediHRnDNzJ == true){ediHRnDNzJ = false;}
      if(OAKgtwDVFZ == true){OAKgtwDVFZ = false;}
      if(FGiZqHYxAb == true){FGiZqHYxAb = false;}
      if(qytNbNiWaN == true){qytNbNiWaN = false;}
      if(TEUJZjdCbw == true){TEUJZjdCbw = false;}
      if(iyAcrYtJPJ == true){iyAcrYtJPJ = false;}
      if(PGNWSPGwEf == true){PGNWSPGwEf = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FHXXJLWVXC
{ 
  void ZIlrqbqFwY()
  { 
      bool mzYpyOuwmb = false;
      bool BEHYFQHElY = false;
      bool pUNECEtNuy = false;
      bool yXtrQDWcsU = false;
      bool IYuZyiyJtb = false;
      bool QMkgkRlTms = false;
      bool ThAnddKOIB = false;
      bool seLQQQYmGS = false;
      bool FkSShklMHy = false;
      bool RCozAaXHwU = false;
      bool YiAsilBTQT = false;
      bool ryNTmiOgGr = false;
      bool qxHJjpoLPp = false;
      bool sRqltWjZju = false;
      bool jSwnFSoUMR = false;
      bool LpADdabqIr = false;
      bool wpEaaaHaYQ = false;
      bool znxKIYBTEQ = false;
      bool ATzydCQnIT = false;
      bool tsQrztNVMe = false;
      string AjEcwXITVP;
      string CjLRpxPyRT;
      string gzKbwdMjKj;
      string QNDEFKCKiN;
      string HoPhLQuuBX;
      string qpBLgBYbgS;
      string LYJlzofGLe;
      string tEPDLricQf;
      string hRhlraLrXE;
      string ladQCifdiV;
      string jniWcmZxzo;
      string RwKAjspWsX;
      string dniHsTZzBc;
      string MFstwshLJF;
      string ANshmoPdfJ;
      string sIjKrwFbcJ;
      string MawVbLCPEp;
      string KdNnXPtATD;
      string DwSqjDVhCz;
      string nGizrYRWGa;
      if(AjEcwXITVP == jniWcmZxzo){mzYpyOuwmb = true;}
      else if(jniWcmZxzo == AjEcwXITVP){YiAsilBTQT = true;}
      if(CjLRpxPyRT == RwKAjspWsX){BEHYFQHElY = true;}
      else if(RwKAjspWsX == CjLRpxPyRT){ryNTmiOgGr = true;}
      if(gzKbwdMjKj == dniHsTZzBc){pUNECEtNuy = true;}
      else if(dniHsTZzBc == gzKbwdMjKj){qxHJjpoLPp = true;}
      if(QNDEFKCKiN == MFstwshLJF){yXtrQDWcsU = true;}
      else if(MFstwshLJF == QNDEFKCKiN){sRqltWjZju = true;}
      if(HoPhLQuuBX == ANshmoPdfJ){IYuZyiyJtb = true;}
      else if(ANshmoPdfJ == HoPhLQuuBX){jSwnFSoUMR = true;}
      if(qpBLgBYbgS == sIjKrwFbcJ){QMkgkRlTms = true;}
      else if(sIjKrwFbcJ == qpBLgBYbgS){LpADdabqIr = true;}
      if(LYJlzofGLe == MawVbLCPEp){ThAnddKOIB = true;}
      else if(MawVbLCPEp == LYJlzofGLe){wpEaaaHaYQ = true;}
      if(tEPDLricQf == KdNnXPtATD){seLQQQYmGS = true;}
      if(hRhlraLrXE == DwSqjDVhCz){FkSShklMHy = true;}
      if(ladQCifdiV == nGizrYRWGa){RCozAaXHwU = true;}
      while(KdNnXPtATD == tEPDLricQf){znxKIYBTEQ = true;}
      while(DwSqjDVhCz == DwSqjDVhCz){ATzydCQnIT = true;}
      while(nGizrYRWGa == nGizrYRWGa){tsQrztNVMe = true;}
      if(mzYpyOuwmb == true){mzYpyOuwmb = false;}
      if(BEHYFQHElY == true){BEHYFQHElY = false;}
      if(pUNECEtNuy == true){pUNECEtNuy = false;}
      if(yXtrQDWcsU == true){yXtrQDWcsU = false;}
      if(IYuZyiyJtb == true){IYuZyiyJtb = false;}
      if(QMkgkRlTms == true){QMkgkRlTms = false;}
      if(ThAnddKOIB == true){ThAnddKOIB = false;}
      if(seLQQQYmGS == true){seLQQQYmGS = false;}
      if(FkSShklMHy == true){FkSShklMHy = false;}
      if(RCozAaXHwU == true){RCozAaXHwU = false;}
      if(YiAsilBTQT == true){YiAsilBTQT = false;}
      if(ryNTmiOgGr == true){ryNTmiOgGr = false;}
      if(qxHJjpoLPp == true){qxHJjpoLPp = false;}
      if(sRqltWjZju == true){sRqltWjZju = false;}
      if(jSwnFSoUMR == true){jSwnFSoUMR = false;}
      if(LpADdabqIr == true){LpADdabqIr = false;}
      if(wpEaaaHaYQ == true){wpEaaaHaYQ = false;}
      if(znxKIYBTEQ == true){znxKIYBTEQ = false;}
      if(ATzydCQnIT == true){ATzydCQnIT = false;}
      if(tsQrztNVMe == true){tsQrztNVMe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BQRSBTCKQL
{ 
  void QqRuMTOzuy()
  { 
      bool bSkQLAyVMU = false;
      bool LJkzRngILE = false;
      bool eUqgmaRCDd = false;
      bool tXokexCRIj = false;
      bool znhEXVLZaS = false;
      bool EUVRpHaEck = false;
      bool iwShamUATT = false;
      bool kOAbEplmIR = false;
      bool PqbjZZCWtX = false;
      bool WyoWADVAdj = false;
      bool BKGGhoTsMA = false;
      bool UnuLUqTiHA = false;
      bool kxGuwlZcbA = false;
      bool UlIYQEkVBj = false;
      bool FIKsDhDdCI = false;
      bool jlrcyDEKJu = false;
      bool PpRLfieWtQ = false;
      bool FygWucmMrO = false;
      bool iuaSQhsfZM = false;
      bool zaLGUPpyMU = false;
      string lPKXtdWeYo;
      string xKdCHIGAON;
      string wlLkySRPEd;
      string nbUXrfrueK;
      string VoUussMSex;
      string cElQCzDZYa;
      string LHatBUoGJa;
      string QLVDqGRQqF;
      string GyjGAipEUr;
      string cRVDyOOZtY;
      string DFJEgRhhYu;
      string hliTTNWUsR;
      string ekkQyzpSFc;
      string rrWSCAofZi;
      string BHeZKqiCjk;
      string rsgbsSUrGh;
      string bPcwDoOQQS;
      string YqtKPXGLGH;
      string PnhTjyKtYj;
      string fxYrHkdhVU;
      if(lPKXtdWeYo == DFJEgRhhYu){bSkQLAyVMU = true;}
      else if(DFJEgRhhYu == lPKXtdWeYo){BKGGhoTsMA = true;}
      if(xKdCHIGAON == hliTTNWUsR){LJkzRngILE = true;}
      else if(hliTTNWUsR == xKdCHIGAON){UnuLUqTiHA = true;}
      if(wlLkySRPEd == ekkQyzpSFc){eUqgmaRCDd = true;}
      else if(ekkQyzpSFc == wlLkySRPEd){kxGuwlZcbA = true;}
      if(nbUXrfrueK == rrWSCAofZi){tXokexCRIj = true;}
      else if(rrWSCAofZi == nbUXrfrueK){UlIYQEkVBj = true;}
      if(VoUussMSex == BHeZKqiCjk){znhEXVLZaS = true;}
      else if(BHeZKqiCjk == VoUussMSex){FIKsDhDdCI = true;}
      if(cElQCzDZYa == rsgbsSUrGh){EUVRpHaEck = true;}
      else if(rsgbsSUrGh == cElQCzDZYa){jlrcyDEKJu = true;}
      if(LHatBUoGJa == bPcwDoOQQS){iwShamUATT = true;}
      else if(bPcwDoOQQS == LHatBUoGJa){PpRLfieWtQ = true;}
      if(QLVDqGRQqF == YqtKPXGLGH){kOAbEplmIR = true;}
      if(GyjGAipEUr == PnhTjyKtYj){PqbjZZCWtX = true;}
      if(cRVDyOOZtY == fxYrHkdhVU){WyoWADVAdj = true;}
      while(YqtKPXGLGH == QLVDqGRQqF){FygWucmMrO = true;}
      while(PnhTjyKtYj == PnhTjyKtYj){iuaSQhsfZM = true;}
      while(fxYrHkdhVU == fxYrHkdhVU){zaLGUPpyMU = true;}
      if(bSkQLAyVMU == true){bSkQLAyVMU = false;}
      if(LJkzRngILE == true){LJkzRngILE = false;}
      if(eUqgmaRCDd == true){eUqgmaRCDd = false;}
      if(tXokexCRIj == true){tXokexCRIj = false;}
      if(znhEXVLZaS == true){znhEXVLZaS = false;}
      if(EUVRpHaEck == true){EUVRpHaEck = false;}
      if(iwShamUATT == true){iwShamUATT = false;}
      if(kOAbEplmIR == true){kOAbEplmIR = false;}
      if(PqbjZZCWtX == true){PqbjZZCWtX = false;}
      if(WyoWADVAdj == true){WyoWADVAdj = false;}
      if(BKGGhoTsMA == true){BKGGhoTsMA = false;}
      if(UnuLUqTiHA == true){UnuLUqTiHA = false;}
      if(kxGuwlZcbA == true){kxGuwlZcbA = false;}
      if(UlIYQEkVBj == true){UlIYQEkVBj = false;}
      if(FIKsDhDdCI == true){FIKsDhDdCI = false;}
      if(jlrcyDEKJu == true){jlrcyDEKJu = false;}
      if(PpRLfieWtQ == true){PpRLfieWtQ = false;}
      if(FygWucmMrO == true){FygWucmMrO = false;}
      if(iuaSQhsfZM == true){iuaSQhsfZM = false;}
      if(zaLGUPpyMU == true){zaLGUPpyMU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CCUTSSNLIA
{ 
  void EnLYOdLhEo()
  { 
      bool EhwUPrHnlH = false;
      bool eUWIqyPCLh = false;
      bool IFUhRiqzTH = false;
      bool CYZUeUPyWK = false;
      bool VSxjfLYtuT = false;
      bool txJBhzNODf = false;
      bool AHTeFFuBcW = false;
      bool hReALpHjCG = false;
      bool KxDANEPXmf = false;
      bool HBtpJNySeZ = false;
      bool PWQYlbSIpr = false;
      bool qcMxVDESQA = false;
      bool pWOxOfWnqx = false;
      bool qoFbpmfdOk = false;
      bool lGhtndowUG = false;
      bool JfCdjsITJF = false;
      bool EdRrACXjHh = false;
      bool WZEeNjnTIk = false;
      bool qaSjmLmarw = false;
      bool eMTIgHopsl = false;
      string ogWbGmyNic;
      string YdWoKhFreA;
      string nVxFLFzqVa;
      string BVKrVIaidr;
      string iEyrDWMTYy;
      string jkJVQTjRQB;
      string nGqHbCTeTC;
      string GVOlRiXFCn;
      string reRXqPLnaC;
      string YJSbEbMpQA;
      string DhAWqNVWEg;
      string wdOcIJoSPL;
      string peDRdxCmho;
      string WgoBjKdugc;
      string snjtFttpSu;
      string lsaEJYdirs;
      string LioOKOoMsx;
      string etllRBtJLj;
      string CMTmDcGjrK;
      string fuCmRJzVip;
      if(ogWbGmyNic == DhAWqNVWEg){EhwUPrHnlH = true;}
      else if(DhAWqNVWEg == ogWbGmyNic){PWQYlbSIpr = true;}
      if(YdWoKhFreA == wdOcIJoSPL){eUWIqyPCLh = true;}
      else if(wdOcIJoSPL == YdWoKhFreA){qcMxVDESQA = true;}
      if(nVxFLFzqVa == peDRdxCmho){IFUhRiqzTH = true;}
      else if(peDRdxCmho == nVxFLFzqVa){pWOxOfWnqx = true;}
      if(BVKrVIaidr == WgoBjKdugc){CYZUeUPyWK = true;}
      else if(WgoBjKdugc == BVKrVIaidr){qoFbpmfdOk = true;}
      if(iEyrDWMTYy == snjtFttpSu){VSxjfLYtuT = true;}
      else if(snjtFttpSu == iEyrDWMTYy){lGhtndowUG = true;}
      if(jkJVQTjRQB == lsaEJYdirs){txJBhzNODf = true;}
      else if(lsaEJYdirs == jkJVQTjRQB){JfCdjsITJF = true;}
      if(nGqHbCTeTC == LioOKOoMsx){AHTeFFuBcW = true;}
      else if(LioOKOoMsx == nGqHbCTeTC){EdRrACXjHh = true;}
      if(GVOlRiXFCn == etllRBtJLj){hReALpHjCG = true;}
      if(reRXqPLnaC == CMTmDcGjrK){KxDANEPXmf = true;}
      if(YJSbEbMpQA == fuCmRJzVip){HBtpJNySeZ = true;}
      while(etllRBtJLj == GVOlRiXFCn){WZEeNjnTIk = true;}
      while(CMTmDcGjrK == CMTmDcGjrK){qaSjmLmarw = true;}
      while(fuCmRJzVip == fuCmRJzVip){eMTIgHopsl = true;}
      if(EhwUPrHnlH == true){EhwUPrHnlH = false;}
      if(eUWIqyPCLh == true){eUWIqyPCLh = false;}
      if(IFUhRiqzTH == true){IFUhRiqzTH = false;}
      if(CYZUeUPyWK == true){CYZUeUPyWK = false;}
      if(VSxjfLYtuT == true){VSxjfLYtuT = false;}
      if(txJBhzNODf == true){txJBhzNODf = false;}
      if(AHTeFFuBcW == true){AHTeFFuBcW = false;}
      if(hReALpHjCG == true){hReALpHjCG = false;}
      if(KxDANEPXmf == true){KxDANEPXmf = false;}
      if(HBtpJNySeZ == true){HBtpJNySeZ = false;}
      if(PWQYlbSIpr == true){PWQYlbSIpr = false;}
      if(qcMxVDESQA == true){qcMxVDESQA = false;}
      if(pWOxOfWnqx == true){pWOxOfWnqx = false;}
      if(qoFbpmfdOk == true){qoFbpmfdOk = false;}
      if(lGhtndowUG == true){lGhtndowUG = false;}
      if(JfCdjsITJF == true){JfCdjsITJF = false;}
      if(EdRrACXjHh == true){EdRrACXjHh = false;}
      if(WZEeNjnTIk == true){WZEeNjnTIk = false;}
      if(qaSjmLmarw == true){qaSjmLmarw = false;}
      if(eMTIgHopsl == true){eMTIgHopsl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HFJQJOHATI
{ 
  void NoyCTSZKkq()
  { 
      bool aWQlUrykDB = false;
      bool ZzoTNfJRAl = false;
      bool DtXaUTjpmQ = false;
      bool FaWMjxXAtc = false;
      bool lrgMERXGrO = false;
      bool HSyJMfzpQB = false;
      bool HIylwybciQ = false;
      bool IJXXzKNKVC = false;
      bool ucxYJlcMAE = false;
      bool yhisUcttQG = false;
      bool ouWaFyuZKB = false;
      bool SnTOGdKZcK = false;
      bool VObeRZZdCV = false;
      bool PStPMzGVSL = false;
      bool pCCylJNUmp = false;
      bool SURGjaSqJf = false;
      bool XQXnkztEBG = false;
      bool XnGQTTMdsH = false;
      bool LEqMDtBmel = false;
      bool fOMmmYeNMl = false;
      string TKxeJCwcwA;
      string bEwUqneGfb;
      string cbErXBRbry;
      string hzwmSPHMBz;
      string lQOzgBNsYr;
      string AMfiPqGEGI;
      string kTkxRcHJBa;
      string fSEgpLNEKW;
      string mXDAyVSNWa;
      string ICGsOSfQhD;
      string sErxLhHzsD;
      string dCtiUMPjyw;
      string KtlZRcLKRW;
      string fwWHdifFnQ;
      string NTUTxGIWXC;
      string CuSkszRReh;
      string VwhVXgflFl;
      string gKwhhqZScL;
      string TomPUkdkOd;
      string QNZGtQkywP;
      if(TKxeJCwcwA == sErxLhHzsD){aWQlUrykDB = true;}
      else if(sErxLhHzsD == TKxeJCwcwA){ouWaFyuZKB = true;}
      if(bEwUqneGfb == dCtiUMPjyw){ZzoTNfJRAl = true;}
      else if(dCtiUMPjyw == bEwUqneGfb){SnTOGdKZcK = true;}
      if(cbErXBRbry == KtlZRcLKRW){DtXaUTjpmQ = true;}
      else if(KtlZRcLKRW == cbErXBRbry){VObeRZZdCV = true;}
      if(hzwmSPHMBz == fwWHdifFnQ){FaWMjxXAtc = true;}
      else if(fwWHdifFnQ == hzwmSPHMBz){PStPMzGVSL = true;}
      if(lQOzgBNsYr == NTUTxGIWXC){lrgMERXGrO = true;}
      else if(NTUTxGIWXC == lQOzgBNsYr){pCCylJNUmp = true;}
      if(AMfiPqGEGI == CuSkszRReh){HSyJMfzpQB = true;}
      else if(CuSkszRReh == AMfiPqGEGI){SURGjaSqJf = true;}
      if(kTkxRcHJBa == VwhVXgflFl){HIylwybciQ = true;}
      else if(VwhVXgflFl == kTkxRcHJBa){XQXnkztEBG = true;}
      if(fSEgpLNEKW == gKwhhqZScL){IJXXzKNKVC = true;}
      if(mXDAyVSNWa == TomPUkdkOd){ucxYJlcMAE = true;}
      if(ICGsOSfQhD == QNZGtQkywP){yhisUcttQG = true;}
      while(gKwhhqZScL == fSEgpLNEKW){XnGQTTMdsH = true;}
      while(TomPUkdkOd == TomPUkdkOd){LEqMDtBmel = true;}
      while(QNZGtQkywP == QNZGtQkywP){fOMmmYeNMl = true;}
      if(aWQlUrykDB == true){aWQlUrykDB = false;}
      if(ZzoTNfJRAl == true){ZzoTNfJRAl = false;}
      if(DtXaUTjpmQ == true){DtXaUTjpmQ = false;}
      if(FaWMjxXAtc == true){FaWMjxXAtc = false;}
      if(lrgMERXGrO == true){lrgMERXGrO = false;}
      if(HSyJMfzpQB == true){HSyJMfzpQB = false;}
      if(HIylwybciQ == true){HIylwybciQ = false;}
      if(IJXXzKNKVC == true){IJXXzKNKVC = false;}
      if(ucxYJlcMAE == true){ucxYJlcMAE = false;}
      if(yhisUcttQG == true){yhisUcttQG = false;}
      if(ouWaFyuZKB == true){ouWaFyuZKB = false;}
      if(SnTOGdKZcK == true){SnTOGdKZcK = false;}
      if(VObeRZZdCV == true){VObeRZZdCV = false;}
      if(PStPMzGVSL == true){PStPMzGVSL = false;}
      if(pCCylJNUmp == true){pCCylJNUmp = false;}
      if(SURGjaSqJf == true){SURGjaSqJf = false;}
      if(XQXnkztEBG == true){XQXnkztEBG = false;}
      if(XnGQTTMdsH == true){XnGQTTMdsH = false;}
      if(LEqMDtBmel == true){LEqMDtBmel = false;}
      if(fOMmmYeNMl == true){fOMmmYeNMl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BSIPWOQZZS
{ 
  void KYXqGubmeX()
  { 
      bool kOHbHyFuhS = false;
      bool UIlCwVULTt = false;
      bool bHAYmSCYUY = false;
      bool CPTPhdDPqo = false;
      bool erJaYwfulc = false;
      bool cbmXEdNhnn = false;
      bool wgZIPEnWTS = false;
      bool RETePiyZAq = false;
      bool mGlqwacwKE = false;
      bool PVaDryRAKU = false;
      bool HIbTKYHIIV = false;
      bool enhxBothMo = false;
      bool brMOCwcQlQ = false;
      bool QyOeideSgX = false;
      bool TUnoZbndBD = false;
      bool EjnVsPwMRO = false;
      bool uqVqEgYCNW = false;
      bool KaBoVHEWoZ = false;
      bool IzOhMgKslT = false;
      bool hRARcbywjr = false;
      string jOzGqZULkr;
      string ZGSStVNBUp;
      string krjwmWKFQi;
      string cgLEJKLxeu;
      string ipUBsYsiKB;
      string htdBardDST;
      string OOuCPsuOJj;
      string KPWYHtWiDO;
      string dbtOAhDJdQ;
      string fIdKfTuSlz;
      string ceZZqSXEtf;
      string UuzEpmUfCg;
      string oljrEBkLsm;
      string zEwEkyBTXg;
      string JzOywjJZTs;
      string zOQFJqthVo;
      string gMLHfPzuSr;
      string lAIUWpWluT;
      string rLrznknqts;
      string ESyizxmGSf;
      if(jOzGqZULkr == ceZZqSXEtf){kOHbHyFuhS = true;}
      else if(ceZZqSXEtf == jOzGqZULkr){HIbTKYHIIV = true;}
      if(ZGSStVNBUp == UuzEpmUfCg){UIlCwVULTt = true;}
      else if(UuzEpmUfCg == ZGSStVNBUp){enhxBothMo = true;}
      if(krjwmWKFQi == oljrEBkLsm){bHAYmSCYUY = true;}
      else if(oljrEBkLsm == krjwmWKFQi){brMOCwcQlQ = true;}
      if(cgLEJKLxeu == zEwEkyBTXg){CPTPhdDPqo = true;}
      else if(zEwEkyBTXg == cgLEJKLxeu){QyOeideSgX = true;}
      if(ipUBsYsiKB == JzOywjJZTs){erJaYwfulc = true;}
      else if(JzOywjJZTs == ipUBsYsiKB){TUnoZbndBD = true;}
      if(htdBardDST == zOQFJqthVo){cbmXEdNhnn = true;}
      else if(zOQFJqthVo == htdBardDST){EjnVsPwMRO = true;}
      if(OOuCPsuOJj == gMLHfPzuSr){wgZIPEnWTS = true;}
      else if(gMLHfPzuSr == OOuCPsuOJj){uqVqEgYCNW = true;}
      if(KPWYHtWiDO == lAIUWpWluT){RETePiyZAq = true;}
      if(dbtOAhDJdQ == rLrznknqts){mGlqwacwKE = true;}
      if(fIdKfTuSlz == ESyizxmGSf){PVaDryRAKU = true;}
      while(lAIUWpWluT == KPWYHtWiDO){KaBoVHEWoZ = true;}
      while(rLrznknqts == rLrznknqts){IzOhMgKslT = true;}
      while(ESyizxmGSf == ESyizxmGSf){hRARcbywjr = true;}
      if(kOHbHyFuhS == true){kOHbHyFuhS = false;}
      if(UIlCwVULTt == true){UIlCwVULTt = false;}
      if(bHAYmSCYUY == true){bHAYmSCYUY = false;}
      if(CPTPhdDPqo == true){CPTPhdDPqo = false;}
      if(erJaYwfulc == true){erJaYwfulc = false;}
      if(cbmXEdNhnn == true){cbmXEdNhnn = false;}
      if(wgZIPEnWTS == true){wgZIPEnWTS = false;}
      if(RETePiyZAq == true){RETePiyZAq = false;}
      if(mGlqwacwKE == true){mGlqwacwKE = false;}
      if(PVaDryRAKU == true){PVaDryRAKU = false;}
      if(HIbTKYHIIV == true){HIbTKYHIIV = false;}
      if(enhxBothMo == true){enhxBothMo = false;}
      if(brMOCwcQlQ == true){brMOCwcQlQ = false;}
      if(QyOeideSgX == true){QyOeideSgX = false;}
      if(TUnoZbndBD == true){TUnoZbndBD = false;}
      if(EjnVsPwMRO == true){EjnVsPwMRO = false;}
      if(uqVqEgYCNW == true){uqVqEgYCNW = false;}
      if(KaBoVHEWoZ == true){KaBoVHEWoZ = false;}
      if(IzOhMgKslT == true){IzOhMgKslT = false;}
      if(hRARcbywjr == true){hRARcbywjr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PDILRYQSON
{ 
  void BobfWaRlLT()
  { 
      bool FMqzZRoPGY = false;
      bool sfyOHTRhCt = false;
      bool EqbQXMxpDx = false;
      bool eeRZcrOiTw = false;
      bool jRqiXeFIsE = false;
      bool muAlbNjZbM = false;
      bool UzLJsOOYlP = false;
      bool FhOFJmzsrr = false;
      bool HowzJjeeyO = false;
      bool InHGbkFhpJ = false;
      bool QfWdzFftbn = false;
      bool OphiEeCMAI = false;
      bool iiaWHRaHcY = false;
      bool gRDibaRRzE = false;
      bool DjDJARGXBw = false;
      bool hsggZgFzWh = false;
      bool dXRCHwmyHd = false;
      bool NjMwEzfQWp = false;
      bool LCGfqmjVIt = false;
      bool URuQZfRACD = false;
      string rldeIbbwPn;
      string KbMihFaXYw;
      string NnRRjRRpNl;
      string KcnpgNhuzZ;
      string PUwsePzfuk;
      string joKAQDUMtq;
      string KlhcchnWCU;
      string WeYzgrrXmT;
      string SlubCVRmYc;
      string rVAHOdlSQx;
      string bbcZNEQLTY;
      string KxYaYYpFaH;
      string PAxYcBeQjC;
      string FYHorpIUOy;
      string EFTzStwZhp;
      string mfrJhadExM;
      string YzbQSlojhi;
      string QVqcCJgyGh;
      string EFhsOYgyHR;
      string LDXBYFdCff;
      if(rldeIbbwPn == bbcZNEQLTY){FMqzZRoPGY = true;}
      else if(bbcZNEQLTY == rldeIbbwPn){QfWdzFftbn = true;}
      if(KbMihFaXYw == KxYaYYpFaH){sfyOHTRhCt = true;}
      else if(KxYaYYpFaH == KbMihFaXYw){OphiEeCMAI = true;}
      if(NnRRjRRpNl == PAxYcBeQjC){EqbQXMxpDx = true;}
      else if(PAxYcBeQjC == NnRRjRRpNl){iiaWHRaHcY = true;}
      if(KcnpgNhuzZ == FYHorpIUOy){eeRZcrOiTw = true;}
      else if(FYHorpIUOy == KcnpgNhuzZ){gRDibaRRzE = true;}
      if(PUwsePzfuk == EFTzStwZhp){jRqiXeFIsE = true;}
      else if(EFTzStwZhp == PUwsePzfuk){DjDJARGXBw = true;}
      if(joKAQDUMtq == mfrJhadExM){muAlbNjZbM = true;}
      else if(mfrJhadExM == joKAQDUMtq){hsggZgFzWh = true;}
      if(KlhcchnWCU == YzbQSlojhi){UzLJsOOYlP = true;}
      else if(YzbQSlojhi == KlhcchnWCU){dXRCHwmyHd = true;}
      if(WeYzgrrXmT == QVqcCJgyGh){FhOFJmzsrr = true;}
      if(SlubCVRmYc == EFhsOYgyHR){HowzJjeeyO = true;}
      if(rVAHOdlSQx == LDXBYFdCff){InHGbkFhpJ = true;}
      while(QVqcCJgyGh == WeYzgrrXmT){NjMwEzfQWp = true;}
      while(EFhsOYgyHR == EFhsOYgyHR){LCGfqmjVIt = true;}
      while(LDXBYFdCff == LDXBYFdCff){URuQZfRACD = true;}
      if(FMqzZRoPGY == true){FMqzZRoPGY = false;}
      if(sfyOHTRhCt == true){sfyOHTRhCt = false;}
      if(EqbQXMxpDx == true){EqbQXMxpDx = false;}
      if(eeRZcrOiTw == true){eeRZcrOiTw = false;}
      if(jRqiXeFIsE == true){jRqiXeFIsE = false;}
      if(muAlbNjZbM == true){muAlbNjZbM = false;}
      if(UzLJsOOYlP == true){UzLJsOOYlP = false;}
      if(FhOFJmzsrr == true){FhOFJmzsrr = false;}
      if(HowzJjeeyO == true){HowzJjeeyO = false;}
      if(InHGbkFhpJ == true){InHGbkFhpJ = false;}
      if(QfWdzFftbn == true){QfWdzFftbn = false;}
      if(OphiEeCMAI == true){OphiEeCMAI = false;}
      if(iiaWHRaHcY == true){iiaWHRaHcY = false;}
      if(gRDibaRRzE == true){gRDibaRRzE = false;}
      if(DjDJARGXBw == true){DjDJARGXBw = false;}
      if(hsggZgFzWh == true){hsggZgFzWh = false;}
      if(dXRCHwmyHd == true){dXRCHwmyHd = false;}
      if(NjMwEzfQWp == true){NjMwEzfQWp = false;}
      if(LCGfqmjVIt == true){LCGfqmjVIt = false;}
      if(URuQZfRACD == true){URuQZfRACD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OFVNEYDQNT
{ 
  void prrnbwumom()
  { 
      bool LfSfubynyp = false;
      bool yKRXEkpVrA = false;
      bool IyqkwzFodN = false;
      bool zqNMrMLzkT = false;
      bool LoEIRYGKzd = false;
      bool lkoROMngiw = false;
      bool WIJkpGxFfq = false;
      bool ZqjXnTtiSX = false;
      bool stozaOCuyf = false;
      bool XdPoxCeTRL = false;
      bool aBeIEtARcu = false;
      bool SFNKDbTKJb = false;
      bool EBGASwbuZk = false;
      bool nZQHwrPgug = false;
      bool NkRDNSPElV = false;
      bool iUJpGZteoI = false;
      bool SFqqGroGOK = false;
      bool LlOcOSiCmA = false;
      bool fWJDCTPZAW = false;
      bool xkrmNIsZxU = false;
      string VOCaKuWHph;
      string VPxgtbLIKu;
      string kAzGidXTyT;
      string hnAudVECzp;
      string SuedwfDAQj;
      string YVYdDUOwrg;
      string XCIVAgPCDX;
      string XJJTcoVUKD;
      string aXDNuLPVrI;
      string GIJRSPmJcO;
      string PAxrMPMEBN;
      string zsloFzztZo;
      string RsJtQPzqBM;
      string CMrumZaKBf;
      string yKUdJMWULT;
      string QrinxWpCbx;
      string PVDnLlBpCf;
      string XnstpGXfjD;
      string sdegwKdDyQ;
      string iYiEPcHgSW;
      if(VOCaKuWHph == PAxrMPMEBN){LfSfubynyp = true;}
      else if(PAxrMPMEBN == VOCaKuWHph){aBeIEtARcu = true;}
      if(VPxgtbLIKu == zsloFzztZo){yKRXEkpVrA = true;}
      else if(zsloFzztZo == VPxgtbLIKu){SFNKDbTKJb = true;}
      if(kAzGidXTyT == RsJtQPzqBM){IyqkwzFodN = true;}
      else if(RsJtQPzqBM == kAzGidXTyT){EBGASwbuZk = true;}
      if(hnAudVECzp == CMrumZaKBf){zqNMrMLzkT = true;}
      else if(CMrumZaKBf == hnAudVECzp){nZQHwrPgug = true;}
      if(SuedwfDAQj == yKUdJMWULT){LoEIRYGKzd = true;}
      else if(yKUdJMWULT == SuedwfDAQj){NkRDNSPElV = true;}
      if(YVYdDUOwrg == QrinxWpCbx){lkoROMngiw = true;}
      else if(QrinxWpCbx == YVYdDUOwrg){iUJpGZteoI = true;}
      if(XCIVAgPCDX == PVDnLlBpCf){WIJkpGxFfq = true;}
      else if(PVDnLlBpCf == XCIVAgPCDX){SFqqGroGOK = true;}
      if(XJJTcoVUKD == XnstpGXfjD){ZqjXnTtiSX = true;}
      if(aXDNuLPVrI == sdegwKdDyQ){stozaOCuyf = true;}
      if(GIJRSPmJcO == iYiEPcHgSW){XdPoxCeTRL = true;}
      while(XnstpGXfjD == XJJTcoVUKD){LlOcOSiCmA = true;}
      while(sdegwKdDyQ == sdegwKdDyQ){fWJDCTPZAW = true;}
      while(iYiEPcHgSW == iYiEPcHgSW){xkrmNIsZxU = true;}
      if(LfSfubynyp == true){LfSfubynyp = false;}
      if(yKRXEkpVrA == true){yKRXEkpVrA = false;}
      if(IyqkwzFodN == true){IyqkwzFodN = false;}
      if(zqNMrMLzkT == true){zqNMrMLzkT = false;}
      if(LoEIRYGKzd == true){LoEIRYGKzd = false;}
      if(lkoROMngiw == true){lkoROMngiw = false;}
      if(WIJkpGxFfq == true){WIJkpGxFfq = false;}
      if(ZqjXnTtiSX == true){ZqjXnTtiSX = false;}
      if(stozaOCuyf == true){stozaOCuyf = false;}
      if(XdPoxCeTRL == true){XdPoxCeTRL = false;}
      if(aBeIEtARcu == true){aBeIEtARcu = false;}
      if(SFNKDbTKJb == true){SFNKDbTKJb = false;}
      if(EBGASwbuZk == true){EBGASwbuZk = false;}
      if(nZQHwrPgug == true){nZQHwrPgug = false;}
      if(NkRDNSPElV == true){NkRDNSPElV = false;}
      if(iUJpGZteoI == true){iUJpGZteoI = false;}
      if(SFqqGroGOK == true){SFqqGroGOK = false;}
      if(LlOcOSiCmA == true){LlOcOSiCmA = false;}
      if(fWJDCTPZAW == true){fWJDCTPZAW = false;}
      if(xkrmNIsZxU == true){xkrmNIsZxU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IQICUHFNNW
{ 
  void oNjDOfRetB()
  { 
      bool kzZtJhRuNK = false;
      bool aegrQYOiNM = false;
      bool DcaumSiAcV = false;
      bool cZWBOnwhSS = false;
      bool qwbccesCPL = false;
      bool rBZNmHXeGq = false;
      bool zQRqgBIfcc = false;
      bool cfQtVHBxRH = false;
      bool YfkRdazIOH = false;
      bool PQbbTZdjub = false;
      bool WMYmiLkeqe = false;
      bool FaMyRSeANF = false;
      bool BzRVMLEoaw = false;
      bool cRCpkLGyhR = false;
      bool pURiBoEcJB = false;
      bool udWoZWuDZW = false;
      bool GwEuBnYWMF = false;
      bool LfRuPghhTW = false;
      bool HaeZAztMPN = false;
      bool QDXLKtQVhN = false;
      string mkluBWltsz;
      string tGuGApUeSZ;
      string CgqUECYkiZ;
      string SAGbquDHes;
      string fcYwbFqUBr;
      string CKOXCGsRqy;
      string yBKSgtzTPF;
      string lMAZsZIury;
      string LEiqWHkhYk;
      string BlQfueSFPg;
      string FWEwFpKoec;
      string yyYrmsRSuY;
      string JQteDAuUdO;
      string HCeKsnAmlg;
      string wuDwlDMYrC;
      string MzKpXncZrE;
      string upKPVuSNzy;
      string sdgOcNNxrz;
      string mVbbBIqomE;
      string KaugkGknaq;
      if(mkluBWltsz == FWEwFpKoec){kzZtJhRuNK = true;}
      else if(FWEwFpKoec == mkluBWltsz){WMYmiLkeqe = true;}
      if(tGuGApUeSZ == yyYrmsRSuY){aegrQYOiNM = true;}
      else if(yyYrmsRSuY == tGuGApUeSZ){FaMyRSeANF = true;}
      if(CgqUECYkiZ == JQteDAuUdO){DcaumSiAcV = true;}
      else if(JQteDAuUdO == CgqUECYkiZ){BzRVMLEoaw = true;}
      if(SAGbquDHes == HCeKsnAmlg){cZWBOnwhSS = true;}
      else if(HCeKsnAmlg == SAGbquDHes){cRCpkLGyhR = true;}
      if(fcYwbFqUBr == wuDwlDMYrC){qwbccesCPL = true;}
      else if(wuDwlDMYrC == fcYwbFqUBr){pURiBoEcJB = true;}
      if(CKOXCGsRqy == MzKpXncZrE){rBZNmHXeGq = true;}
      else if(MzKpXncZrE == CKOXCGsRqy){udWoZWuDZW = true;}
      if(yBKSgtzTPF == upKPVuSNzy){zQRqgBIfcc = true;}
      else if(upKPVuSNzy == yBKSgtzTPF){GwEuBnYWMF = true;}
      if(lMAZsZIury == sdgOcNNxrz){cfQtVHBxRH = true;}
      if(LEiqWHkhYk == mVbbBIqomE){YfkRdazIOH = true;}
      if(BlQfueSFPg == KaugkGknaq){PQbbTZdjub = true;}
      while(sdgOcNNxrz == lMAZsZIury){LfRuPghhTW = true;}
      while(mVbbBIqomE == mVbbBIqomE){HaeZAztMPN = true;}
      while(KaugkGknaq == KaugkGknaq){QDXLKtQVhN = true;}
      if(kzZtJhRuNK == true){kzZtJhRuNK = false;}
      if(aegrQYOiNM == true){aegrQYOiNM = false;}
      if(DcaumSiAcV == true){DcaumSiAcV = false;}
      if(cZWBOnwhSS == true){cZWBOnwhSS = false;}
      if(qwbccesCPL == true){qwbccesCPL = false;}
      if(rBZNmHXeGq == true){rBZNmHXeGq = false;}
      if(zQRqgBIfcc == true){zQRqgBIfcc = false;}
      if(cfQtVHBxRH == true){cfQtVHBxRH = false;}
      if(YfkRdazIOH == true){YfkRdazIOH = false;}
      if(PQbbTZdjub == true){PQbbTZdjub = false;}
      if(WMYmiLkeqe == true){WMYmiLkeqe = false;}
      if(FaMyRSeANF == true){FaMyRSeANF = false;}
      if(BzRVMLEoaw == true){BzRVMLEoaw = false;}
      if(cRCpkLGyhR == true){cRCpkLGyhR = false;}
      if(pURiBoEcJB == true){pURiBoEcJB = false;}
      if(udWoZWuDZW == true){udWoZWuDZW = false;}
      if(GwEuBnYWMF == true){GwEuBnYWMF = false;}
      if(LfRuPghhTW == true){LfRuPghhTW = false;}
      if(HaeZAztMPN == true){HaeZAztMPN = false;}
      if(QDXLKtQVhN == true){QDXLKtQVhN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XCKDSYTTXY
{ 
  void SksbIYFLoD()
  { 
      bool smVUzpUXZK = false;
      bool nXtKwyGLFZ = false;
      bool DSySFMPOIc = false;
      bool axumlixMgd = false;
      bool EtklsWCWMK = false;
      bool ukLHEZoDPm = false;
      bool aFOxjpbHXN = false;
      bool AsdaVJgLYh = false;
      bool KsCygolmhL = false;
      bool tsxtDzfnCD = false;
      bool ttyyJCGjlu = false;
      bool scegbgDQPt = false;
      bool EoYLTHwjXV = false;
      bool crIUmPuDne = false;
      bool iBBmSQNPSW = false;
      bool wOXHzFVmeC = false;
      bool FWMPRAUJtM = false;
      bool lzbkLCmksD = false;
      bool sgQBZOUBVg = false;
      bool sHXlYLolVR = false;
      string ZCUNhHLfPm;
      string pGkUagjmAF;
      string xBPOieHdio;
      string nDYZiFRlUE;
      string onKYkpWDOl;
      string GIdyaXlGeD;
      string AhwazmgKlV;
      string PySuPJqpds;
      string HDOxieYazL;
      string SKHQEGqiZx;
      string ILQrVHoiiR;
      string fubkwwTzrU;
      string NGKaCEpCpI;
      string RIQgjulWaa;
      string opqheWhWUI;
      string mrpFiCApoR;
      string RmsZzWduAA;
      string JwXReXOcue;
      string VTggUYSLxh;
      string twgIIBxQhI;
      if(ZCUNhHLfPm == ILQrVHoiiR){smVUzpUXZK = true;}
      else if(ILQrVHoiiR == ZCUNhHLfPm){ttyyJCGjlu = true;}
      if(pGkUagjmAF == fubkwwTzrU){nXtKwyGLFZ = true;}
      else if(fubkwwTzrU == pGkUagjmAF){scegbgDQPt = true;}
      if(xBPOieHdio == NGKaCEpCpI){DSySFMPOIc = true;}
      else if(NGKaCEpCpI == xBPOieHdio){EoYLTHwjXV = true;}
      if(nDYZiFRlUE == RIQgjulWaa){axumlixMgd = true;}
      else if(RIQgjulWaa == nDYZiFRlUE){crIUmPuDne = true;}
      if(onKYkpWDOl == opqheWhWUI){EtklsWCWMK = true;}
      else if(opqheWhWUI == onKYkpWDOl){iBBmSQNPSW = true;}
      if(GIdyaXlGeD == mrpFiCApoR){ukLHEZoDPm = true;}
      else if(mrpFiCApoR == GIdyaXlGeD){wOXHzFVmeC = true;}
      if(AhwazmgKlV == RmsZzWduAA){aFOxjpbHXN = true;}
      else if(RmsZzWduAA == AhwazmgKlV){FWMPRAUJtM = true;}
      if(PySuPJqpds == JwXReXOcue){AsdaVJgLYh = true;}
      if(HDOxieYazL == VTggUYSLxh){KsCygolmhL = true;}
      if(SKHQEGqiZx == twgIIBxQhI){tsxtDzfnCD = true;}
      while(JwXReXOcue == PySuPJqpds){lzbkLCmksD = true;}
      while(VTggUYSLxh == VTggUYSLxh){sgQBZOUBVg = true;}
      while(twgIIBxQhI == twgIIBxQhI){sHXlYLolVR = true;}
      if(smVUzpUXZK == true){smVUzpUXZK = false;}
      if(nXtKwyGLFZ == true){nXtKwyGLFZ = false;}
      if(DSySFMPOIc == true){DSySFMPOIc = false;}
      if(axumlixMgd == true){axumlixMgd = false;}
      if(EtklsWCWMK == true){EtklsWCWMK = false;}
      if(ukLHEZoDPm == true){ukLHEZoDPm = false;}
      if(aFOxjpbHXN == true){aFOxjpbHXN = false;}
      if(AsdaVJgLYh == true){AsdaVJgLYh = false;}
      if(KsCygolmhL == true){KsCygolmhL = false;}
      if(tsxtDzfnCD == true){tsxtDzfnCD = false;}
      if(ttyyJCGjlu == true){ttyyJCGjlu = false;}
      if(scegbgDQPt == true){scegbgDQPt = false;}
      if(EoYLTHwjXV == true){EoYLTHwjXV = false;}
      if(crIUmPuDne == true){crIUmPuDne = false;}
      if(iBBmSQNPSW == true){iBBmSQNPSW = false;}
      if(wOXHzFVmeC == true){wOXHzFVmeC = false;}
      if(FWMPRAUJtM == true){FWMPRAUJtM = false;}
      if(lzbkLCmksD == true){lzbkLCmksD = false;}
      if(sgQBZOUBVg == true){sgQBZOUBVg = false;}
      if(sHXlYLolVR == true){sHXlYLolVR = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TITSOBVSWQ
{ 
  void mqpZnKzNjZ()
  { 
      bool WYYcyiTPtZ = false;
      bool eKykrCqQoF = false;
      bool iZBzpEhLEs = false;
      bool ssQXjfNuPR = false;
      bool LmJPNqVJTd = false;
      bool MCHcRwWZls = false;
      bool AIKBfktZAL = false;
      bool nwBXtwQkkz = false;
      bool hYqZfplmFm = false;
      bool InIVQCbIic = false;
      bool wwIIpPAjiV = false;
      bool flolLbRjUp = false;
      bool yGTMCAmfGr = false;
      bool THCHClPijP = false;
      bool oBEjHWzVKo = false;
      bool kqnsdobuFo = false;
      bool cUdpBXigjY = false;
      bool LizEAxCjFf = false;
      bool BRVQGbTIcN = false;
      bool KTcumUXNhj = false;
      string keqFNNZDxp;
      string qDhYpZSkWR;
      string gjBAjTNXte;
      string zNhjeFXrTr;
      string ciZyaUcfkS;
      string RYxqGAdqVt;
      string ABJSpcUysk;
      string BZRenxKDYO;
      string pglaMZQpde;
      string bgWYraZGHx;
      string AemARrHRDI;
      string pZRGDTOhFP;
      string LdTOITDrmM;
      string XDuaOLslJU;
      string SzmUnBOQUY;
      string LrAqlVKYqx;
      string nOepkleprr;
      string ZcXkxMqzZG;
      string ElXNAeCEos;
      string BUtMfDTxiB;
      if(keqFNNZDxp == AemARrHRDI){WYYcyiTPtZ = true;}
      else if(AemARrHRDI == keqFNNZDxp){wwIIpPAjiV = true;}
      if(qDhYpZSkWR == pZRGDTOhFP){eKykrCqQoF = true;}
      else if(pZRGDTOhFP == qDhYpZSkWR){flolLbRjUp = true;}
      if(gjBAjTNXte == LdTOITDrmM){iZBzpEhLEs = true;}
      else if(LdTOITDrmM == gjBAjTNXte){yGTMCAmfGr = true;}
      if(zNhjeFXrTr == XDuaOLslJU){ssQXjfNuPR = true;}
      else if(XDuaOLslJU == zNhjeFXrTr){THCHClPijP = true;}
      if(ciZyaUcfkS == SzmUnBOQUY){LmJPNqVJTd = true;}
      else if(SzmUnBOQUY == ciZyaUcfkS){oBEjHWzVKo = true;}
      if(RYxqGAdqVt == LrAqlVKYqx){MCHcRwWZls = true;}
      else if(LrAqlVKYqx == RYxqGAdqVt){kqnsdobuFo = true;}
      if(ABJSpcUysk == nOepkleprr){AIKBfktZAL = true;}
      else if(nOepkleprr == ABJSpcUysk){cUdpBXigjY = true;}
      if(BZRenxKDYO == ZcXkxMqzZG){nwBXtwQkkz = true;}
      if(pglaMZQpde == ElXNAeCEos){hYqZfplmFm = true;}
      if(bgWYraZGHx == BUtMfDTxiB){InIVQCbIic = true;}
      while(ZcXkxMqzZG == BZRenxKDYO){LizEAxCjFf = true;}
      while(ElXNAeCEos == ElXNAeCEos){BRVQGbTIcN = true;}
      while(BUtMfDTxiB == BUtMfDTxiB){KTcumUXNhj = true;}
      if(WYYcyiTPtZ == true){WYYcyiTPtZ = false;}
      if(eKykrCqQoF == true){eKykrCqQoF = false;}
      if(iZBzpEhLEs == true){iZBzpEhLEs = false;}
      if(ssQXjfNuPR == true){ssQXjfNuPR = false;}
      if(LmJPNqVJTd == true){LmJPNqVJTd = false;}
      if(MCHcRwWZls == true){MCHcRwWZls = false;}
      if(AIKBfktZAL == true){AIKBfktZAL = false;}
      if(nwBXtwQkkz == true){nwBXtwQkkz = false;}
      if(hYqZfplmFm == true){hYqZfplmFm = false;}
      if(InIVQCbIic == true){InIVQCbIic = false;}
      if(wwIIpPAjiV == true){wwIIpPAjiV = false;}
      if(flolLbRjUp == true){flolLbRjUp = false;}
      if(yGTMCAmfGr == true){yGTMCAmfGr = false;}
      if(THCHClPijP == true){THCHClPijP = false;}
      if(oBEjHWzVKo == true){oBEjHWzVKo = false;}
      if(kqnsdobuFo == true){kqnsdobuFo = false;}
      if(cUdpBXigjY == true){cUdpBXigjY = false;}
      if(LizEAxCjFf == true){LizEAxCjFf = false;}
      if(BRVQGbTIcN == true){BRVQGbTIcN = false;}
      if(KTcumUXNhj == true){KTcumUXNhj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UMPJYCQVCD
{ 
  void NyhPhqEbhQ()
  { 
      bool IEhRYuUPdu = false;
      bool jVWWemrIVN = false;
      bool LhJFCToVka = false;
      bool CmIGVSYzJQ = false;
      bool LIAcUlMsuc = false;
      bool RnWbJaucOF = false;
      bool FzdDrxNQAL = false;
      bool OszFCDgfkP = false;
      bool GwZIIOPfwu = false;
      bool XwjEKMdxkW = false;
      bool YuriOYnRZG = false;
      bool sOryyJYUuc = false;
      bool ghTFnRJtuo = false;
      bool YjuNqKEpti = false;
      bool ZyFpBIEkFA = false;
      bool mIBHopbluw = false;
      bool PsVjgITeGm = false;
      bool EhaLCicdDf = false;
      bool NXotCPhmXB = false;
      bool iRuxTfbUJr = false;
      string IiTdSGbZsS;
      string wlQiYoZFis;
      string zQNQBuHpGZ;
      string oQFGWypAqL;
      string IKxyOGNrZN;
      string TRYgfrVCbX;
      string zkTScegJLa;
      string yYNWknSPCu;
      string hOPsGXqXad;
      string bmYTMWqdUi;
      string ZDfzOuDCwq;
      string YGAfpkVaMF;
      string nIigyKAtKN;
      string xotSVdhQdK;
      string TnbjFUmrdb;
      string BQQPtQUyAD;
      string tJrhtjMhKL;
      string CFwBkzfybd;
      string XHQiMfXeXE;
      string IiPaPKwspV;
      if(IiTdSGbZsS == ZDfzOuDCwq){IEhRYuUPdu = true;}
      else if(ZDfzOuDCwq == IiTdSGbZsS){YuriOYnRZG = true;}
      if(wlQiYoZFis == YGAfpkVaMF){jVWWemrIVN = true;}
      else if(YGAfpkVaMF == wlQiYoZFis){sOryyJYUuc = true;}
      if(zQNQBuHpGZ == nIigyKAtKN){LhJFCToVka = true;}
      else if(nIigyKAtKN == zQNQBuHpGZ){ghTFnRJtuo = true;}
      if(oQFGWypAqL == xotSVdhQdK){CmIGVSYzJQ = true;}
      else if(xotSVdhQdK == oQFGWypAqL){YjuNqKEpti = true;}
      if(IKxyOGNrZN == TnbjFUmrdb){LIAcUlMsuc = true;}
      else if(TnbjFUmrdb == IKxyOGNrZN){ZyFpBIEkFA = true;}
      if(TRYgfrVCbX == BQQPtQUyAD){RnWbJaucOF = true;}
      else if(BQQPtQUyAD == TRYgfrVCbX){mIBHopbluw = true;}
      if(zkTScegJLa == tJrhtjMhKL){FzdDrxNQAL = true;}
      else if(tJrhtjMhKL == zkTScegJLa){PsVjgITeGm = true;}
      if(yYNWknSPCu == CFwBkzfybd){OszFCDgfkP = true;}
      if(hOPsGXqXad == XHQiMfXeXE){GwZIIOPfwu = true;}
      if(bmYTMWqdUi == IiPaPKwspV){XwjEKMdxkW = true;}
      while(CFwBkzfybd == yYNWknSPCu){EhaLCicdDf = true;}
      while(XHQiMfXeXE == XHQiMfXeXE){NXotCPhmXB = true;}
      while(IiPaPKwspV == IiPaPKwspV){iRuxTfbUJr = true;}
      if(IEhRYuUPdu == true){IEhRYuUPdu = false;}
      if(jVWWemrIVN == true){jVWWemrIVN = false;}
      if(LhJFCToVka == true){LhJFCToVka = false;}
      if(CmIGVSYzJQ == true){CmIGVSYzJQ = false;}
      if(LIAcUlMsuc == true){LIAcUlMsuc = false;}
      if(RnWbJaucOF == true){RnWbJaucOF = false;}
      if(FzdDrxNQAL == true){FzdDrxNQAL = false;}
      if(OszFCDgfkP == true){OszFCDgfkP = false;}
      if(GwZIIOPfwu == true){GwZIIOPfwu = false;}
      if(XwjEKMdxkW == true){XwjEKMdxkW = false;}
      if(YuriOYnRZG == true){YuriOYnRZG = false;}
      if(sOryyJYUuc == true){sOryyJYUuc = false;}
      if(ghTFnRJtuo == true){ghTFnRJtuo = false;}
      if(YjuNqKEpti == true){YjuNqKEpti = false;}
      if(ZyFpBIEkFA == true){ZyFpBIEkFA = false;}
      if(mIBHopbluw == true){mIBHopbluw = false;}
      if(PsVjgITeGm == true){PsVjgITeGm = false;}
      if(EhaLCicdDf == true){EhaLCicdDf = false;}
      if(NXotCPhmXB == true){NXotCPhmXB = false;}
      if(iRuxTfbUJr == true){iRuxTfbUJr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOGNYUGYHE
{ 
  void DbSBAfluUJ()
  { 
      bool nNWYrnKxmM = false;
      bool WGPcXkEDDe = false;
      bool RClDGcONIA = false;
      bool LLFbZnbbsH = false;
      bool fadfPVLVsh = false;
      bool LbOujkbTRP = false;
      bool YxomAKAnDj = false;
      bool IbJuzpYFmk = false;
      bool QsTanjxKrn = false;
      bool LIKdiilpfF = false;
      bool VWSJYjSSus = false;
      bool LWffyfcNzQ = false;
      bool MpwOYAQfMQ = false;
      bool EhZhPtZKBE = false;
      bool RdOycobdeC = false;
      bool GYqOreZEkY = false;
      bool SlFeocwyQE = false;
      bool FRLuzhbxki = false;
      bool XmAKKeqKoj = false;
      bool TVZckeYiJz = false;
      string RQnIyVklVf;
      string mdebJjEutO;
      string xcSstWIeAt;
      string KdRBWsRDgl;
      string KAcJbJrVcm;
      string wbhBAnxuNb;
      string NFkaUJMcEQ;
      string SBNIDxzJQk;
      string dlBMxiuPAh;
      string uWoFYFOCoS;
      string xPQMUuNZxD;
      string ondcxaDXck;
      string NwcWUuLIjy;
      string hxTymOVorC;
      string UVRwsHsMHx;
      string BqYDEAlsqZ;
      string jFpzVqTeoi;
      string ENfaPrdfdN;
      string BzAlwqYQsl;
      string bbyRSPfRaB;
      if(RQnIyVklVf == xPQMUuNZxD){nNWYrnKxmM = true;}
      else if(xPQMUuNZxD == RQnIyVklVf){VWSJYjSSus = true;}
      if(mdebJjEutO == ondcxaDXck){WGPcXkEDDe = true;}
      else if(ondcxaDXck == mdebJjEutO){LWffyfcNzQ = true;}
      if(xcSstWIeAt == NwcWUuLIjy){RClDGcONIA = true;}
      else if(NwcWUuLIjy == xcSstWIeAt){MpwOYAQfMQ = true;}
      if(KdRBWsRDgl == hxTymOVorC){LLFbZnbbsH = true;}
      else if(hxTymOVorC == KdRBWsRDgl){EhZhPtZKBE = true;}
      if(KAcJbJrVcm == UVRwsHsMHx){fadfPVLVsh = true;}
      else if(UVRwsHsMHx == KAcJbJrVcm){RdOycobdeC = true;}
      if(wbhBAnxuNb == BqYDEAlsqZ){LbOujkbTRP = true;}
      else if(BqYDEAlsqZ == wbhBAnxuNb){GYqOreZEkY = true;}
      if(NFkaUJMcEQ == jFpzVqTeoi){YxomAKAnDj = true;}
      else if(jFpzVqTeoi == NFkaUJMcEQ){SlFeocwyQE = true;}
      if(SBNIDxzJQk == ENfaPrdfdN){IbJuzpYFmk = true;}
      if(dlBMxiuPAh == BzAlwqYQsl){QsTanjxKrn = true;}
      if(uWoFYFOCoS == bbyRSPfRaB){LIKdiilpfF = true;}
      while(ENfaPrdfdN == SBNIDxzJQk){FRLuzhbxki = true;}
      while(BzAlwqYQsl == BzAlwqYQsl){XmAKKeqKoj = true;}
      while(bbyRSPfRaB == bbyRSPfRaB){TVZckeYiJz = true;}
      if(nNWYrnKxmM == true){nNWYrnKxmM = false;}
      if(WGPcXkEDDe == true){WGPcXkEDDe = false;}
      if(RClDGcONIA == true){RClDGcONIA = false;}
      if(LLFbZnbbsH == true){LLFbZnbbsH = false;}
      if(fadfPVLVsh == true){fadfPVLVsh = false;}
      if(LbOujkbTRP == true){LbOujkbTRP = false;}
      if(YxomAKAnDj == true){YxomAKAnDj = false;}
      if(IbJuzpYFmk == true){IbJuzpYFmk = false;}
      if(QsTanjxKrn == true){QsTanjxKrn = false;}
      if(LIKdiilpfF == true){LIKdiilpfF = false;}
      if(VWSJYjSSus == true){VWSJYjSSus = false;}
      if(LWffyfcNzQ == true){LWffyfcNzQ = false;}
      if(MpwOYAQfMQ == true){MpwOYAQfMQ = false;}
      if(EhZhPtZKBE == true){EhZhPtZKBE = false;}
      if(RdOycobdeC == true){RdOycobdeC = false;}
      if(GYqOreZEkY == true){GYqOreZEkY = false;}
      if(SlFeocwyQE == true){SlFeocwyQE = false;}
      if(FRLuzhbxki == true){FRLuzhbxki = false;}
      if(XmAKKeqKoj == true){XmAKKeqKoj = false;}
      if(TVZckeYiJz == true){TVZckeYiJz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HCIKEZQGTC
{ 
  void GGSBcUKXUt()
  { 
      bool ozYiCkMxxu = false;
      bool VTjHTrHbLN = false;
      bool mcbLPQrwTm = false;
      bool xwhmmcpfHw = false;
      bool HXHsBWgnyE = false;
      bool oGsdQMXNTk = false;
      bool sPcJxYkwTp = false;
      bool BaUOFOUhKr = false;
      bool jDyPyqCEWa = false;
      bool FcMVDqtyub = false;
      bool ljUHCNleXB = false;
      bool qoDaAoRNKG = false;
      bool WfZskYOyKN = false;
      bool TWmiGAiqNk = false;
      bool FWgUjQYkUO = false;
      bool ZgmoEFKbVc = false;
      bool ymTJgTBUTA = false;
      bool FazZBAzEIU = false;
      bool AccwnmjydB = false;
      bool oHUjlRCWmP = false;
      string VptgtCWUYB;
      string rDfWVKekNa;
      string mJBVoYGolS;
      string FoCdPeUZAH;
      string anofdXujZf;
      string jLmBbMoXqU;
      string whiFwCBBGp;
      string pEjGAzTVqQ;
      string dVOFitBACD;
      string pwhfKrEjjb;
      string WsdipeagBc;
      string rTfapZSdVC;
      string AAzCzsWEuH;
      string tjkPsjrePE;
      string SUzYLWPGok;
      string YaOgaWtajx;
      string HWlshIhQhG;
      string hnOQjfBZqc;
      string cPOJhTVEnW;
      string pQuRwwhFqD;
      if(VptgtCWUYB == WsdipeagBc){ozYiCkMxxu = true;}
      else if(WsdipeagBc == VptgtCWUYB){ljUHCNleXB = true;}
      if(rDfWVKekNa == rTfapZSdVC){VTjHTrHbLN = true;}
      else if(rTfapZSdVC == rDfWVKekNa){qoDaAoRNKG = true;}
      if(mJBVoYGolS == AAzCzsWEuH){mcbLPQrwTm = true;}
      else if(AAzCzsWEuH == mJBVoYGolS){WfZskYOyKN = true;}
      if(FoCdPeUZAH == tjkPsjrePE){xwhmmcpfHw = true;}
      else if(tjkPsjrePE == FoCdPeUZAH){TWmiGAiqNk = true;}
      if(anofdXujZf == SUzYLWPGok){HXHsBWgnyE = true;}
      else if(SUzYLWPGok == anofdXujZf){FWgUjQYkUO = true;}
      if(jLmBbMoXqU == YaOgaWtajx){oGsdQMXNTk = true;}
      else if(YaOgaWtajx == jLmBbMoXqU){ZgmoEFKbVc = true;}
      if(whiFwCBBGp == HWlshIhQhG){sPcJxYkwTp = true;}
      else if(HWlshIhQhG == whiFwCBBGp){ymTJgTBUTA = true;}
      if(pEjGAzTVqQ == hnOQjfBZqc){BaUOFOUhKr = true;}
      if(dVOFitBACD == cPOJhTVEnW){jDyPyqCEWa = true;}
      if(pwhfKrEjjb == pQuRwwhFqD){FcMVDqtyub = true;}
      while(hnOQjfBZqc == pEjGAzTVqQ){FazZBAzEIU = true;}
      while(cPOJhTVEnW == cPOJhTVEnW){AccwnmjydB = true;}
      while(pQuRwwhFqD == pQuRwwhFqD){oHUjlRCWmP = true;}
      if(ozYiCkMxxu == true){ozYiCkMxxu = false;}
      if(VTjHTrHbLN == true){VTjHTrHbLN = false;}
      if(mcbLPQrwTm == true){mcbLPQrwTm = false;}
      if(xwhmmcpfHw == true){xwhmmcpfHw = false;}
      if(HXHsBWgnyE == true){HXHsBWgnyE = false;}
      if(oGsdQMXNTk == true){oGsdQMXNTk = false;}
      if(sPcJxYkwTp == true){sPcJxYkwTp = false;}
      if(BaUOFOUhKr == true){BaUOFOUhKr = false;}
      if(jDyPyqCEWa == true){jDyPyqCEWa = false;}
      if(FcMVDqtyub == true){FcMVDqtyub = false;}
      if(ljUHCNleXB == true){ljUHCNleXB = false;}
      if(qoDaAoRNKG == true){qoDaAoRNKG = false;}
      if(WfZskYOyKN == true){WfZskYOyKN = false;}
      if(TWmiGAiqNk == true){TWmiGAiqNk = false;}
      if(FWgUjQYkUO == true){FWgUjQYkUO = false;}
      if(ZgmoEFKbVc == true){ZgmoEFKbVc = false;}
      if(ymTJgTBUTA == true){ymTJgTBUTA = false;}
      if(FazZBAzEIU == true){FazZBAzEIU = false;}
      if(AccwnmjydB == true){AccwnmjydB = false;}
      if(oHUjlRCWmP == true){oHUjlRCWmP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UUHLFHIKFL
{ 
  void liFRODecGE()
  { 
      bool QMuZTGDxTj = false;
      bool fNuxaUULxl = false;
      bool pQQHcABRxT = false;
      bool djQrfsjyxD = false;
      bool NsySYegzTn = false;
      bool PjPfEAunxS = false;
      bool FkAzGwwCRh = false;
      bool jVOsXwRyhx = false;
      bool TCyJEsUjne = false;
      bool xuAdycszLc = false;
      bool idhkDsUWIH = false;
      bool hxGAyDmptM = false;
      bool bLNljoUktu = false;
      bool DSlEOoTMFK = false;
      bool YxbgwzoAfu = false;
      bool oCEyBWPuBO = false;
      bool XjbMEPLBkI = false;
      bool iakjndjgoj = false;
      bool zUEzFANmDr = false;
      bool YXKqmnVPbf = false;
      string iBrKtWTCqE;
      string NimNWNWcdX;
      string fsdpjwaxPe;
      string dBYdsyOeDf;
      string PrfgUFIRPS;
      string gmmfXoXCZx;
      string wVlMZMJJTX;
      string zrZxImNxVd;
      string nIhwxqOrFE;
      string aMEFjsFSGI;
      string uzyqTnNoXj;
      string bZguJmnoPh;
      string yRDPxlnTMP;
      string gtqMinKpcJ;
      string dYUUbAfIuw;
      string EPDXJcuhth;
      string AscHDgiTMU;
      string cxMGEZxhmx;
      string GfYQKQqSCF;
      string rbwuIjwMRt;
      if(iBrKtWTCqE == uzyqTnNoXj){QMuZTGDxTj = true;}
      else if(uzyqTnNoXj == iBrKtWTCqE){idhkDsUWIH = true;}
      if(NimNWNWcdX == bZguJmnoPh){fNuxaUULxl = true;}
      else if(bZguJmnoPh == NimNWNWcdX){hxGAyDmptM = true;}
      if(fsdpjwaxPe == yRDPxlnTMP){pQQHcABRxT = true;}
      else if(yRDPxlnTMP == fsdpjwaxPe){bLNljoUktu = true;}
      if(dBYdsyOeDf == gtqMinKpcJ){djQrfsjyxD = true;}
      else if(gtqMinKpcJ == dBYdsyOeDf){DSlEOoTMFK = true;}
      if(PrfgUFIRPS == dYUUbAfIuw){NsySYegzTn = true;}
      else if(dYUUbAfIuw == PrfgUFIRPS){YxbgwzoAfu = true;}
      if(gmmfXoXCZx == EPDXJcuhth){PjPfEAunxS = true;}
      else if(EPDXJcuhth == gmmfXoXCZx){oCEyBWPuBO = true;}
      if(wVlMZMJJTX == AscHDgiTMU){FkAzGwwCRh = true;}
      else if(AscHDgiTMU == wVlMZMJJTX){XjbMEPLBkI = true;}
      if(zrZxImNxVd == cxMGEZxhmx){jVOsXwRyhx = true;}
      if(nIhwxqOrFE == GfYQKQqSCF){TCyJEsUjne = true;}
      if(aMEFjsFSGI == rbwuIjwMRt){xuAdycszLc = true;}
      while(cxMGEZxhmx == zrZxImNxVd){iakjndjgoj = true;}
      while(GfYQKQqSCF == GfYQKQqSCF){zUEzFANmDr = true;}
      while(rbwuIjwMRt == rbwuIjwMRt){YXKqmnVPbf = true;}
      if(QMuZTGDxTj == true){QMuZTGDxTj = false;}
      if(fNuxaUULxl == true){fNuxaUULxl = false;}
      if(pQQHcABRxT == true){pQQHcABRxT = false;}
      if(djQrfsjyxD == true){djQrfsjyxD = false;}
      if(NsySYegzTn == true){NsySYegzTn = false;}
      if(PjPfEAunxS == true){PjPfEAunxS = false;}
      if(FkAzGwwCRh == true){FkAzGwwCRh = false;}
      if(jVOsXwRyhx == true){jVOsXwRyhx = false;}
      if(TCyJEsUjne == true){TCyJEsUjne = false;}
      if(xuAdycszLc == true){xuAdycszLc = false;}
      if(idhkDsUWIH == true){idhkDsUWIH = false;}
      if(hxGAyDmptM == true){hxGAyDmptM = false;}
      if(bLNljoUktu == true){bLNljoUktu = false;}
      if(DSlEOoTMFK == true){DSlEOoTMFK = false;}
      if(YxbgwzoAfu == true){YxbgwzoAfu = false;}
      if(oCEyBWPuBO == true){oCEyBWPuBO = false;}
      if(XjbMEPLBkI == true){XjbMEPLBkI = false;}
      if(iakjndjgoj == true){iakjndjgoj = false;}
      if(zUEzFANmDr == true){zUEzFANmDr = false;}
      if(YXKqmnVPbf == true){YXKqmnVPbf = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OALTFDMIXQ
{ 
  void BUNOhiIhhn()
  { 
      bool FuITzkShlW = false;
      bool yihyecWBwy = false;
      bool rlGfIuVwGp = false;
      bool qjQNpXgEgr = false;
      bool bymsBKxnMD = false;
      bool TnwSZTobLD = false;
      bool PWaueOSUHG = false;
      bool uRUEVsyNdx = false;
      bool ygiDWDnxmW = false;
      bool hEscbFWrNm = false;
      bool qcfFHBTrxg = false;
      bool UbxNFhnWXa = false;
      bool aUgkYpYqbP = false;
      bool VGzqFUAcuV = false;
      bool iuAGCGHeWs = false;
      bool RzTmMVuugQ = false;
      bool bbPgKNcGHj = false;
      bool hKaCqeUahS = false;
      bool isThyVmMei = false;
      bool YEuesHfTwn = false;
      string jlLkQwLJMr;
      string MgxjdQmBtf;
      string jBGudfycDb;
      string yzWwyyXHUs;
      string UsrdUeKVmc;
      string dWoInieQHj;
      string xhFjsNJBkf;
      string sefBKpJcay;
      string VcpaxbJOIP;
      string SHzkummHkH;
      string VJjHjypmxx;
      string ZCoJOUrelR;
      string DtrVMkfuGX;
      string DBqTbLsipz;
      string kVRKpQRpWa;
      string tHUSDaQNuy;
      string SugYtXKVgC;
      string aJnmggcgfw;
      string TpYfcufYPA;
      string temrfporQR;
      if(jlLkQwLJMr == VJjHjypmxx){FuITzkShlW = true;}
      else if(VJjHjypmxx == jlLkQwLJMr){qcfFHBTrxg = true;}
      if(MgxjdQmBtf == ZCoJOUrelR){yihyecWBwy = true;}
      else if(ZCoJOUrelR == MgxjdQmBtf){UbxNFhnWXa = true;}
      if(jBGudfycDb == DtrVMkfuGX){rlGfIuVwGp = true;}
      else if(DtrVMkfuGX == jBGudfycDb){aUgkYpYqbP = true;}
      if(yzWwyyXHUs == DBqTbLsipz){qjQNpXgEgr = true;}
      else if(DBqTbLsipz == yzWwyyXHUs){VGzqFUAcuV = true;}
      if(UsrdUeKVmc == kVRKpQRpWa){bymsBKxnMD = true;}
      else if(kVRKpQRpWa == UsrdUeKVmc){iuAGCGHeWs = true;}
      if(dWoInieQHj == tHUSDaQNuy){TnwSZTobLD = true;}
      else if(tHUSDaQNuy == dWoInieQHj){RzTmMVuugQ = true;}
      if(xhFjsNJBkf == SugYtXKVgC){PWaueOSUHG = true;}
      else if(SugYtXKVgC == xhFjsNJBkf){bbPgKNcGHj = true;}
      if(sefBKpJcay == aJnmggcgfw){uRUEVsyNdx = true;}
      if(VcpaxbJOIP == TpYfcufYPA){ygiDWDnxmW = true;}
      if(SHzkummHkH == temrfporQR){hEscbFWrNm = true;}
      while(aJnmggcgfw == sefBKpJcay){hKaCqeUahS = true;}
      while(TpYfcufYPA == TpYfcufYPA){isThyVmMei = true;}
      while(temrfporQR == temrfporQR){YEuesHfTwn = true;}
      if(FuITzkShlW == true){FuITzkShlW = false;}
      if(yihyecWBwy == true){yihyecWBwy = false;}
      if(rlGfIuVwGp == true){rlGfIuVwGp = false;}
      if(qjQNpXgEgr == true){qjQNpXgEgr = false;}
      if(bymsBKxnMD == true){bymsBKxnMD = false;}
      if(TnwSZTobLD == true){TnwSZTobLD = false;}
      if(PWaueOSUHG == true){PWaueOSUHG = false;}
      if(uRUEVsyNdx == true){uRUEVsyNdx = false;}
      if(ygiDWDnxmW == true){ygiDWDnxmW = false;}
      if(hEscbFWrNm == true){hEscbFWrNm = false;}
      if(qcfFHBTrxg == true){qcfFHBTrxg = false;}
      if(UbxNFhnWXa == true){UbxNFhnWXa = false;}
      if(aUgkYpYqbP == true){aUgkYpYqbP = false;}
      if(VGzqFUAcuV == true){VGzqFUAcuV = false;}
      if(iuAGCGHeWs == true){iuAGCGHeWs = false;}
      if(RzTmMVuugQ == true){RzTmMVuugQ = false;}
      if(bbPgKNcGHj == true){bbPgKNcGHj = false;}
      if(hKaCqeUahS == true){hKaCqeUahS = false;}
      if(isThyVmMei == true){isThyVmMei = false;}
      if(YEuesHfTwn == true){YEuesHfTwn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LZZKQIFFET
{ 
  void wUKLrssFzw()
  { 
      bool iXpSygZUph = false;
      bool iuuwUqxZKy = false;
      bool SKyakujqtb = false;
      bool QNhpVBfYHk = false;
      bool leldwlExqK = false;
      bool WwdCgYZPhC = false;
      bool xhZORUNFaS = false;
      bool KYSZLmXTdy = false;
      bool plAjEUjeIw = false;
      bool bQqEmEFXMb = false;
      bool YzWYzydDqt = false;
      bool BFkiMKhoJx = false;
      bool khwdqsgNmH = false;
      bool gzHCZpPbjF = false;
      bool pgnNuVfIqb = false;
      bool WPNPppDsgG = false;
      bool RPQjNByhIW = false;
      bool uFFDGDthhV = false;
      bool NXVKjQkrLi = false;
      bool lWtdnZQEAt = false;
      string PPJYkFIZqq;
      string JrxrrzQjVF;
      string qkzXaRoGCd;
      string BnyqgpNWks;
      string nMplfBfGfp;
      string dTHXedQkyS;
      string QBWMpUtGLC;
      string lsybQetMrY;
      string lhaLAggZLC;
      string ZbXScmodwK;
      string GZudEjVeHA;
      string iDsZfiaLMI;
      string nUtJbJVezd;
      string pQQhRMmwHP;
      string LBRLSWiylt;
      string LdQmVlelCu;
      string OpecKSmKHT;
      string udDwxmCipF;
      string ZWrFRLAcMf;
      string hAtMBUVoBA;
      if(PPJYkFIZqq == GZudEjVeHA){iXpSygZUph = true;}
      else if(GZudEjVeHA == PPJYkFIZqq){YzWYzydDqt = true;}
      if(JrxrrzQjVF == iDsZfiaLMI){iuuwUqxZKy = true;}
      else if(iDsZfiaLMI == JrxrrzQjVF){BFkiMKhoJx = true;}
      if(qkzXaRoGCd == nUtJbJVezd){SKyakujqtb = true;}
      else if(nUtJbJVezd == qkzXaRoGCd){khwdqsgNmH = true;}
      if(BnyqgpNWks == pQQhRMmwHP){QNhpVBfYHk = true;}
      else if(pQQhRMmwHP == BnyqgpNWks){gzHCZpPbjF = true;}
      if(nMplfBfGfp == LBRLSWiylt){leldwlExqK = true;}
      else if(LBRLSWiylt == nMplfBfGfp){pgnNuVfIqb = true;}
      if(dTHXedQkyS == LdQmVlelCu){WwdCgYZPhC = true;}
      else if(LdQmVlelCu == dTHXedQkyS){WPNPppDsgG = true;}
      if(QBWMpUtGLC == OpecKSmKHT){xhZORUNFaS = true;}
      else if(OpecKSmKHT == QBWMpUtGLC){RPQjNByhIW = true;}
      if(lsybQetMrY == udDwxmCipF){KYSZLmXTdy = true;}
      if(lhaLAggZLC == ZWrFRLAcMf){plAjEUjeIw = true;}
      if(ZbXScmodwK == hAtMBUVoBA){bQqEmEFXMb = true;}
      while(udDwxmCipF == lsybQetMrY){uFFDGDthhV = true;}
      while(ZWrFRLAcMf == ZWrFRLAcMf){NXVKjQkrLi = true;}
      while(hAtMBUVoBA == hAtMBUVoBA){lWtdnZQEAt = true;}
      if(iXpSygZUph == true){iXpSygZUph = false;}
      if(iuuwUqxZKy == true){iuuwUqxZKy = false;}
      if(SKyakujqtb == true){SKyakujqtb = false;}
      if(QNhpVBfYHk == true){QNhpVBfYHk = false;}
      if(leldwlExqK == true){leldwlExqK = false;}
      if(WwdCgYZPhC == true){WwdCgYZPhC = false;}
      if(xhZORUNFaS == true){xhZORUNFaS = false;}
      if(KYSZLmXTdy == true){KYSZLmXTdy = false;}
      if(plAjEUjeIw == true){plAjEUjeIw = false;}
      if(bQqEmEFXMb == true){bQqEmEFXMb = false;}
      if(YzWYzydDqt == true){YzWYzydDqt = false;}
      if(BFkiMKhoJx == true){BFkiMKhoJx = false;}
      if(khwdqsgNmH == true){khwdqsgNmH = false;}
      if(gzHCZpPbjF == true){gzHCZpPbjF = false;}
      if(pgnNuVfIqb == true){pgnNuVfIqb = false;}
      if(WPNPppDsgG == true){WPNPppDsgG = false;}
      if(RPQjNByhIW == true){RPQjNByhIW = false;}
      if(uFFDGDthhV == true){uFFDGDthhV = false;}
      if(NXVKjQkrLi == true){NXVKjQkrLi = false;}
      if(lWtdnZQEAt == true){lWtdnZQEAt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZRCREGIGYS
{ 
  void PmxNrnlqxN()
  { 
      bool jLZgbSWgaO = false;
      bool dJHURmlFVE = false;
      bool IZtqQJrZTV = false;
      bool qDKbQGGtus = false;
      bool ZOYQfKqxNm = false;
      bool njBoWJRhQS = false;
      bool axMEWazfDK = false;
      bool fqKcetBdKG = false;
      bool RtLghHBVFG = false;
      bool oYdbScBUsF = false;
      bool LLsDGwZYKc = false;
      bool InWJAfleWP = false;
      bool YcZxVHNhJB = false;
      bool rgkFuGBHal = false;
      bool hcLcxBBTbm = false;
      bool qVxdfhkwhy = false;
      bool hrOIjbDROt = false;
      bool edEkgwRxzT = false;
      bool GtFWKViHYN = false;
      bool UEYbDpNCrV = false;
      string jjqPBUoNGO;
      string MWqhyAIjRG;
      string uRLUxjcVlu;
      string jINTdeIJDA;
      string rYbjywlJAg;
      string ZnEjxRXWRQ;
      string qSaKeDJXDS;
      string qyfRNWobVB;
      string xTUFVuEcWM;
      string HlqlIZxRFt;
      string lOFhoaHlWa;
      string bwAQTWxVoi;
      string zgAqLEHtJn;
      string YXVaOhNmPt;
      string mRGwjKlaSk;
      string bbbVuEOOVj;
      string TNchBEZZBk;
      string IJFtugaEHB;
      string SfbXLxuzeJ;
      string TzRfZZoXte;
      if(jjqPBUoNGO == lOFhoaHlWa){jLZgbSWgaO = true;}
      else if(lOFhoaHlWa == jjqPBUoNGO){LLsDGwZYKc = true;}
      if(MWqhyAIjRG == bwAQTWxVoi){dJHURmlFVE = true;}
      else if(bwAQTWxVoi == MWqhyAIjRG){InWJAfleWP = true;}
      if(uRLUxjcVlu == zgAqLEHtJn){IZtqQJrZTV = true;}
      else if(zgAqLEHtJn == uRLUxjcVlu){YcZxVHNhJB = true;}
      if(jINTdeIJDA == YXVaOhNmPt){qDKbQGGtus = true;}
      else if(YXVaOhNmPt == jINTdeIJDA){rgkFuGBHal = true;}
      if(rYbjywlJAg == mRGwjKlaSk){ZOYQfKqxNm = true;}
      else if(mRGwjKlaSk == rYbjywlJAg){hcLcxBBTbm = true;}
      if(ZnEjxRXWRQ == bbbVuEOOVj){njBoWJRhQS = true;}
      else if(bbbVuEOOVj == ZnEjxRXWRQ){qVxdfhkwhy = true;}
      if(qSaKeDJXDS == TNchBEZZBk){axMEWazfDK = true;}
      else if(TNchBEZZBk == qSaKeDJXDS){hrOIjbDROt = true;}
      if(qyfRNWobVB == IJFtugaEHB){fqKcetBdKG = true;}
      if(xTUFVuEcWM == SfbXLxuzeJ){RtLghHBVFG = true;}
      if(HlqlIZxRFt == TzRfZZoXte){oYdbScBUsF = true;}
      while(IJFtugaEHB == qyfRNWobVB){edEkgwRxzT = true;}
      while(SfbXLxuzeJ == SfbXLxuzeJ){GtFWKViHYN = true;}
      while(TzRfZZoXte == TzRfZZoXte){UEYbDpNCrV = true;}
      if(jLZgbSWgaO == true){jLZgbSWgaO = false;}
      if(dJHURmlFVE == true){dJHURmlFVE = false;}
      if(IZtqQJrZTV == true){IZtqQJrZTV = false;}
      if(qDKbQGGtus == true){qDKbQGGtus = false;}
      if(ZOYQfKqxNm == true){ZOYQfKqxNm = false;}
      if(njBoWJRhQS == true){njBoWJRhQS = false;}
      if(axMEWazfDK == true){axMEWazfDK = false;}
      if(fqKcetBdKG == true){fqKcetBdKG = false;}
      if(RtLghHBVFG == true){RtLghHBVFG = false;}
      if(oYdbScBUsF == true){oYdbScBUsF = false;}
      if(LLsDGwZYKc == true){LLsDGwZYKc = false;}
      if(InWJAfleWP == true){InWJAfleWP = false;}
      if(YcZxVHNhJB == true){YcZxVHNhJB = false;}
      if(rgkFuGBHal == true){rgkFuGBHal = false;}
      if(hcLcxBBTbm == true){hcLcxBBTbm = false;}
      if(qVxdfhkwhy == true){qVxdfhkwhy = false;}
      if(hrOIjbDROt == true){hrOIjbDROt = false;}
      if(edEkgwRxzT == true){edEkgwRxzT = false;}
      if(GtFWKViHYN == true){GtFWKViHYN = false;}
      if(UEYbDpNCrV == true){UEYbDpNCrV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DGWGNGSHZE
{ 
  void TPXCnWkFJg()
  { 
      bool TFmeEttUxq = false;
      bool pOSEWqoSXI = false;
      bool lObxBPqzqC = false;
      bool BhIzJQBLnV = false;
      bool TfpLFysbTS = false;
      bool htsTtEdodn = false;
      bool CtdtoIFSuF = false;
      bool oceCoFYQqD = false;
      bool WOfXUHonFE = false;
      bool mMhzTYCuVx = false;
      bool rZgNdEOqen = false;
      bool fNxZRmOFYh = false;
      bool SQNpSctfiP = false;
      bool CcttKwgRtQ = false;
      bool ryJQXZPHhf = false;
      bool JxWndbMquB = false;
      bool JiIPlDTDPU = false;
      bool FxLIoUHxSr = false;
      bool MmyCFFGFPi = false;
      bool jpWsHJptdU = false;
      string qoqdXTAfQx;
      string cXHJuHODuf;
      string YpOwNwdPWe;
      string wfoyfKWexG;
      string qRBCpdwYSl;
      string lYYLbAZuQb;
      string okJlAwhJcj;
      string jjGooGnMFi;
      string AExksWHcWe;
      string jbOUQGIPBt;
      string umuhBBSiWV;
      string LNpgbkmYsd;
      string nFlyAIVkXi;
      string yYKKmMJOiq;
      string EYFASWkgsf;
      string SjWWGpAxhs;
      string fgLnXmCftO;
      string bYoBZPzlec;
      string WnqrPOBmDw;
      string MLgpKWNRcf;
      if(qoqdXTAfQx == umuhBBSiWV){TFmeEttUxq = true;}
      else if(umuhBBSiWV == qoqdXTAfQx){rZgNdEOqen = true;}
      if(cXHJuHODuf == LNpgbkmYsd){pOSEWqoSXI = true;}
      else if(LNpgbkmYsd == cXHJuHODuf){fNxZRmOFYh = true;}
      if(YpOwNwdPWe == nFlyAIVkXi){lObxBPqzqC = true;}
      else if(nFlyAIVkXi == YpOwNwdPWe){SQNpSctfiP = true;}
      if(wfoyfKWexG == yYKKmMJOiq){BhIzJQBLnV = true;}
      else if(yYKKmMJOiq == wfoyfKWexG){CcttKwgRtQ = true;}
      if(qRBCpdwYSl == EYFASWkgsf){TfpLFysbTS = true;}
      else if(EYFASWkgsf == qRBCpdwYSl){ryJQXZPHhf = true;}
      if(lYYLbAZuQb == SjWWGpAxhs){htsTtEdodn = true;}
      else if(SjWWGpAxhs == lYYLbAZuQb){JxWndbMquB = true;}
      if(okJlAwhJcj == fgLnXmCftO){CtdtoIFSuF = true;}
      else if(fgLnXmCftO == okJlAwhJcj){JiIPlDTDPU = true;}
      if(jjGooGnMFi == bYoBZPzlec){oceCoFYQqD = true;}
      if(AExksWHcWe == WnqrPOBmDw){WOfXUHonFE = true;}
      if(jbOUQGIPBt == MLgpKWNRcf){mMhzTYCuVx = true;}
      while(bYoBZPzlec == jjGooGnMFi){FxLIoUHxSr = true;}
      while(WnqrPOBmDw == WnqrPOBmDw){MmyCFFGFPi = true;}
      while(MLgpKWNRcf == MLgpKWNRcf){jpWsHJptdU = true;}
      if(TFmeEttUxq == true){TFmeEttUxq = false;}
      if(pOSEWqoSXI == true){pOSEWqoSXI = false;}
      if(lObxBPqzqC == true){lObxBPqzqC = false;}
      if(BhIzJQBLnV == true){BhIzJQBLnV = false;}
      if(TfpLFysbTS == true){TfpLFysbTS = false;}
      if(htsTtEdodn == true){htsTtEdodn = false;}
      if(CtdtoIFSuF == true){CtdtoIFSuF = false;}
      if(oceCoFYQqD == true){oceCoFYQqD = false;}
      if(WOfXUHonFE == true){WOfXUHonFE = false;}
      if(mMhzTYCuVx == true){mMhzTYCuVx = false;}
      if(rZgNdEOqen == true){rZgNdEOqen = false;}
      if(fNxZRmOFYh == true){fNxZRmOFYh = false;}
      if(SQNpSctfiP == true){SQNpSctfiP = false;}
      if(CcttKwgRtQ == true){CcttKwgRtQ = false;}
      if(ryJQXZPHhf == true){ryJQXZPHhf = false;}
      if(JxWndbMquB == true){JxWndbMquB = false;}
      if(JiIPlDTDPU == true){JiIPlDTDPU = false;}
      if(FxLIoUHxSr == true){FxLIoUHxSr = false;}
      if(MmyCFFGFPi == true){MmyCFFGFPi = false;}
      if(jpWsHJptdU == true){jpWsHJptdU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XVMWEHNRKQ
{ 
  void ILZxfntWoQ()
  { 
      bool RAiAIXsfgR = false;
      bool BaJzdfsdin = false;
      bool AeewPrDtNe = false;
      bool NhEmqpcrpr = false;
      bool VQrUUhNkIh = false;
      bool XQfkJmUmjP = false;
      bool TySCESVVIm = false;
      bool DBmqRilKah = false;
      bool eRlfGxCTdE = false;
      bool VoHaXXypQB = false;
      bool LrcUdWcnSZ = false;
      bool AKLCkCMluH = false;
      bool QBrDRNsMTt = false;
      bool jwtZVmoQBj = false;
      bool TNBDELcFpV = false;
      bool KLtIjJzYDQ = false;
      bool cQUYoEPlfy = false;
      bool MwkLmDAnzG = false;
      bool jnVxiYBkoM = false;
      bool mPMkHDrbNA = false;
      string ZKpMNqCxLr;
      string WosUdNMonk;
      string qbUlsGKiYT;
      string jQmRHTEJPz;
      string NmgmOMjWzn;
      string IbpyxxoDTG;
      string gSYrHGBaNJ;
      string JDYoWDthWf;
      string VmjkKHZoUJ;
      string dAoIRuOoeI;
      string KJJodFCBas;
      string CHgnjsingA;
      string gLFDfJNQXb;
      string oFDOUMctGF;
      string EYObGVSBXY;
      string LZHdayEFJU;
      string qlczDxESSX;
      string PngbwIxcwJ;
      string pLrzPPxZmV;
      string IXklNRuPYJ;
      if(ZKpMNqCxLr == KJJodFCBas){RAiAIXsfgR = true;}
      else if(KJJodFCBas == ZKpMNqCxLr){LrcUdWcnSZ = true;}
      if(WosUdNMonk == CHgnjsingA){BaJzdfsdin = true;}
      else if(CHgnjsingA == WosUdNMonk){AKLCkCMluH = true;}
      if(qbUlsGKiYT == gLFDfJNQXb){AeewPrDtNe = true;}
      else if(gLFDfJNQXb == qbUlsGKiYT){QBrDRNsMTt = true;}
      if(jQmRHTEJPz == oFDOUMctGF){NhEmqpcrpr = true;}
      else if(oFDOUMctGF == jQmRHTEJPz){jwtZVmoQBj = true;}
      if(NmgmOMjWzn == EYObGVSBXY){VQrUUhNkIh = true;}
      else if(EYObGVSBXY == NmgmOMjWzn){TNBDELcFpV = true;}
      if(IbpyxxoDTG == LZHdayEFJU){XQfkJmUmjP = true;}
      else if(LZHdayEFJU == IbpyxxoDTG){KLtIjJzYDQ = true;}
      if(gSYrHGBaNJ == qlczDxESSX){TySCESVVIm = true;}
      else if(qlczDxESSX == gSYrHGBaNJ){cQUYoEPlfy = true;}
      if(JDYoWDthWf == PngbwIxcwJ){DBmqRilKah = true;}
      if(VmjkKHZoUJ == pLrzPPxZmV){eRlfGxCTdE = true;}
      if(dAoIRuOoeI == IXklNRuPYJ){VoHaXXypQB = true;}
      while(PngbwIxcwJ == JDYoWDthWf){MwkLmDAnzG = true;}
      while(pLrzPPxZmV == pLrzPPxZmV){jnVxiYBkoM = true;}
      while(IXklNRuPYJ == IXklNRuPYJ){mPMkHDrbNA = true;}
      if(RAiAIXsfgR == true){RAiAIXsfgR = false;}
      if(BaJzdfsdin == true){BaJzdfsdin = false;}
      if(AeewPrDtNe == true){AeewPrDtNe = false;}
      if(NhEmqpcrpr == true){NhEmqpcrpr = false;}
      if(VQrUUhNkIh == true){VQrUUhNkIh = false;}
      if(XQfkJmUmjP == true){XQfkJmUmjP = false;}
      if(TySCESVVIm == true){TySCESVVIm = false;}
      if(DBmqRilKah == true){DBmqRilKah = false;}
      if(eRlfGxCTdE == true){eRlfGxCTdE = false;}
      if(VoHaXXypQB == true){VoHaXXypQB = false;}
      if(LrcUdWcnSZ == true){LrcUdWcnSZ = false;}
      if(AKLCkCMluH == true){AKLCkCMluH = false;}
      if(QBrDRNsMTt == true){QBrDRNsMTt = false;}
      if(jwtZVmoQBj == true){jwtZVmoQBj = false;}
      if(TNBDELcFpV == true){TNBDELcFpV = false;}
      if(KLtIjJzYDQ == true){KLtIjJzYDQ = false;}
      if(cQUYoEPlfy == true){cQUYoEPlfy = false;}
      if(MwkLmDAnzG == true){MwkLmDAnzG = false;}
      if(jnVxiYBkoM == true){jnVxiYBkoM = false;}
      if(mPMkHDrbNA == true){mPMkHDrbNA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FMOMRVUIUQ
{ 
  void VEnoBtEeOj()
  { 
      bool cEwCfeTOzJ = false;
      bool FbRJzsCzcE = false;
      bool jrYkkIefDC = false;
      bool SLUsAfiWqS = false;
      bool nMhkIoNujo = false;
      bool pwWGykeKQW = false;
      bool AnLwptGYIS = false;
      bool JfIJQXKmcu = false;
      bool ujuOLVoOuP = false;
      bool AYFTSzAktO = false;
      bool holReKmMpZ = false;
      bool argUaQsxjT = false;
      bool ErcsRkanqQ = false;
      bool WYsONGxdiB = false;
      bool ibYUoIqjsB = false;
      bool GDAZJpLszt = false;
      bool jzUYDBySHT = false;
      bool SONikcTHEJ = false;
      bool spuoKTYSFW = false;
      bool tEeoRiGzRm = false;
      string PmsHlKDJGo;
      string kHUrgbuJLd;
      string XIwmqYIRQJ;
      string uWFpnwNrwk;
      string bMoGRGnSFF;
      string DTopPGqFPN;
      string ClsxsNCdDk;
      string ODNaBKSoUj;
      string joLKGbgOag;
      string EHbhhoSqMD;
      string EKPTqzGOnE;
      string mBjfcNaiqR;
      string fmCVgHaerM;
      string rbbRfZYqTG;
      string VSaWtCWwFU;
      string OjAuWRHsmi;
      string XbWaMLYSzD;
      string niPkwNLfbd;
      string jeYWyHbgtN;
      string ReEjhsuzKh;
      if(PmsHlKDJGo == EKPTqzGOnE){cEwCfeTOzJ = true;}
      else if(EKPTqzGOnE == PmsHlKDJGo){holReKmMpZ = true;}
      if(kHUrgbuJLd == mBjfcNaiqR){FbRJzsCzcE = true;}
      else if(mBjfcNaiqR == kHUrgbuJLd){argUaQsxjT = true;}
      if(XIwmqYIRQJ == fmCVgHaerM){jrYkkIefDC = true;}
      else if(fmCVgHaerM == XIwmqYIRQJ){ErcsRkanqQ = true;}
      if(uWFpnwNrwk == rbbRfZYqTG){SLUsAfiWqS = true;}
      else if(rbbRfZYqTG == uWFpnwNrwk){WYsONGxdiB = true;}
      if(bMoGRGnSFF == VSaWtCWwFU){nMhkIoNujo = true;}
      else if(VSaWtCWwFU == bMoGRGnSFF){ibYUoIqjsB = true;}
      if(DTopPGqFPN == OjAuWRHsmi){pwWGykeKQW = true;}
      else if(OjAuWRHsmi == DTopPGqFPN){GDAZJpLszt = true;}
      if(ClsxsNCdDk == XbWaMLYSzD){AnLwptGYIS = true;}
      else if(XbWaMLYSzD == ClsxsNCdDk){jzUYDBySHT = true;}
      if(ODNaBKSoUj == niPkwNLfbd){JfIJQXKmcu = true;}
      if(joLKGbgOag == jeYWyHbgtN){ujuOLVoOuP = true;}
      if(EHbhhoSqMD == ReEjhsuzKh){AYFTSzAktO = true;}
      while(niPkwNLfbd == ODNaBKSoUj){SONikcTHEJ = true;}
      while(jeYWyHbgtN == jeYWyHbgtN){spuoKTYSFW = true;}
      while(ReEjhsuzKh == ReEjhsuzKh){tEeoRiGzRm = true;}
      if(cEwCfeTOzJ == true){cEwCfeTOzJ = false;}
      if(FbRJzsCzcE == true){FbRJzsCzcE = false;}
      if(jrYkkIefDC == true){jrYkkIefDC = false;}
      if(SLUsAfiWqS == true){SLUsAfiWqS = false;}
      if(nMhkIoNujo == true){nMhkIoNujo = false;}
      if(pwWGykeKQW == true){pwWGykeKQW = false;}
      if(AnLwptGYIS == true){AnLwptGYIS = false;}
      if(JfIJQXKmcu == true){JfIJQXKmcu = false;}
      if(ujuOLVoOuP == true){ujuOLVoOuP = false;}
      if(AYFTSzAktO == true){AYFTSzAktO = false;}
      if(holReKmMpZ == true){holReKmMpZ = false;}
      if(argUaQsxjT == true){argUaQsxjT = false;}
      if(ErcsRkanqQ == true){ErcsRkanqQ = false;}
      if(WYsONGxdiB == true){WYsONGxdiB = false;}
      if(ibYUoIqjsB == true){ibYUoIqjsB = false;}
      if(GDAZJpLszt == true){GDAZJpLszt = false;}
      if(jzUYDBySHT == true){jzUYDBySHT = false;}
      if(SONikcTHEJ == true){SONikcTHEJ = false;}
      if(spuoKTYSFW == true){spuoKTYSFW = false;}
      if(tEeoRiGzRm == true){tEeoRiGzRm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VYVQURTWPN
{ 
  void gtDAxTbKis()
  { 
      bool iYSlhkdDxu = false;
      bool gdjeObSSRV = false;
      bool WpzjkwjTbr = false;
      bool XiutcCsJDf = false;
      bool zOQBzOxYaQ = false;
      bool xKRwSrZhBl = false;
      bool AKaFWxipCL = false;
      bool iIPkVHYxjW = false;
      bool yLrLtNRyaj = false;
      bool cKIRfomKdB = false;
      bool eHWPgoufaK = false;
      bool SbyenoRPLJ = false;
      bool fkORhXzyFg = false;
      bool UOSiqqncdE = false;
      bool SkXQQBMdcp = false;
      bool WXzUVtrEgI = false;
      bool oxaspEJPmw = false;
      bool AnkGptNxBg = false;
      bool LVUPJwOyMw = false;
      bool ziwUPVKdaM = false;
      string ywLETrKYpd;
      string IHLVOsYBpl;
      string fFGomugnQY;
      string ghZwBCixFB;
      string hkpqJTmACx;
      string JYseLsoWQX;
      string LWIQwZgPUL;
      string JauoDtOPeT;
      string IgkTPInujH;
      string MMfgpLcYti;
      string lWfpmicEbR;
      string dZqxPdkoBn;
      string lleMzxtNyr;
      string RoUugkAufg;
      string ejSRdmUiRL;
      string amXGxAqSqp;
      string nOFiznEwQe;
      string coIUICSNiL;
      string bLwlRdROxf;
      string sgEorQdjRo;
      if(ywLETrKYpd == lWfpmicEbR){iYSlhkdDxu = true;}
      else if(lWfpmicEbR == ywLETrKYpd){eHWPgoufaK = true;}
      if(IHLVOsYBpl == dZqxPdkoBn){gdjeObSSRV = true;}
      else if(dZqxPdkoBn == IHLVOsYBpl){SbyenoRPLJ = true;}
      if(fFGomugnQY == lleMzxtNyr){WpzjkwjTbr = true;}
      else if(lleMzxtNyr == fFGomugnQY){fkORhXzyFg = true;}
      if(ghZwBCixFB == RoUugkAufg){XiutcCsJDf = true;}
      else if(RoUugkAufg == ghZwBCixFB){UOSiqqncdE = true;}
      if(hkpqJTmACx == ejSRdmUiRL){zOQBzOxYaQ = true;}
      else if(ejSRdmUiRL == hkpqJTmACx){SkXQQBMdcp = true;}
      if(JYseLsoWQX == amXGxAqSqp){xKRwSrZhBl = true;}
      else if(amXGxAqSqp == JYseLsoWQX){WXzUVtrEgI = true;}
      if(LWIQwZgPUL == nOFiznEwQe){AKaFWxipCL = true;}
      else if(nOFiznEwQe == LWIQwZgPUL){oxaspEJPmw = true;}
      if(JauoDtOPeT == coIUICSNiL){iIPkVHYxjW = true;}
      if(IgkTPInujH == bLwlRdROxf){yLrLtNRyaj = true;}
      if(MMfgpLcYti == sgEorQdjRo){cKIRfomKdB = true;}
      while(coIUICSNiL == JauoDtOPeT){AnkGptNxBg = true;}
      while(bLwlRdROxf == bLwlRdROxf){LVUPJwOyMw = true;}
      while(sgEorQdjRo == sgEorQdjRo){ziwUPVKdaM = true;}
      if(iYSlhkdDxu == true){iYSlhkdDxu = false;}
      if(gdjeObSSRV == true){gdjeObSSRV = false;}
      if(WpzjkwjTbr == true){WpzjkwjTbr = false;}
      if(XiutcCsJDf == true){XiutcCsJDf = false;}
      if(zOQBzOxYaQ == true){zOQBzOxYaQ = false;}
      if(xKRwSrZhBl == true){xKRwSrZhBl = false;}
      if(AKaFWxipCL == true){AKaFWxipCL = false;}
      if(iIPkVHYxjW == true){iIPkVHYxjW = false;}
      if(yLrLtNRyaj == true){yLrLtNRyaj = false;}
      if(cKIRfomKdB == true){cKIRfomKdB = false;}
      if(eHWPgoufaK == true){eHWPgoufaK = false;}
      if(SbyenoRPLJ == true){SbyenoRPLJ = false;}
      if(fkORhXzyFg == true){fkORhXzyFg = false;}
      if(UOSiqqncdE == true){UOSiqqncdE = false;}
      if(SkXQQBMdcp == true){SkXQQBMdcp = false;}
      if(WXzUVtrEgI == true){WXzUVtrEgI = false;}
      if(oxaspEJPmw == true){oxaspEJPmw = false;}
      if(AnkGptNxBg == true){AnkGptNxBg = false;}
      if(LVUPJwOyMw == true){LVUPJwOyMw = false;}
      if(ziwUPVKdaM == true){ziwUPVKdaM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GKLEGISQEV
{ 
  void NxdLcDjtXW()
  { 
      bool VTWbhTDkGo = false;
      bool KqqjawViPE = false;
      bool BlNjLEFJOM = false;
      bool PILXBAJISa = false;
      bool JODiwZnecY = false;
      bool xcPZkjGlQc = false;
      bool OtVsZCeSiZ = false;
      bool mNQotmienk = false;
      bool PZfpPlrkmR = false;
      bool tWqoMPsJIt = false;
      bool uVwCkiLafy = false;
      bool AbyeNnNfdm = false;
      bool JsfReQVbce = false;
      bool tYVtptdcEf = false;
      bool aQFnQSYMbX = false;
      bool BMeVjBoeXU = false;
      bool NjyBEaFDoS = false;
      bool KjbyMrnrAM = false;
      bool rJAEdgsAJo = false;
      bool FzFiSpSEio = false;
      string rzyjMTbjIP;
      string aadgtyLpzK;
      string TpPqtTLPBC;
      string kEEyQtoDWu;
      string sLMYLEBVwf;
      string fOGOECjHos;
      string ILhGdVWfQz;
      string IISERHcqep;
      string SoTVVWyosO;
      string KhNdlgcGto;
      string hUFwWWuCww;
      string YrbzZTrMjH;
      string XhmBoatnSk;
      string eWwpHyiWzy;
      string awrwpRFOER;
      string oQWmRAgDHu;
      string cGjLRCulqb;
      string jUMkgbopLW;
      string UfjAuNAJXH;
      string QfPDVTMfem;
      if(rzyjMTbjIP == hUFwWWuCww){VTWbhTDkGo = true;}
      else if(hUFwWWuCww == rzyjMTbjIP){uVwCkiLafy = true;}
      if(aadgtyLpzK == YrbzZTrMjH){KqqjawViPE = true;}
      else if(YrbzZTrMjH == aadgtyLpzK){AbyeNnNfdm = true;}
      if(TpPqtTLPBC == XhmBoatnSk){BlNjLEFJOM = true;}
      else if(XhmBoatnSk == TpPqtTLPBC){JsfReQVbce = true;}
      if(kEEyQtoDWu == eWwpHyiWzy){PILXBAJISa = true;}
      else if(eWwpHyiWzy == kEEyQtoDWu){tYVtptdcEf = true;}
      if(sLMYLEBVwf == awrwpRFOER){JODiwZnecY = true;}
      else if(awrwpRFOER == sLMYLEBVwf){aQFnQSYMbX = true;}
      if(fOGOECjHos == oQWmRAgDHu){xcPZkjGlQc = true;}
      else if(oQWmRAgDHu == fOGOECjHos){BMeVjBoeXU = true;}
      if(ILhGdVWfQz == cGjLRCulqb){OtVsZCeSiZ = true;}
      else if(cGjLRCulqb == ILhGdVWfQz){NjyBEaFDoS = true;}
      if(IISERHcqep == jUMkgbopLW){mNQotmienk = true;}
      if(SoTVVWyosO == UfjAuNAJXH){PZfpPlrkmR = true;}
      if(KhNdlgcGto == QfPDVTMfem){tWqoMPsJIt = true;}
      while(jUMkgbopLW == IISERHcqep){KjbyMrnrAM = true;}
      while(UfjAuNAJXH == UfjAuNAJXH){rJAEdgsAJo = true;}
      while(QfPDVTMfem == QfPDVTMfem){FzFiSpSEio = true;}
      if(VTWbhTDkGo == true){VTWbhTDkGo = false;}
      if(KqqjawViPE == true){KqqjawViPE = false;}
      if(BlNjLEFJOM == true){BlNjLEFJOM = false;}
      if(PILXBAJISa == true){PILXBAJISa = false;}
      if(JODiwZnecY == true){JODiwZnecY = false;}
      if(xcPZkjGlQc == true){xcPZkjGlQc = false;}
      if(OtVsZCeSiZ == true){OtVsZCeSiZ = false;}
      if(mNQotmienk == true){mNQotmienk = false;}
      if(PZfpPlrkmR == true){PZfpPlrkmR = false;}
      if(tWqoMPsJIt == true){tWqoMPsJIt = false;}
      if(uVwCkiLafy == true){uVwCkiLafy = false;}
      if(AbyeNnNfdm == true){AbyeNnNfdm = false;}
      if(JsfReQVbce == true){JsfReQVbce = false;}
      if(tYVtptdcEf == true){tYVtptdcEf = false;}
      if(aQFnQSYMbX == true){aQFnQSYMbX = false;}
      if(BMeVjBoeXU == true){BMeVjBoeXU = false;}
      if(NjyBEaFDoS == true){NjyBEaFDoS = false;}
      if(KjbyMrnrAM == true){KjbyMrnrAM = false;}
      if(rJAEdgsAJo == true){rJAEdgsAJo = false;}
      if(FzFiSpSEio == true){FzFiSpSEio = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KBHOFHIUEM
{ 
  void imNcFEcSCO()
  { 
      bool YVVtYxEces = false;
      bool MaKStUdgVq = false;
      bool DRFnnBzckB = false;
      bool FKgiVMUecn = false;
      bool KUJlWpDGhJ = false;
      bool jwCZrGQhFX = false;
      bool ybrdmBtawh = false;
      bool AzDhfhISRy = false;
      bool yrtbRJIGpP = false;
      bool wmZFoicfzU = false;
      bool wbKAIjrTsK = false;
      bool yxPGYGgPLd = false;
      bool rmjgkNYxrw = false;
      bool EbTwCVgsSg = false;
      bool RnNfQyWSgi = false;
      bool CNTVbIqdPu = false;
      bool LtbxFVdSED = false;
      bool YmxeLEZEPl = false;
      bool pGQItqHyTs = false;
      bool dDdWVWMQZt = false;
      string RSaGkxzWLR;
      string FKPuYWZuxK;
      string LhPwMFIMjR;
      string qSAnaaeqaw;
      string ULrDNVVmlK;
      string excFhjDdte;
      string nlmsamKbUx;
      string qhUApzpMiq;
      string jVaZmtpuFq;
      string WBxuHhYybZ;
      string LtSnSwbbOh;
      string sqtDuSLNFg;
      string xOVcaokYrH;
      string aqjIeoDmGp;
      string RNfXyfGtzo;
      string VLsucwYoNc;
      string LDjHGBHkXF;
      string PYGPPiixet;
      string hqYCkPyofi;
      string QNZcdNcXQw;
      if(RSaGkxzWLR == LtSnSwbbOh){YVVtYxEces = true;}
      else if(LtSnSwbbOh == RSaGkxzWLR){wbKAIjrTsK = true;}
      if(FKPuYWZuxK == sqtDuSLNFg){MaKStUdgVq = true;}
      else if(sqtDuSLNFg == FKPuYWZuxK){yxPGYGgPLd = true;}
      if(LhPwMFIMjR == xOVcaokYrH){DRFnnBzckB = true;}
      else if(xOVcaokYrH == LhPwMFIMjR){rmjgkNYxrw = true;}
      if(qSAnaaeqaw == aqjIeoDmGp){FKgiVMUecn = true;}
      else if(aqjIeoDmGp == qSAnaaeqaw){EbTwCVgsSg = true;}
      if(ULrDNVVmlK == RNfXyfGtzo){KUJlWpDGhJ = true;}
      else if(RNfXyfGtzo == ULrDNVVmlK){RnNfQyWSgi = true;}
      if(excFhjDdte == VLsucwYoNc){jwCZrGQhFX = true;}
      else if(VLsucwYoNc == excFhjDdte){CNTVbIqdPu = true;}
      if(nlmsamKbUx == LDjHGBHkXF){ybrdmBtawh = true;}
      else if(LDjHGBHkXF == nlmsamKbUx){LtbxFVdSED = true;}
      if(qhUApzpMiq == PYGPPiixet){AzDhfhISRy = true;}
      if(jVaZmtpuFq == hqYCkPyofi){yrtbRJIGpP = true;}
      if(WBxuHhYybZ == QNZcdNcXQw){wmZFoicfzU = true;}
      while(PYGPPiixet == qhUApzpMiq){YmxeLEZEPl = true;}
      while(hqYCkPyofi == hqYCkPyofi){pGQItqHyTs = true;}
      while(QNZcdNcXQw == QNZcdNcXQw){dDdWVWMQZt = true;}
      if(YVVtYxEces == true){YVVtYxEces = false;}
      if(MaKStUdgVq == true){MaKStUdgVq = false;}
      if(DRFnnBzckB == true){DRFnnBzckB = false;}
      if(FKgiVMUecn == true){FKgiVMUecn = false;}
      if(KUJlWpDGhJ == true){KUJlWpDGhJ = false;}
      if(jwCZrGQhFX == true){jwCZrGQhFX = false;}
      if(ybrdmBtawh == true){ybrdmBtawh = false;}
      if(AzDhfhISRy == true){AzDhfhISRy = false;}
      if(yrtbRJIGpP == true){yrtbRJIGpP = false;}
      if(wmZFoicfzU == true){wmZFoicfzU = false;}
      if(wbKAIjrTsK == true){wbKAIjrTsK = false;}
      if(yxPGYGgPLd == true){yxPGYGgPLd = false;}
      if(rmjgkNYxrw == true){rmjgkNYxrw = false;}
      if(EbTwCVgsSg == true){EbTwCVgsSg = false;}
      if(RnNfQyWSgi == true){RnNfQyWSgi = false;}
      if(CNTVbIqdPu == true){CNTVbIqdPu = false;}
      if(LtbxFVdSED == true){LtbxFVdSED = false;}
      if(YmxeLEZEPl == true){YmxeLEZEPl = false;}
      if(pGQItqHyTs == true){pGQItqHyTs = false;}
      if(dDdWVWMQZt == true){dDdWVWMQZt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KFIBZQPKOL
{ 
  void oklwSiOMCY()
  { 
      bool upfUtGkyeN = false;
      bool EMncHsxzTl = false;
      bool ZwkDICtxHG = false;
      bool fWLcYkpniY = false;
      bool hFGKxqRhAH = false;
      bool XyKVayFcLr = false;
      bool LmpYghBKEW = false;
      bool CJQuDToaFR = false;
      bool jMxAhhMoCZ = false;
      bool fNbpMtPhcP = false;
      bool oHccEOgORG = false;
      bool JdWsIzpopz = false;
      bool ULIdqogZhJ = false;
      bool DHxpbciTuT = false;
      bool AxXuQBqihN = false;
      bool DZqctdyzlP = false;
      bool gGgICCggQf = false;
      bool PWtWzXnNNp = false;
      bool xyrTFdPUuy = false;
      bool djhelsQPrO = false;
      string khZpDpYBzT;
      string uelkAzXrgI;
      string cAxsgKIHFe;
      string pHdgGsDLWR;
      string qhEoilPGDF;
      string lRXIHkkgJr;
      string hRwUcnAEeH;
      string wLVPRtuHJt;
      string uBDArYiVZU;
      string IVCejZljMu;
      string kKbwJZwIqN;
      string odWPxgNwbE;
      string EeeJyZJpfq;
      string ILtOIjYHqR;
      string badMedylhC;
      string LpHHqGfSxi;
      string QFczEdiIGM;
      string IFEJbdBSdT;
      string aWIcfSDlXC;
      string EGKHBVxdHB;
      if(khZpDpYBzT == kKbwJZwIqN){upfUtGkyeN = true;}
      else if(kKbwJZwIqN == khZpDpYBzT){oHccEOgORG = true;}
      if(uelkAzXrgI == odWPxgNwbE){EMncHsxzTl = true;}
      else if(odWPxgNwbE == uelkAzXrgI){JdWsIzpopz = true;}
      if(cAxsgKIHFe == EeeJyZJpfq){ZwkDICtxHG = true;}
      else if(EeeJyZJpfq == cAxsgKIHFe){ULIdqogZhJ = true;}
      if(pHdgGsDLWR == ILtOIjYHqR){fWLcYkpniY = true;}
      else if(ILtOIjYHqR == pHdgGsDLWR){DHxpbciTuT = true;}
      if(qhEoilPGDF == badMedylhC){hFGKxqRhAH = true;}
      else if(badMedylhC == qhEoilPGDF){AxXuQBqihN = true;}
      if(lRXIHkkgJr == LpHHqGfSxi){XyKVayFcLr = true;}
      else if(LpHHqGfSxi == lRXIHkkgJr){DZqctdyzlP = true;}
      if(hRwUcnAEeH == QFczEdiIGM){LmpYghBKEW = true;}
      else if(QFczEdiIGM == hRwUcnAEeH){gGgICCggQf = true;}
      if(wLVPRtuHJt == IFEJbdBSdT){CJQuDToaFR = true;}
      if(uBDArYiVZU == aWIcfSDlXC){jMxAhhMoCZ = true;}
      if(IVCejZljMu == EGKHBVxdHB){fNbpMtPhcP = true;}
      while(IFEJbdBSdT == wLVPRtuHJt){PWtWzXnNNp = true;}
      while(aWIcfSDlXC == aWIcfSDlXC){xyrTFdPUuy = true;}
      while(EGKHBVxdHB == EGKHBVxdHB){djhelsQPrO = true;}
      if(upfUtGkyeN == true){upfUtGkyeN = false;}
      if(EMncHsxzTl == true){EMncHsxzTl = false;}
      if(ZwkDICtxHG == true){ZwkDICtxHG = false;}
      if(fWLcYkpniY == true){fWLcYkpniY = false;}
      if(hFGKxqRhAH == true){hFGKxqRhAH = false;}
      if(XyKVayFcLr == true){XyKVayFcLr = false;}
      if(LmpYghBKEW == true){LmpYghBKEW = false;}
      if(CJQuDToaFR == true){CJQuDToaFR = false;}
      if(jMxAhhMoCZ == true){jMxAhhMoCZ = false;}
      if(fNbpMtPhcP == true){fNbpMtPhcP = false;}
      if(oHccEOgORG == true){oHccEOgORG = false;}
      if(JdWsIzpopz == true){JdWsIzpopz = false;}
      if(ULIdqogZhJ == true){ULIdqogZhJ = false;}
      if(DHxpbciTuT == true){DHxpbciTuT = false;}
      if(AxXuQBqihN == true){AxXuQBqihN = false;}
      if(DZqctdyzlP == true){DZqctdyzlP = false;}
      if(gGgICCggQf == true){gGgICCggQf = false;}
      if(PWtWzXnNNp == true){PWtWzXnNNp = false;}
      if(xyrTFdPUuy == true){xyrTFdPUuy = false;}
      if(djhelsQPrO == true){djhelsQPrO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DPRYPBAYJP
{ 
  void PiPEFgOFmE()
  { 
      bool tixowsaEAi = false;
      bool XUrjVALggz = false;
      bool FQTLJMWZaN = false;
      bool fxAjYCKZIa = false;
      bool gNGDHYaark = false;
      bool FKmSRcOlfi = false;
      bool WBcmDaTHLS = false;
      bool olYFSrKPDj = false;
      bool luhoHUKhSw = false;
      bool nYsYDfjSAg = false;
      bool ESPYwTlgzY = false;
      bool RyhuxbSneh = false;
      bool PkoOePrzBm = false;
      bool IrPWfSeJsn = false;
      bool idhtdopybb = false;
      bool MwmRQSdAtf = false;
      bool aleMQZiLPd = false;
      bool wJupzlDtko = false;
      bool ZsXZZMExFM = false;
      bool ndONNwyUcE = false;
      string wgWnmJEeQj;
      string dsXRmYNKRt;
      string kkiOdoiEOI;
      string odncMZCNaF;
      string YoewJgaJBH;
      string HQgTZLxHKy;
      string azPHtzFuEN;
      string WAiMVuFmEx;
      string NYCGBYxTDB;
      string btCceYYWBJ;
      string afEnkGgTyd;
      string FKDNkBwJGV;
      string ClrDkcxGhw;
      string WhluVhUOVt;
      string qdUYuRqorc;
      string bOQeHonADH;
      string IZlBHXXHqC;
      string ghnMdXQSIS;
      string lIOEUHRGmf;
      string sTLNkQrTRX;
      if(wgWnmJEeQj == afEnkGgTyd){tixowsaEAi = true;}
      else if(afEnkGgTyd == wgWnmJEeQj){ESPYwTlgzY = true;}
      if(dsXRmYNKRt == FKDNkBwJGV){XUrjVALggz = true;}
      else if(FKDNkBwJGV == dsXRmYNKRt){RyhuxbSneh = true;}
      if(kkiOdoiEOI == ClrDkcxGhw){FQTLJMWZaN = true;}
      else if(ClrDkcxGhw == kkiOdoiEOI){PkoOePrzBm = true;}
      if(odncMZCNaF == WhluVhUOVt){fxAjYCKZIa = true;}
      else if(WhluVhUOVt == odncMZCNaF){IrPWfSeJsn = true;}
      if(YoewJgaJBH == qdUYuRqorc){gNGDHYaark = true;}
      else if(qdUYuRqorc == YoewJgaJBH){idhtdopybb = true;}
      if(HQgTZLxHKy == bOQeHonADH){FKmSRcOlfi = true;}
      else if(bOQeHonADH == HQgTZLxHKy){MwmRQSdAtf = true;}
      if(azPHtzFuEN == IZlBHXXHqC){WBcmDaTHLS = true;}
      else if(IZlBHXXHqC == azPHtzFuEN){aleMQZiLPd = true;}
      if(WAiMVuFmEx == ghnMdXQSIS){olYFSrKPDj = true;}
      if(NYCGBYxTDB == lIOEUHRGmf){luhoHUKhSw = true;}
      if(btCceYYWBJ == sTLNkQrTRX){nYsYDfjSAg = true;}
      while(ghnMdXQSIS == WAiMVuFmEx){wJupzlDtko = true;}
      while(lIOEUHRGmf == lIOEUHRGmf){ZsXZZMExFM = true;}
      while(sTLNkQrTRX == sTLNkQrTRX){ndONNwyUcE = true;}
      if(tixowsaEAi == true){tixowsaEAi = false;}
      if(XUrjVALggz == true){XUrjVALggz = false;}
      if(FQTLJMWZaN == true){FQTLJMWZaN = false;}
      if(fxAjYCKZIa == true){fxAjYCKZIa = false;}
      if(gNGDHYaark == true){gNGDHYaark = false;}
      if(FKmSRcOlfi == true){FKmSRcOlfi = false;}
      if(WBcmDaTHLS == true){WBcmDaTHLS = false;}
      if(olYFSrKPDj == true){olYFSrKPDj = false;}
      if(luhoHUKhSw == true){luhoHUKhSw = false;}
      if(nYsYDfjSAg == true){nYsYDfjSAg = false;}
      if(ESPYwTlgzY == true){ESPYwTlgzY = false;}
      if(RyhuxbSneh == true){RyhuxbSneh = false;}
      if(PkoOePrzBm == true){PkoOePrzBm = false;}
      if(IrPWfSeJsn == true){IrPWfSeJsn = false;}
      if(idhtdopybb == true){idhtdopybb = false;}
      if(MwmRQSdAtf == true){MwmRQSdAtf = false;}
      if(aleMQZiLPd == true){aleMQZiLPd = false;}
      if(wJupzlDtko == true){wJupzlDtko = false;}
      if(ZsXZZMExFM == true){ZsXZZMExFM = false;}
      if(ndONNwyUcE == true){ndONNwyUcE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YLJUOMPFRN
{ 
  void soLHShAZjH()
  { 
      bool xyThsdGzgI = false;
      bool LphHaQImVM = false;
      bool MjqoZlAfnp = false;
      bool qyHUOWIDYH = false;
      bool ztZYHrwkLA = false;
      bool eImJlMHdHG = false;
      bool caLpQNhNrD = false;
      bool IEONNSKShh = false;
      bool oPUmLiKyhC = false;
      bool PdwReXhjHt = false;
      bool KirHrasYwT = false;
      bool CowZRMGYeg = false;
      bool CiepmdHwio = false;
      bool jZKmWFWMtj = false;
      bool okBOzagVAS = false;
      bool RIFaraECTO = false;
      bool TwpHibdJDL = false;
      bool osetqQhPpM = false;
      bool wkwQXaUjtu = false;
      bool BUrfQrutfW = false;
      string AYbEgXjyRV;
      string Ppmcrydfsz;
      string yWbPdfTyao;
      string FuijptizCL;
      string OwzmEzpMRE;
      string dAOBzdxLaA;
      string ElWcQUGDlx;
      string OddhkCWnLe;
      string oGfWLEghkM;
      string xWYBGbYWWi;
      string rmaTozWfQw;
      string GIAuKxFAQL;
      string nYSYArFbaI;
      string dzRblCfLhL;
      string hARnFnWcpJ;
      string WYFKuSTBTw;
      string stEHRGqXII;
      string QsRFdmHdTL;
      string VclRkFtGgS;
      string herlRmuJVy;
      if(AYbEgXjyRV == rmaTozWfQw){xyThsdGzgI = true;}
      else if(rmaTozWfQw == AYbEgXjyRV){KirHrasYwT = true;}
      if(Ppmcrydfsz == GIAuKxFAQL){LphHaQImVM = true;}
      else if(GIAuKxFAQL == Ppmcrydfsz){CowZRMGYeg = true;}
      if(yWbPdfTyao == nYSYArFbaI){MjqoZlAfnp = true;}
      else if(nYSYArFbaI == yWbPdfTyao){CiepmdHwio = true;}
      if(FuijptizCL == dzRblCfLhL){qyHUOWIDYH = true;}
      else if(dzRblCfLhL == FuijptizCL){jZKmWFWMtj = true;}
      if(OwzmEzpMRE == hARnFnWcpJ){ztZYHrwkLA = true;}
      else if(hARnFnWcpJ == OwzmEzpMRE){okBOzagVAS = true;}
      if(dAOBzdxLaA == WYFKuSTBTw){eImJlMHdHG = true;}
      else if(WYFKuSTBTw == dAOBzdxLaA){RIFaraECTO = true;}
      if(ElWcQUGDlx == stEHRGqXII){caLpQNhNrD = true;}
      else if(stEHRGqXII == ElWcQUGDlx){TwpHibdJDL = true;}
      if(OddhkCWnLe == QsRFdmHdTL){IEONNSKShh = true;}
      if(oGfWLEghkM == VclRkFtGgS){oPUmLiKyhC = true;}
      if(xWYBGbYWWi == herlRmuJVy){PdwReXhjHt = true;}
      while(QsRFdmHdTL == OddhkCWnLe){osetqQhPpM = true;}
      while(VclRkFtGgS == VclRkFtGgS){wkwQXaUjtu = true;}
      while(herlRmuJVy == herlRmuJVy){BUrfQrutfW = true;}
      if(xyThsdGzgI == true){xyThsdGzgI = false;}
      if(LphHaQImVM == true){LphHaQImVM = false;}
      if(MjqoZlAfnp == true){MjqoZlAfnp = false;}
      if(qyHUOWIDYH == true){qyHUOWIDYH = false;}
      if(ztZYHrwkLA == true){ztZYHrwkLA = false;}
      if(eImJlMHdHG == true){eImJlMHdHG = false;}
      if(caLpQNhNrD == true){caLpQNhNrD = false;}
      if(IEONNSKShh == true){IEONNSKShh = false;}
      if(oPUmLiKyhC == true){oPUmLiKyhC = false;}
      if(PdwReXhjHt == true){PdwReXhjHt = false;}
      if(KirHrasYwT == true){KirHrasYwT = false;}
      if(CowZRMGYeg == true){CowZRMGYeg = false;}
      if(CiepmdHwio == true){CiepmdHwio = false;}
      if(jZKmWFWMtj == true){jZKmWFWMtj = false;}
      if(okBOzagVAS == true){okBOzagVAS = false;}
      if(RIFaraECTO == true){RIFaraECTO = false;}
      if(TwpHibdJDL == true){TwpHibdJDL = false;}
      if(osetqQhPpM == true){osetqQhPpM = false;}
      if(wkwQXaUjtu == true){wkwQXaUjtu = false;}
      if(BUrfQrutfW == true){BUrfQrutfW = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DTBZUHWIOG
{ 
  void mcIDRSFtpU()
  { 
      bool DuIUBlRWop = false;
      bool fUYZEsOTqD = false;
      bool ueQPdmUJpO = false;
      bool QLAwhCfbhu = false;
      bool wSNUhOwOVD = false;
      bool zaKTmXerpN = false;
      bool RIfaYUUcrB = false;
      bool NmluHcRuhP = false;
      bool qFmlOoAgIJ = false;
      bool XmkUcmlmuN = false;
      bool tTVTpMoOLK = false;
      bool GiKaWLVtLi = false;
      bool TnThcZBQST = false;
      bool pdAUUXiSwB = false;
      bool zlUyDobJyQ = false;
      bool RraLsGznqP = false;
      bool KYurTbhoGz = false;
      bool bCyJigSVem = false;
      bool yrSBCyCnaO = false;
      bool Xubfqapbea = false;
      string YhLHEIfQAl;
      string sjXZIEqKEl;
      string HLiwjcqkDt;
      string QkSlTrZsCN;
      string BbipVysLBC;
      string GznnNNyOWb;
      string RtNGUHRlGQ;
      string exHpkebral;
      string jfjAnRwdYF;
      string sKWawGOcVN;
      string fGFjXIqxDi;
      string kGnjqkKMSD;
      string LmNAXUaftd;
      string QzWMGcaAce;
      string ZyaoUmVliK;
      string wBJSCrsELC;
      string hLDYRkiadO;
      string JJimcMMXek;
      string ynwihOoDKN;
      string jVoZxOuBWi;
      if(YhLHEIfQAl == fGFjXIqxDi){DuIUBlRWop = true;}
      else if(fGFjXIqxDi == YhLHEIfQAl){tTVTpMoOLK = true;}
      if(sjXZIEqKEl == kGnjqkKMSD){fUYZEsOTqD = true;}
      else if(kGnjqkKMSD == sjXZIEqKEl){GiKaWLVtLi = true;}
      if(HLiwjcqkDt == LmNAXUaftd){ueQPdmUJpO = true;}
      else if(LmNAXUaftd == HLiwjcqkDt){TnThcZBQST = true;}
      if(QkSlTrZsCN == QzWMGcaAce){QLAwhCfbhu = true;}
      else if(QzWMGcaAce == QkSlTrZsCN){pdAUUXiSwB = true;}
      if(BbipVysLBC == ZyaoUmVliK){wSNUhOwOVD = true;}
      else if(ZyaoUmVliK == BbipVysLBC){zlUyDobJyQ = true;}
      if(GznnNNyOWb == wBJSCrsELC){zaKTmXerpN = true;}
      else if(wBJSCrsELC == GznnNNyOWb){RraLsGznqP = true;}
      if(RtNGUHRlGQ == hLDYRkiadO){RIfaYUUcrB = true;}
      else if(hLDYRkiadO == RtNGUHRlGQ){KYurTbhoGz = true;}
      if(exHpkebral == JJimcMMXek){NmluHcRuhP = true;}
      if(jfjAnRwdYF == ynwihOoDKN){qFmlOoAgIJ = true;}
      if(sKWawGOcVN == jVoZxOuBWi){XmkUcmlmuN = true;}
      while(JJimcMMXek == exHpkebral){bCyJigSVem = true;}
      while(ynwihOoDKN == ynwihOoDKN){yrSBCyCnaO = true;}
      while(jVoZxOuBWi == jVoZxOuBWi){Xubfqapbea = true;}
      if(DuIUBlRWop == true){DuIUBlRWop = false;}
      if(fUYZEsOTqD == true){fUYZEsOTqD = false;}
      if(ueQPdmUJpO == true){ueQPdmUJpO = false;}
      if(QLAwhCfbhu == true){QLAwhCfbhu = false;}
      if(wSNUhOwOVD == true){wSNUhOwOVD = false;}
      if(zaKTmXerpN == true){zaKTmXerpN = false;}
      if(RIfaYUUcrB == true){RIfaYUUcrB = false;}
      if(NmluHcRuhP == true){NmluHcRuhP = false;}
      if(qFmlOoAgIJ == true){qFmlOoAgIJ = false;}
      if(XmkUcmlmuN == true){XmkUcmlmuN = false;}
      if(tTVTpMoOLK == true){tTVTpMoOLK = false;}
      if(GiKaWLVtLi == true){GiKaWLVtLi = false;}
      if(TnThcZBQST == true){TnThcZBQST = false;}
      if(pdAUUXiSwB == true){pdAUUXiSwB = false;}
      if(zlUyDobJyQ == true){zlUyDobJyQ = false;}
      if(RraLsGznqP == true){RraLsGznqP = false;}
      if(KYurTbhoGz == true){KYurTbhoGz = false;}
      if(bCyJigSVem == true){bCyJigSVem = false;}
      if(yrSBCyCnaO == true){yrSBCyCnaO = false;}
      if(Xubfqapbea == true){Xubfqapbea = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LSOFNXHCXR
{ 
  void KrCsbGmSZU()
  { 
      bool MaauOfxttm = false;
      bool gRFanGVlVX = false;
      bool alLgPPRsfG = false;
      bool KKSuuYPjWI = false;
      bool ZhpeQcFFpP = false;
      bool szcFpZOtfy = false;
      bool NDOmcWzJeO = false;
      bool pkPNyrwgQm = false;
      bool KLCQMLhNEV = false;
      bool KVEYtWTCCs = false;
      bool cuaaYIjOHx = false;
      bool chdrnCByaR = false;
      bool olFIEUgpTa = false;
      bool YsdWJOkaqE = false;
      bool aSFGsbrLjr = false;
      bool VUbkEqpSWE = false;
      bool bsufYiEGiZ = false;
      bool QZGWpVfyPc = false;
      bool cUGbFDRrRS = false;
      bool XSEmchXZhJ = false;
      string YiVWZyTLTJ;
      string pmrFfFEDxO;
      string bBGwJMOOEf;
      string MbgIXiMjoc;
      string zVHBEOSjXn;
      string PESsuXbYMN;
      string iFHmwOGihx;
      string sTOTOGtFru;
      string sepPPIInNz;
      string GmWastNTEo;
      string HjBZuupyen;
      string dWgRUEOrWn;
      string GRViqGxmaT;
      string rZACmJdPTU;
      string RJJYViFFGo;
      string eUxPipOSio;
      string yCXKuDWedK;
      string azXCVwIwMI;
      string AqfknWcjSx;
      string uKgeVPlydR;
      if(YiVWZyTLTJ == HjBZuupyen){MaauOfxttm = true;}
      else if(HjBZuupyen == YiVWZyTLTJ){cuaaYIjOHx = true;}
      if(pmrFfFEDxO == dWgRUEOrWn){gRFanGVlVX = true;}
      else if(dWgRUEOrWn == pmrFfFEDxO){chdrnCByaR = true;}
      if(bBGwJMOOEf == GRViqGxmaT){alLgPPRsfG = true;}
      else if(GRViqGxmaT == bBGwJMOOEf){olFIEUgpTa = true;}
      if(MbgIXiMjoc == rZACmJdPTU){KKSuuYPjWI = true;}
      else if(rZACmJdPTU == MbgIXiMjoc){YsdWJOkaqE = true;}
      if(zVHBEOSjXn == RJJYViFFGo){ZhpeQcFFpP = true;}
      else if(RJJYViFFGo == zVHBEOSjXn){aSFGsbrLjr = true;}
      if(PESsuXbYMN == eUxPipOSio){szcFpZOtfy = true;}
      else if(eUxPipOSio == PESsuXbYMN){VUbkEqpSWE = true;}
      if(iFHmwOGihx == yCXKuDWedK){NDOmcWzJeO = true;}
      else if(yCXKuDWedK == iFHmwOGihx){bsufYiEGiZ = true;}
      if(sTOTOGtFru == azXCVwIwMI){pkPNyrwgQm = true;}
      if(sepPPIInNz == AqfknWcjSx){KLCQMLhNEV = true;}
      if(GmWastNTEo == uKgeVPlydR){KVEYtWTCCs = true;}
      while(azXCVwIwMI == sTOTOGtFru){QZGWpVfyPc = true;}
      while(AqfknWcjSx == AqfknWcjSx){cUGbFDRrRS = true;}
      while(uKgeVPlydR == uKgeVPlydR){XSEmchXZhJ = true;}
      if(MaauOfxttm == true){MaauOfxttm = false;}
      if(gRFanGVlVX == true){gRFanGVlVX = false;}
      if(alLgPPRsfG == true){alLgPPRsfG = false;}
      if(KKSuuYPjWI == true){KKSuuYPjWI = false;}
      if(ZhpeQcFFpP == true){ZhpeQcFFpP = false;}
      if(szcFpZOtfy == true){szcFpZOtfy = false;}
      if(NDOmcWzJeO == true){NDOmcWzJeO = false;}
      if(pkPNyrwgQm == true){pkPNyrwgQm = false;}
      if(KLCQMLhNEV == true){KLCQMLhNEV = false;}
      if(KVEYtWTCCs == true){KVEYtWTCCs = false;}
      if(cuaaYIjOHx == true){cuaaYIjOHx = false;}
      if(chdrnCByaR == true){chdrnCByaR = false;}
      if(olFIEUgpTa == true){olFIEUgpTa = false;}
      if(YsdWJOkaqE == true){YsdWJOkaqE = false;}
      if(aSFGsbrLjr == true){aSFGsbrLjr = false;}
      if(VUbkEqpSWE == true){VUbkEqpSWE = false;}
      if(bsufYiEGiZ == true){bsufYiEGiZ = false;}
      if(QZGWpVfyPc == true){QZGWpVfyPc = false;}
      if(cUGbFDRrRS == true){cUGbFDRrRS = false;}
      if(XSEmchXZhJ == true){XSEmchXZhJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FGFJJWYARQ
{ 
  void QknXMYBuCE()
  { 
      bool gQABikdIHE = false;
      bool QUuVteIFKu = false;
      bool jyCyBzFeHN = false;
      bool zZTszgYrSk = false;
      bool BcnjlwzcEB = false;
      bool prHtlhOBzi = false;
      bool fyAsEqBBVM = false;
      bool GQlYsGKRME = false;
      bool zbaFXGRZHQ = false;
      bool SXrjEDbtet = false;
      bool uxGquYFiAe = false;
      bool XxntiDWuVy = false;
      bool YbtFtiYCUo = false;
      bool dPlRJBUjKy = false;
      bool BiYbrUixAB = false;
      bool ggXgVTDBxM = false;
      bool ACOLBIKVkg = false;
      bool hmkXxoDRMY = false;
      bool zaBTIxqEwo = false;
      bool yTlxyyUNaj = false;
      string oOSdRdezhI;
      string lcLRjMluWR;
      string FaIrBEAOmo;
      string dqrIMqQtxK;
      string KoRLylFYDw;
      string XNzAdOGacN;
      string cPMrdtjDFs;
      string wrxMGPIwtO;
      string yaKqRQrrdx;
      string koQjgtcYgD;
      string BCTKzfPmsN;
      string FJgOOCEUaN;
      string CrYHHGaNfL;
      string mjPpMuHAql;
      string BFzPraICUd;
      string oHxTBJdLCs;
      string ECHgaVtqBT;
      string uEdoMxrIPf;
      string yYsBWfluoC;
      string RuyygyxSoV;
      if(oOSdRdezhI == BCTKzfPmsN){gQABikdIHE = true;}
      else if(BCTKzfPmsN == oOSdRdezhI){uxGquYFiAe = true;}
      if(lcLRjMluWR == FJgOOCEUaN){QUuVteIFKu = true;}
      else if(FJgOOCEUaN == lcLRjMluWR){XxntiDWuVy = true;}
      if(FaIrBEAOmo == CrYHHGaNfL){jyCyBzFeHN = true;}
      else if(CrYHHGaNfL == FaIrBEAOmo){YbtFtiYCUo = true;}
      if(dqrIMqQtxK == mjPpMuHAql){zZTszgYrSk = true;}
      else if(mjPpMuHAql == dqrIMqQtxK){dPlRJBUjKy = true;}
      if(KoRLylFYDw == BFzPraICUd){BcnjlwzcEB = true;}
      else if(BFzPraICUd == KoRLylFYDw){BiYbrUixAB = true;}
      if(XNzAdOGacN == oHxTBJdLCs){prHtlhOBzi = true;}
      else if(oHxTBJdLCs == XNzAdOGacN){ggXgVTDBxM = true;}
      if(cPMrdtjDFs == ECHgaVtqBT){fyAsEqBBVM = true;}
      else if(ECHgaVtqBT == cPMrdtjDFs){ACOLBIKVkg = true;}
      if(wrxMGPIwtO == uEdoMxrIPf){GQlYsGKRME = true;}
      if(yaKqRQrrdx == yYsBWfluoC){zbaFXGRZHQ = true;}
      if(koQjgtcYgD == RuyygyxSoV){SXrjEDbtet = true;}
      while(uEdoMxrIPf == wrxMGPIwtO){hmkXxoDRMY = true;}
      while(yYsBWfluoC == yYsBWfluoC){zaBTIxqEwo = true;}
      while(RuyygyxSoV == RuyygyxSoV){yTlxyyUNaj = true;}
      if(gQABikdIHE == true){gQABikdIHE = false;}
      if(QUuVteIFKu == true){QUuVteIFKu = false;}
      if(jyCyBzFeHN == true){jyCyBzFeHN = false;}
      if(zZTszgYrSk == true){zZTszgYrSk = false;}
      if(BcnjlwzcEB == true){BcnjlwzcEB = false;}
      if(prHtlhOBzi == true){prHtlhOBzi = false;}
      if(fyAsEqBBVM == true){fyAsEqBBVM = false;}
      if(GQlYsGKRME == true){GQlYsGKRME = false;}
      if(zbaFXGRZHQ == true){zbaFXGRZHQ = false;}
      if(SXrjEDbtet == true){SXrjEDbtet = false;}
      if(uxGquYFiAe == true){uxGquYFiAe = false;}
      if(XxntiDWuVy == true){XxntiDWuVy = false;}
      if(YbtFtiYCUo == true){YbtFtiYCUo = false;}
      if(dPlRJBUjKy == true){dPlRJBUjKy = false;}
      if(BiYbrUixAB == true){BiYbrUixAB = false;}
      if(ggXgVTDBxM == true){ggXgVTDBxM = false;}
      if(ACOLBIKVkg == true){ACOLBIKVkg = false;}
      if(hmkXxoDRMY == true){hmkXxoDRMY = false;}
      if(zaBTIxqEwo == true){zaBTIxqEwo = false;}
      if(yTlxyyUNaj == true){yTlxyyUNaj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YZGKUVXLVA
{ 
  void aBLtWeYfTW()
  { 
      bool uoHJRuEpIP = false;
      bool eCwYyiBppg = false;
      bool SwYXorkVqP = false;
      bool XDUEIpKXMT = false;
      bool MmXQnAiGyL = false;
      bool kLwCNzPiUZ = false;
      bool JhysrbgqPI = false;
      bool XdcVUeXIio = false;
      bool IeXedMGSqJ = false;
      bool abDmJdoeXM = false;
      bool SqpomHhZhh = false;
      bool aHzfQeKSOf = false;
      bool VNzGgwZpMp = false;
      bool psjWVQnhrW = false;
      bool dfjJhTbXVe = false;
      bool TVPrqErjIq = false;
      bool grNqqgWcJz = false;
      bool pbarCodpXN = false;
      bool TCTgWUdqfQ = false;
      bool YEZAokRsRB = false;
      string DhgeMyeFfO;
      string BhWyRDyuNm;
      string wghyGGDqgf;
      string xtXODtFCIT;
      string YpNDqOCscB;
      string OGiwsWAiXF;
      string tmYARGKIuX;
      string xulsNGSeGr;
      string EKNeQqLaUf;
      string tWdCZlRMlj;
      string EtxFETTohm;
      string DVbeBjYsjZ;
      string uimOcZRiTb;
      string oGrVnJMIax;
      string XVshcYuyBo;
      string MXcLoUHhcd;
      string BjUFywcFqq;
      string WQSlCYpFlE;
      string eShePQdggS;
      string CeewumBmjg;
      if(DhgeMyeFfO == EtxFETTohm){uoHJRuEpIP = true;}
      else if(EtxFETTohm == DhgeMyeFfO){SqpomHhZhh = true;}
      if(BhWyRDyuNm == DVbeBjYsjZ){eCwYyiBppg = true;}
      else if(DVbeBjYsjZ == BhWyRDyuNm){aHzfQeKSOf = true;}
      if(wghyGGDqgf == uimOcZRiTb){SwYXorkVqP = true;}
      else if(uimOcZRiTb == wghyGGDqgf){VNzGgwZpMp = true;}
      if(xtXODtFCIT == oGrVnJMIax){XDUEIpKXMT = true;}
      else if(oGrVnJMIax == xtXODtFCIT){psjWVQnhrW = true;}
      if(YpNDqOCscB == XVshcYuyBo){MmXQnAiGyL = true;}
      else if(XVshcYuyBo == YpNDqOCscB){dfjJhTbXVe = true;}
      if(OGiwsWAiXF == MXcLoUHhcd){kLwCNzPiUZ = true;}
      else if(MXcLoUHhcd == OGiwsWAiXF){TVPrqErjIq = true;}
      if(tmYARGKIuX == BjUFywcFqq){JhysrbgqPI = true;}
      else if(BjUFywcFqq == tmYARGKIuX){grNqqgWcJz = true;}
      if(xulsNGSeGr == WQSlCYpFlE){XdcVUeXIio = true;}
      if(EKNeQqLaUf == eShePQdggS){IeXedMGSqJ = true;}
      if(tWdCZlRMlj == CeewumBmjg){abDmJdoeXM = true;}
      while(WQSlCYpFlE == xulsNGSeGr){pbarCodpXN = true;}
      while(eShePQdggS == eShePQdggS){TCTgWUdqfQ = true;}
      while(CeewumBmjg == CeewumBmjg){YEZAokRsRB = true;}
      if(uoHJRuEpIP == true){uoHJRuEpIP = false;}
      if(eCwYyiBppg == true){eCwYyiBppg = false;}
      if(SwYXorkVqP == true){SwYXorkVqP = false;}
      if(XDUEIpKXMT == true){XDUEIpKXMT = false;}
      if(MmXQnAiGyL == true){MmXQnAiGyL = false;}
      if(kLwCNzPiUZ == true){kLwCNzPiUZ = false;}
      if(JhysrbgqPI == true){JhysrbgqPI = false;}
      if(XdcVUeXIio == true){XdcVUeXIio = false;}
      if(IeXedMGSqJ == true){IeXedMGSqJ = false;}
      if(abDmJdoeXM == true){abDmJdoeXM = false;}
      if(SqpomHhZhh == true){SqpomHhZhh = false;}
      if(aHzfQeKSOf == true){aHzfQeKSOf = false;}
      if(VNzGgwZpMp == true){VNzGgwZpMp = false;}
      if(psjWVQnhrW == true){psjWVQnhrW = false;}
      if(dfjJhTbXVe == true){dfjJhTbXVe = false;}
      if(TVPrqErjIq == true){TVPrqErjIq = false;}
      if(grNqqgWcJz == true){grNqqgWcJz = false;}
      if(pbarCodpXN == true){pbarCodpXN = false;}
      if(TCTgWUdqfQ == true){TCTgWUdqfQ = false;}
      if(YEZAokRsRB == true){YEZAokRsRB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LIXDYKYXRD
{ 
  void DxqekNjHnZ()
  { 
      bool THEMqGhReW = false;
      bool ouhQjMqLbe = false;
      bool dVRLeJDwjV = false;
      bool SSrSbSGLRg = false;
      bool gnNtcDATOa = false;
      bool VlbEAypsgh = false;
      bool ClhIDVDOoQ = false;
      bool FesuypHWhb = false;
      bool IjgIzgqnRc = false;
      bool cMejduGFpH = false;
      bool AkdyUpsfFK = false;
      bool qSKFZSEYJW = false;
      bool ePxLnrAJRD = false;
      bool iBgaaWFimI = false;
      bool mKhGkJBYJK = false;
      bool pgwSbknRJc = false;
      bool pjOBboljdC = false;
      bool NVkAjUgpae = false;
      bool qbzGNBFNzp = false;
      bool imYKSDufeu = false;
      string DrdsLWxFzZ;
      string tmBywtcKpX;
      string zEkNyGHTOl;
      string BeddqLwnAG;
      string JZkAlODzca;
      string HqyLMyGQtF;
      string PcqNXPJXGj;
      string ynKjOZSjbj;
      string CHpmdKfRQs;
      string FfstqghXcW;
      string EesrcSfNxl;
      string eVnKCHlEiN;
      string irGMeSuPsh;
      string ruHFfXOyJL;
      string dSTdOiqiqj;
      string tcoOmMXfNK;
      string XPUPtjOide;
      string rnuXjQXbxw;
      string aLYJmOFsRS;
      string zijymNxEje;
      if(DrdsLWxFzZ == EesrcSfNxl){THEMqGhReW = true;}
      else if(EesrcSfNxl == DrdsLWxFzZ){AkdyUpsfFK = true;}
      if(tmBywtcKpX == eVnKCHlEiN){ouhQjMqLbe = true;}
      else if(eVnKCHlEiN == tmBywtcKpX){qSKFZSEYJW = true;}
      if(zEkNyGHTOl == irGMeSuPsh){dVRLeJDwjV = true;}
      else if(irGMeSuPsh == zEkNyGHTOl){ePxLnrAJRD = true;}
      if(BeddqLwnAG == ruHFfXOyJL){SSrSbSGLRg = true;}
      else if(ruHFfXOyJL == BeddqLwnAG){iBgaaWFimI = true;}
      if(JZkAlODzca == dSTdOiqiqj){gnNtcDATOa = true;}
      else if(dSTdOiqiqj == JZkAlODzca){mKhGkJBYJK = true;}
      if(HqyLMyGQtF == tcoOmMXfNK){VlbEAypsgh = true;}
      else if(tcoOmMXfNK == HqyLMyGQtF){pgwSbknRJc = true;}
      if(PcqNXPJXGj == XPUPtjOide){ClhIDVDOoQ = true;}
      else if(XPUPtjOide == PcqNXPJXGj){pjOBboljdC = true;}
      if(ynKjOZSjbj == rnuXjQXbxw){FesuypHWhb = true;}
      if(CHpmdKfRQs == aLYJmOFsRS){IjgIzgqnRc = true;}
      if(FfstqghXcW == zijymNxEje){cMejduGFpH = true;}
      while(rnuXjQXbxw == ynKjOZSjbj){NVkAjUgpae = true;}
      while(aLYJmOFsRS == aLYJmOFsRS){qbzGNBFNzp = true;}
      while(zijymNxEje == zijymNxEje){imYKSDufeu = true;}
      if(THEMqGhReW == true){THEMqGhReW = false;}
      if(ouhQjMqLbe == true){ouhQjMqLbe = false;}
      if(dVRLeJDwjV == true){dVRLeJDwjV = false;}
      if(SSrSbSGLRg == true){SSrSbSGLRg = false;}
      if(gnNtcDATOa == true){gnNtcDATOa = false;}
      if(VlbEAypsgh == true){VlbEAypsgh = false;}
      if(ClhIDVDOoQ == true){ClhIDVDOoQ = false;}
      if(FesuypHWhb == true){FesuypHWhb = false;}
      if(IjgIzgqnRc == true){IjgIzgqnRc = false;}
      if(cMejduGFpH == true){cMejduGFpH = false;}
      if(AkdyUpsfFK == true){AkdyUpsfFK = false;}
      if(qSKFZSEYJW == true){qSKFZSEYJW = false;}
      if(ePxLnrAJRD == true){ePxLnrAJRD = false;}
      if(iBgaaWFimI == true){iBgaaWFimI = false;}
      if(mKhGkJBYJK == true){mKhGkJBYJK = false;}
      if(pgwSbknRJc == true){pgwSbknRJc = false;}
      if(pjOBboljdC == true){pjOBboljdC = false;}
      if(NVkAjUgpae == true){NVkAjUgpae = false;}
      if(qbzGNBFNzp == true){qbzGNBFNzp = false;}
      if(imYKSDufeu == true){imYKSDufeu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QUNGHQTPXF
{ 
  void kNGzMTjeYb()
  { 
      bool HYGRWNduBu = false;
      bool ZQYzxLyakx = false;
      bool SfsGSeWMxY = false;
      bool JCwUZoGnLB = false;
      bool juGJOXRnmO = false;
      bool gzHtTbeAyq = false;
      bool pQybkWUDZH = false;
      bool EJPjMWYjqu = false;
      bool NiZhrKuBBx = false;
      bool KGoTLTxeXq = false;
      bool CNOeKWjXgw = false;
      bool CuIDdHpQja = false;
      bool bSCbxyUuXs = false;
      bool ZtMArATuWV = false;
      bool ZXUFuPylpD = false;
      bool cftwPjKmgM = false;
      bool CoznVIhIKd = false;
      bool nusOAEyxgh = false;
      bool zhEUPszPjI = false;
      bool ofNKSkXfil = false;
      string XSMxPTbdRN;
      string ZBNFhYSOJN;
      string YDJnQcPQmF;
      string GcYSkHeqLr;
      string SMCBzqoGkr;
      string QmEmNUJYgm;
      string ELpMHwcuUa;
      string saDQuLAHnZ;
      string mJeuoSYlEQ;
      string xGKYnGVptD;
      string NtkJyoimqB;
      string TBjHztgtWn;
      string WqpEclQeco;
      string pJNkLrXUxz;
      string jluDwuWCYs;
      string ZaSFfWEggM;
      string YEKBLJnyuH;
      string VLxXEkwmHg;
      string ONFiPOqIEo;
      string nqnjaeasql;
      if(XSMxPTbdRN == NtkJyoimqB){HYGRWNduBu = true;}
      else if(NtkJyoimqB == XSMxPTbdRN){CNOeKWjXgw = true;}
      if(ZBNFhYSOJN == TBjHztgtWn){ZQYzxLyakx = true;}
      else if(TBjHztgtWn == ZBNFhYSOJN){CuIDdHpQja = true;}
      if(YDJnQcPQmF == WqpEclQeco){SfsGSeWMxY = true;}
      else if(WqpEclQeco == YDJnQcPQmF){bSCbxyUuXs = true;}
      if(GcYSkHeqLr == pJNkLrXUxz){JCwUZoGnLB = true;}
      else if(pJNkLrXUxz == GcYSkHeqLr){ZtMArATuWV = true;}
      if(SMCBzqoGkr == jluDwuWCYs){juGJOXRnmO = true;}
      else if(jluDwuWCYs == SMCBzqoGkr){ZXUFuPylpD = true;}
      if(QmEmNUJYgm == ZaSFfWEggM){gzHtTbeAyq = true;}
      else if(ZaSFfWEggM == QmEmNUJYgm){cftwPjKmgM = true;}
      if(ELpMHwcuUa == YEKBLJnyuH){pQybkWUDZH = true;}
      else if(YEKBLJnyuH == ELpMHwcuUa){CoznVIhIKd = true;}
      if(saDQuLAHnZ == VLxXEkwmHg){EJPjMWYjqu = true;}
      if(mJeuoSYlEQ == ONFiPOqIEo){NiZhrKuBBx = true;}
      if(xGKYnGVptD == nqnjaeasql){KGoTLTxeXq = true;}
      while(VLxXEkwmHg == saDQuLAHnZ){nusOAEyxgh = true;}
      while(ONFiPOqIEo == ONFiPOqIEo){zhEUPszPjI = true;}
      while(nqnjaeasql == nqnjaeasql){ofNKSkXfil = true;}
      if(HYGRWNduBu == true){HYGRWNduBu = false;}
      if(ZQYzxLyakx == true){ZQYzxLyakx = false;}
      if(SfsGSeWMxY == true){SfsGSeWMxY = false;}
      if(JCwUZoGnLB == true){JCwUZoGnLB = false;}
      if(juGJOXRnmO == true){juGJOXRnmO = false;}
      if(gzHtTbeAyq == true){gzHtTbeAyq = false;}
      if(pQybkWUDZH == true){pQybkWUDZH = false;}
      if(EJPjMWYjqu == true){EJPjMWYjqu = false;}
      if(NiZhrKuBBx == true){NiZhrKuBBx = false;}
      if(KGoTLTxeXq == true){KGoTLTxeXq = false;}
      if(CNOeKWjXgw == true){CNOeKWjXgw = false;}
      if(CuIDdHpQja == true){CuIDdHpQja = false;}
      if(bSCbxyUuXs == true){bSCbxyUuXs = false;}
      if(ZtMArATuWV == true){ZtMArATuWV = false;}
      if(ZXUFuPylpD == true){ZXUFuPylpD = false;}
      if(cftwPjKmgM == true){cftwPjKmgM = false;}
      if(CoznVIhIKd == true){CoznVIhIKd = false;}
      if(nusOAEyxgh == true){nusOAEyxgh = false;}
      if(zhEUPszPjI == true){zhEUPszPjI = false;}
      if(ofNKSkXfil == true){ofNKSkXfil = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PAEYQMLEEW
{ 
  void CNZLKewzNZ()
  { 
      bool jAlyXfDzad = false;
      bool OaPamGshzp = false;
      bool uSsYALjBZG = false;
      bool dJufUFpHuV = false;
      bool kgWxuwICXG = false;
      bool DPXzyQGNxL = false;
      bool wBZTAFejGh = false;
      bool QkmcENwwdD = false;
      bool DgcTDNkwgD = false;
      bool ycKiZiHCwo = false;
      bool RXuppRcutk = false;
      bool UJitIOYGPl = false;
      bool TiDCimaDDV = false;
      bool FsAolelmcl = false;
      bool jpCwiotPaK = false;
      bool KBmKVhUsGD = false;
      bool pYrXeaJtHc = false;
      bool clcdBcfGTt = false;
      bool rKcrKEGpSD = false;
      bool JrfZuLydRH = false;
      string fMIkmwGYyl;
      string jHfateWwkx;
      string BnMtmcykkd;
      string LhfWLsiKnE;
      string kSsFhdbAnO;
      string wkIJSrzNyJ;
      string xUmTaBbJOA;
      string wLIgMitGMI;
      string ByxWhZZVDT;
      string tVMeGQeYef;
      string CRkmSjcaxi;
      string xrApHyWPlA;
      string aFFuGZzcZu;
      string DUbfGjNJDy;
      string wzYUVBUVWH;
      string sPmdFwLogr;
      string jAABebQUiX;
      string jMMmDbgkYF;
      string FcGSCngxaJ;
      string XYcLNwyWos;
      if(fMIkmwGYyl == CRkmSjcaxi){jAlyXfDzad = true;}
      else if(CRkmSjcaxi == fMIkmwGYyl){RXuppRcutk = true;}
      if(jHfateWwkx == xrApHyWPlA){OaPamGshzp = true;}
      else if(xrApHyWPlA == jHfateWwkx){UJitIOYGPl = true;}
      if(BnMtmcykkd == aFFuGZzcZu){uSsYALjBZG = true;}
      else if(aFFuGZzcZu == BnMtmcykkd){TiDCimaDDV = true;}
      if(LhfWLsiKnE == DUbfGjNJDy){dJufUFpHuV = true;}
      else if(DUbfGjNJDy == LhfWLsiKnE){FsAolelmcl = true;}
      if(kSsFhdbAnO == wzYUVBUVWH){kgWxuwICXG = true;}
      else if(wzYUVBUVWH == kSsFhdbAnO){jpCwiotPaK = true;}
      if(wkIJSrzNyJ == sPmdFwLogr){DPXzyQGNxL = true;}
      else if(sPmdFwLogr == wkIJSrzNyJ){KBmKVhUsGD = true;}
      if(xUmTaBbJOA == jAABebQUiX){wBZTAFejGh = true;}
      else if(jAABebQUiX == xUmTaBbJOA){pYrXeaJtHc = true;}
      if(wLIgMitGMI == jMMmDbgkYF){QkmcENwwdD = true;}
      if(ByxWhZZVDT == FcGSCngxaJ){DgcTDNkwgD = true;}
      if(tVMeGQeYef == XYcLNwyWos){ycKiZiHCwo = true;}
      while(jMMmDbgkYF == wLIgMitGMI){clcdBcfGTt = true;}
      while(FcGSCngxaJ == FcGSCngxaJ){rKcrKEGpSD = true;}
      while(XYcLNwyWos == XYcLNwyWos){JrfZuLydRH = true;}
      if(jAlyXfDzad == true){jAlyXfDzad = false;}
      if(OaPamGshzp == true){OaPamGshzp = false;}
      if(uSsYALjBZG == true){uSsYALjBZG = false;}
      if(dJufUFpHuV == true){dJufUFpHuV = false;}
      if(kgWxuwICXG == true){kgWxuwICXG = false;}
      if(DPXzyQGNxL == true){DPXzyQGNxL = false;}
      if(wBZTAFejGh == true){wBZTAFejGh = false;}
      if(QkmcENwwdD == true){QkmcENwwdD = false;}
      if(DgcTDNkwgD == true){DgcTDNkwgD = false;}
      if(ycKiZiHCwo == true){ycKiZiHCwo = false;}
      if(RXuppRcutk == true){RXuppRcutk = false;}
      if(UJitIOYGPl == true){UJitIOYGPl = false;}
      if(TiDCimaDDV == true){TiDCimaDDV = false;}
      if(FsAolelmcl == true){FsAolelmcl = false;}
      if(jpCwiotPaK == true){jpCwiotPaK = false;}
      if(KBmKVhUsGD == true){KBmKVhUsGD = false;}
      if(pYrXeaJtHc == true){pYrXeaJtHc = false;}
      if(clcdBcfGTt == true){clcdBcfGTt = false;}
      if(rKcrKEGpSD == true){rKcrKEGpSD = false;}
      if(JrfZuLydRH == true){JrfZuLydRH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YNJIMCCUIR
{ 
  void MppxLrglDD()
  { 
      bool AmdUeJmeMX = false;
      bool odDDGkYNTB = false;
      bool bUTyPQrkbY = false;
      bool xbdojfldfx = false;
      bool moxCUYirpo = false;
      bool PIZpPfTeEN = false;
      bool QBCnaMANTZ = false;
      bool ytgWNlXRdq = false;
      bool hzumJoBnZE = false;
      bool MGZRfkmJaH = false;
      bool YVYPpUaDbd = false;
      bool HCBMfQGOdl = false;
      bool LLmONrUPpb = false;
      bool yNpWSLaewn = false;
      bool NxLwwJFMlO = false;
      bool eXenwMhJBy = false;
      bool AKNAetLetU = false;
      bool paSpXNzqSE = false;
      bool mIdawaiBZF = false;
      bool cQdynCjLQC = false;
      string xbrZQoTWPU;
      string UuFCHZnOSF;
      string tlcIVFlGUE;
      string ZpDdxtdUTM;
      string gaXsjwPKRW;
      string XEZDoLyjEK;
      string CMCKterbFM;
      string uTuFYSNiXZ;
      string oboyfVPAnk;
      string utQIKUYMzF;
      string DmEFbEJUeV;
      string xPMdUaXEeY;
      string OrmUQBCluD;
      string tBjRVCWiWb;
      string LuUSmfhlmi;
      string xUgZqLzjnZ;
      string BXYaauetIS;
      string DngCNSWUgk;
      string TPyLGZTJaJ;
      string CluwZnVtqq;
      if(xbrZQoTWPU == DmEFbEJUeV){AmdUeJmeMX = true;}
      else if(DmEFbEJUeV == xbrZQoTWPU){YVYPpUaDbd = true;}
      if(UuFCHZnOSF == xPMdUaXEeY){odDDGkYNTB = true;}
      else if(xPMdUaXEeY == UuFCHZnOSF){HCBMfQGOdl = true;}
      if(tlcIVFlGUE == OrmUQBCluD){bUTyPQrkbY = true;}
      else if(OrmUQBCluD == tlcIVFlGUE){LLmONrUPpb = true;}
      if(ZpDdxtdUTM == tBjRVCWiWb){xbdojfldfx = true;}
      else if(tBjRVCWiWb == ZpDdxtdUTM){yNpWSLaewn = true;}
      if(gaXsjwPKRW == LuUSmfhlmi){moxCUYirpo = true;}
      else if(LuUSmfhlmi == gaXsjwPKRW){NxLwwJFMlO = true;}
      if(XEZDoLyjEK == xUgZqLzjnZ){PIZpPfTeEN = true;}
      else if(xUgZqLzjnZ == XEZDoLyjEK){eXenwMhJBy = true;}
      if(CMCKterbFM == BXYaauetIS){QBCnaMANTZ = true;}
      else if(BXYaauetIS == CMCKterbFM){AKNAetLetU = true;}
      if(uTuFYSNiXZ == DngCNSWUgk){ytgWNlXRdq = true;}
      if(oboyfVPAnk == TPyLGZTJaJ){hzumJoBnZE = true;}
      if(utQIKUYMzF == CluwZnVtqq){MGZRfkmJaH = true;}
      while(DngCNSWUgk == uTuFYSNiXZ){paSpXNzqSE = true;}
      while(TPyLGZTJaJ == TPyLGZTJaJ){mIdawaiBZF = true;}
      while(CluwZnVtqq == CluwZnVtqq){cQdynCjLQC = true;}
      if(AmdUeJmeMX == true){AmdUeJmeMX = false;}
      if(odDDGkYNTB == true){odDDGkYNTB = false;}
      if(bUTyPQrkbY == true){bUTyPQrkbY = false;}
      if(xbdojfldfx == true){xbdojfldfx = false;}
      if(moxCUYirpo == true){moxCUYirpo = false;}
      if(PIZpPfTeEN == true){PIZpPfTeEN = false;}
      if(QBCnaMANTZ == true){QBCnaMANTZ = false;}
      if(ytgWNlXRdq == true){ytgWNlXRdq = false;}
      if(hzumJoBnZE == true){hzumJoBnZE = false;}
      if(MGZRfkmJaH == true){MGZRfkmJaH = false;}
      if(YVYPpUaDbd == true){YVYPpUaDbd = false;}
      if(HCBMfQGOdl == true){HCBMfQGOdl = false;}
      if(LLmONrUPpb == true){LLmONrUPpb = false;}
      if(yNpWSLaewn == true){yNpWSLaewn = false;}
      if(NxLwwJFMlO == true){NxLwwJFMlO = false;}
      if(eXenwMhJBy == true){eXenwMhJBy = false;}
      if(AKNAetLetU == true){AKNAetLetU = false;}
      if(paSpXNzqSE == true){paSpXNzqSE = false;}
      if(mIdawaiBZF == true){mIdawaiBZF = false;}
      if(cQdynCjLQC == true){cQdynCjLQC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MAROUBKYNY
{ 
  void QCSLxVOfyk()
  { 
      bool FaNIVFLzKc = false;
      bool ZAxkwqNctB = false;
      bool wesMPlWTOE = false;
      bool tnXDSswgga = false;
      bool uArwPKneTD = false;
      bool rzzuDZhBsj = false;
      bool hqjGikcsSC = false;
      bool ugwTFwmwwE = false;
      bool yqzLLtobAr = false;
      bool LtmxrEPlNe = false;
      bool gtMRILDeoG = false;
      bool mkFnkpqYtn = false;
      bool uBsZpAtzaB = false;
      bool fBiQXDehCA = false;
      bool qJmtFaozLB = false;
      bool MqqYifBmaI = false;
      bool QDEmakBnPh = false;
      bool LJVtxkOzTO = false;
      bool KtnYooKKhr = false;
      bool BqGtimFzST = false;
      string HiBywwmWbm;
      string lbklQjOnOF;
      string JZODpNtYrt;
      string eTEtiMiXYJ;
      string WrzAyhlEae;
      string TqhicgUxHh;
      string CNehIRXqCE;
      string AqOEiQaOFH;
      string apyMZmlURC;
      string rlPpHJNRqf;
      string KuBWecbcAL;
      string GFrzHMPRHV;
      string fCZALOaYxZ;
      string DJXZxfnbgx;
      string KIUFwmsPit;
      string swJVAHUBKQ;
      string CDfaIwgSCa;
      string YnjKKWhTDY;
      string UKoCfHzCYp;
      string LlQQuGcswz;
      if(HiBywwmWbm == KuBWecbcAL){FaNIVFLzKc = true;}
      else if(KuBWecbcAL == HiBywwmWbm){gtMRILDeoG = true;}
      if(lbklQjOnOF == GFrzHMPRHV){ZAxkwqNctB = true;}
      else if(GFrzHMPRHV == lbklQjOnOF){mkFnkpqYtn = true;}
      if(JZODpNtYrt == fCZALOaYxZ){wesMPlWTOE = true;}
      else if(fCZALOaYxZ == JZODpNtYrt){uBsZpAtzaB = true;}
      if(eTEtiMiXYJ == DJXZxfnbgx){tnXDSswgga = true;}
      else if(DJXZxfnbgx == eTEtiMiXYJ){fBiQXDehCA = true;}
      if(WrzAyhlEae == KIUFwmsPit){uArwPKneTD = true;}
      else if(KIUFwmsPit == WrzAyhlEae){qJmtFaozLB = true;}
      if(TqhicgUxHh == swJVAHUBKQ){rzzuDZhBsj = true;}
      else if(swJVAHUBKQ == TqhicgUxHh){MqqYifBmaI = true;}
      if(CNehIRXqCE == CDfaIwgSCa){hqjGikcsSC = true;}
      else if(CDfaIwgSCa == CNehIRXqCE){QDEmakBnPh = true;}
      if(AqOEiQaOFH == YnjKKWhTDY){ugwTFwmwwE = true;}
      if(apyMZmlURC == UKoCfHzCYp){yqzLLtobAr = true;}
      if(rlPpHJNRqf == LlQQuGcswz){LtmxrEPlNe = true;}
      while(YnjKKWhTDY == AqOEiQaOFH){LJVtxkOzTO = true;}
      while(UKoCfHzCYp == UKoCfHzCYp){KtnYooKKhr = true;}
      while(LlQQuGcswz == LlQQuGcswz){BqGtimFzST = true;}
      if(FaNIVFLzKc == true){FaNIVFLzKc = false;}
      if(ZAxkwqNctB == true){ZAxkwqNctB = false;}
      if(wesMPlWTOE == true){wesMPlWTOE = false;}
      if(tnXDSswgga == true){tnXDSswgga = false;}
      if(uArwPKneTD == true){uArwPKneTD = false;}
      if(rzzuDZhBsj == true){rzzuDZhBsj = false;}
      if(hqjGikcsSC == true){hqjGikcsSC = false;}
      if(ugwTFwmwwE == true){ugwTFwmwwE = false;}
      if(yqzLLtobAr == true){yqzLLtobAr = false;}
      if(LtmxrEPlNe == true){LtmxrEPlNe = false;}
      if(gtMRILDeoG == true){gtMRILDeoG = false;}
      if(mkFnkpqYtn == true){mkFnkpqYtn = false;}
      if(uBsZpAtzaB == true){uBsZpAtzaB = false;}
      if(fBiQXDehCA == true){fBiQXDehCA = false;}
      if(qJmtFaozLB == true){qJmtFaozLB = false;}
      if(MqqYifBmaI == true){MqqYifBmaI = false;}
      if(QDEmakBnPh == true){QDEmakBnPh = false;}
      if(LJVtxkOzTO == true){LJVtxkOzTO = false;}
      if(KtnYooKKhr == true){KtnYooKKhr = false;}
      if(BqGtimFzST == true){BqGtimFzST = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GPXGJLWNFG
{ 
  void oodMCQTGkf()
  { 
      bool KTwhcWYulZ = false;
      bool kYmEQPcion = false;
      bool egWxaAFBCz = false;
      bool rGxESkTVIp = false;
      bool iAEUdCZTgI = false;
      bool fELUtdxzHb = false;
      bool QXrlgebike = false;
      bool uHazEdcCTY = false;
      bool ZlUtTntbKo = false;
      bool nUNfYLHRmz = false;
      bool aqhBMXdMPP = false;
      bool BtehBeDySs = false;
      bool jzYwqZstZs = false;
      bool WKOUyiDSgu = false;
      bool JhMtnspeet = false;
      bool IJztOaSlKm = false;
      bool eilVspfKjS = false;
      bool bjQWiBldIx = false;
      bool jNPfkMLUFW = false;
      bool PQItQaupBx = false;
      string bIlhCBrhOW;
      string HdjabARmNh;
      string LOZdArkCiX;
      string OPtkGYJMDA;
      string dnsyknrNzD;
      string MEKWANhIhJ;
      string PpyqHriyUD;
      string qFiNEWUarM;
      string AWDfLKEWkr;
      string pAetjnpCqs;
      string EssuMGVxSd;
      string sHPUJslSfx;
      string mLNbMpancP;
      string fSjzHGdHTE;
      string oHEGzikJkq;
      string CXWsFrfQiV;
      string fgTDxaXQMC;
      string OeyaHfHGcd;
      string FQDkNIuptO;
      string aNYxXImBJY;
      if(bIlhCBrhOW == EssuMGVxSd){KTwhcWYulZ = true;}
      else if(EssuMGVxSd == bIlhCBrhOW){aqhBMXdMPP = true;}
      if(HdjabARmNh == sHPUJslSfx){kYmEQPcion = true;}
      else if(sHPUJslSfx == HdjabARmNh){BtehBeDySs = true;}
      if(LOZdArkCiX == mLNbMpancP){egWxaAFBCz = true;}
      else if(mLNbMpancP == LOZdArkCiX){jzYwqZstZs = true;}
      if(OPtkGYJMDA == fSjzHGdHTE){rGxESkTVIp = true;}
      else if(fSjzHGdHTE == OPtkGYJMDA){WKOUyiDSgu = true;}
      if(dnsyknrNzD == oHEGzikJkq){iAEUdCZTgI = true;}
      else if(oHEGzikJkq == dnsyknrNzD){JhMtnspeet = true;}
      if(MEKWANhIhJ == CXWsFrfQiV){fELUtdxzHb = true;}
      else if(CXWsFrfQiV == MEKWANhIhJ){IJztOaSlKm = true;}
      if(PpyqHriyUD == fgTDxaXQMC){QXrlgebike = true;}
      else if(fgTDxaXQMC == PpyqHriyUD){eilVspfKjS = true;}
      if(qFiNEWUarM == OeyaHfHGcd){uHazEdcCTY = true;}
      if(AWDfLKEWkr == FQDkNIuptO){ZlUtTntbKo = true;}
      if(pAetjnpCqs == aNYxXImBJY){nUNfYLHRmz = true;}
      while(OeyaHfHGcd == qFiNEWUarM){bjQWiBldIx = true;}
      while(FQDkNIuptO == FQDkNIuptO){jNPfkMLUFW = true;}
      while(aNYxXImBJY == aNYxXImBJY){PQItQaupBx = true;}
      if(KTwhcWYulZ == true){KTwhcWYulZ = false;}
      if(kYmEQPcion == true){kYmEQPcion = false;}
      if(egWxaAFBCz == true){egWxaAFBCz = false;}
      if(rGxESkTVIp == true){rGxESkTVIp = false;}
      if(iAEUdCZTgI == true){iAEUdCZTgI = false;}
      if(fELUtdxzHb == true){fELUtdxzHb = false;}
      if(QXrlgebike == true){QXrlgebike = false;}
      if(uHazEdcCTY == true){uHazEdcCTY = false;}
      if(ZlUtTntbKo == true){ZlUtTntbKo = false;}
      if(nUNfYLHRmz == true){nUNfYLHRmz = false;}
      if(aqhBMXdMPP == true){aqhBMXdMPP = false;}
      if(BtehBeDySs == true){BtehBeDySs = false;}
      if(jzYwqZstZs == true){jzYwqZstZs = false;}
      if(WKOUyiDSgu == true){WKOUyiDSgu = false;}
      if(JhMtnspeet == true){JhMtnspeet = false;}
      if(IJztOaSlKm == true){IJztOaSlKm = false;}
      if(eilVspfKjS == true){eilVspfKjS = false;}
      if(bjQWiBldIx == true){bjQWiBldIx = false;}
      if(jNPfkMLUFW == true){jNPfkMLUFW = false;}
      if(PQItQaupBx == true){PQItQaupBx = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WKYLPWWHVO
{ 
  void gMJQDRMMZw()
  { 
      bool jChrPaRuOJ = false;
      bool ijOjUHspUO = false;
      bool yuRTibjcYj = false;
      bool zKTupFpBFb = false;
      bool FrVFPYfFXf = false;
      bool QExpaojNJm = false;
      bool qoVeKYPTtP = false;
      bool QrjrJFZMEK = false;
      bool wdEsTtNcTB = false;
      bool UbrSkQVqtq = false;
      bool IMKIPXTEfV = false;
      bool OWJypMjcLP = false;
      bool LlzstozRHs = false;
      bool iiRNBjSwcu = false;
      bool aIxylxRsbX = false;
      bool fseefVMQNa = false;
      bool YCwlleEwxF = false;
      bool smzQoCoAHw = false;
      bool WGojMpDmhe = false;
      bool WCiCVXMKMG = false;
      string DNRcVaNXrQ;
      string GNaohiIVbn;
      string zhyuHUgiol;
      string JjcWPDGCug;
      string HLuGYQdnfy;
      string MsMJiVOVVA;
      string sdhGHVkEZe;
      string cGZtgmlqVp;
      string qrCqYQWSmt;
      string njftmcdozC;
      string eZOBVFSRcu;
      string iXAeJBaqgq;
      string zSQeprZeHE;
      string JwrSLRdBVT;
      string xFpqKPjBkl;
      string BmIHDMIiKc;
      string fmKXllfgSr;
      string WZUMstcYTZ;
      string IYtUDWnjOz;
      string ItprnJtRFY;
      if(DNRcVaNXrQ == eZOBVFSRcu){jChrPaRuOJ = true;}
      else if(eZOBVFSRcu == DNRcVaNXrQ){IMKIPXTEfV = true;}
      if(GNaohiIVbn == iXAeJBaqgq){ijOjUHspUO = true;}
      else if(iXAeJBaqgq == GNaohiIVbn){OWJypMjcLP = true;}
      if(zhyuHUgiol == zSQeprZeHE){yuRTibjcYj = true;}
      else if(zSQeprZeHE == zhyuHUgiol){LlzstozRHs = true;}
      if(JjcWPDGCug == JwrSLRdBVT){zKTupFpBFb = true;}
      else if(JwrSLRdBVT == JjcWPDGCug){iiRNBjSwcu = true;}
      if(HLuGYQdnfy == xFpqKPjBkl){FrVFPYfFXf = true;}
      else if(xFpqKPjBkl == HLuGYQdnfy){aIxylxRsbX = true;}
      if(MsMJiVOVVA == BmIHDMIiKc){QExpaojNJm = true;}
      else if(BmIHDMIiKc == MsMJiVOVVA){fseefVMQNa = true;}
      if(sdhGHVkEZe == fmKXllfgSr){qoVeKYPTtP = true;}
      else if(fmKXllfgSr == sdhGHVkEZe){YCwlleEwxF = true;}
      if(cGZtgmlqVp == WZUMstcYTZ){QrjrJFZMEK = true;}
      if(qrCqYQWSmt == IYtUDWnjOz){wdEsTtNcTB = true;}
      if(njftmcdozC == ItprnJtRFY){UbrSkQVqtq = true;}
      while(WZUMstcYTZ == cGZtgmlqVp){smzQoCoAHw = true;}
      while(IYtUDWnjOz == IYtUDWnjOz){WGojMpDmhe = true;}
      while(ItprnJtRFY == ItprnJtRFY){WCiCVXMKMG = true;}
      if(jChrPaRuOJ == true){jChrPaRuOJ = false;}
      if(ijOjUHspUO == true){ijOjUHspUO = false;}
      if(yuRTibjcYj == true){yuRTibjcYj = false;}
      if(zKTupFpBFb == true){zKTupFpBFb = false;}
      if(FrVFPYfFXf == true){FrVFPYfFXf = false;}
      if(QExpaojNJm == true){QExpaojNJm = false;}
      if(qoVeKYPTtP == true){qoVeKYPTtP = false;}
      if(QrjrJFZMEK == true){QrjrJFZMEK = false;}
      if(wdEsTtNcTB == true){wdEsTtNcTB = false;}
      if(UbrSkQVqtq == true){UbrSkQVqtq = false;}
      if(IMKIPXTEfV == true){IMKIPXTEfV = false;}
      if(OWJypMjcLP == true){OWJypMjcLP = false;}
      if(LlzstozRHs == true){LlzstozRHs = false;}
      if(iiRNBjSwcu == true){iiRNBjSwcu = false;}
      if(aIxylxRsbX == true){aIxylxRsbX = false;}
      if(fseefVMQNa == true){fseefVMQNa = false;}
      if(YCwlleEwxF == true){YCwlleEwxF = false;}
      if(smzQoCoAHw == true){smzQoCoAHw = false;}
      if(WGojMpDmhe == true){WGojMpDmhe = false;}
      if(WCiCVXMKMG == true){WCiCVXMKMG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RYTNOUNBKR
{ 
  void OIaicIHqrc()
  { 
      bool KuHTsXcIdQ = false;
      bool BIJLypyDGU = false;
      bool fICtgDlpZY = false;
      bool PtuCyeVFqU = false;
      bool aoUffDooDP = false;
      bool rRlICCrNlp = false;
      bool kmhEAGMGMT = false;
      bool UsMkspnVNc = false;
      bool NxlKgyHmzu = false;
      bool lHSBinHPfZ = false;
      bool qibjuyEAOL = false;
      bool OWzIbnAXXB = false;
      bool yZGRhMZLCF = false;
      bool iXpnleUCPh = false;
      bool sHXmgOHKYu = false;
      bool miqdaQbKCh = false;
      bool XsyJcpgLVx = false;
      bool SyIoIqCTjS = false;
      bool EPlGRAfLNV = false;
      bool DKzgJzINKq = false;
      string DHBVhtPJzi;
      string trrQQqMlyH;
      string qytLioLuWi;
      string cWRPkbgSuZ;
      string mbNeSslYgC;
      string aCsXOENpmr;
      string hUmQGXZcen;
      string KspXUPkPso;
      string DlNTIadtBf;
      string OpfCqRoxXu;
      string TIssrgLUCz;
      string SxwxouHTHe;
      string seayZTMmgD;
      string ZnqyVtsTaa;
      string PIrCNQtiNw;
      string KVosBnhtTY;
      string UXnzYeGyat;
      string CmrgPkmYkI;
      string ZdYkrEyywg;
      string MyACGqUsci;
      if(DHBVhtPJzi == TIssrgLUCz){KuHTsXcIdQ = true;}
      else if(TIssrgLUCz == DHBVhtPJzi){qibjuyEAOL = true;}
      if(trrQQqMlyH == SxwxouHTHe){BIJLypyDGU = true;}
      else if(SxwxouHTHe == trrQQqMlyH){OWzIbnAXXB = true;}
      if(qytLioLuWi == seayZTMmgD){fICtgDlpZY = true;}
      else if(seayZTMmgD == qytLioLuWi){yZGRhMZLCF = true;}
      if(cWRPkbgSuZ == ZnqyVtsTaa){PtuCyeVFqU = true;}
      else if(ZnqyVtsTaa == cWRPkbgSuZ){iXpnleUCPh = true;}
      if(mbNeSslYgC == PIrCNQtiNw){aoUffDooDP = true;}
      else if(PIrCNQtiNw == mbNeSslYgC){sHXmgOHKYu = true;}
      if(aCsXOENpmr == KVosBnhtTY){rRlICCrNlp = true;}
      else if(KVosBnhtTY == aCsXOENpmr){miqdaQbKCh = true;}
      if(hUmQGXZcen == UXnzYeGyat){kmhEAGMGMT = true;}
      else if(UXnzYeGyat == hUmQGXZcen){XsyJcpgLVx = true;}
      if(KspXUPkPso == CmrgPkmYkI){UsMkspnVNc = true;}
      if(DlNTIadtBf == ZdYkrEyywg){NxlKgyHmzu = true;}
      if(OpfCqRoxXu == MyACGqUsci){lHSBinHPfZ = true;}
      while(CmrgPkmYkI == KspXUPkPso){SyIoIqCTjS = true;}
      while(ZdYkrEyywg == ZdYkrEyywg){EPlGRAfLNV = true;}
      while(MyACGqUsci == MyACGqUsci){DKzgJzINKq = true;}
      if(KuHTsXcIdQ == true){KuHTsXcIdQ = false;}
      if(BIJLypyDGU == true){BIJLypyDGU = false;}
      if(fICtgDlpZY == true){fICtgDlpZY = false;}
      if(PtuCyeVFqU == true){PtuCyeVFqU = false;}
      if(aoUffDooDP == true){aoUffDooDP = false;}
      if(rRlICCrNlp == true){rRlICCrNlp = false;}
      if(kmhEAGMGMT == true){kmhEAGMGMT = false;}
      if(UsMkspnVNc == true){UsMkspnVNc = false;}
      if(NxlKgyHmzu == true){NxlKgyHmzu = false;}
      if(lHSBinHPfZ == true){lHSBinHPfZ = false;}
      if(qibjuyEAOL == true){qibjuyEAOL = false;}
      if(OWzIbnAXXB == true){OWzIbnAXXB = false;}
      if(yZGRhMZLCF == true){yZGRhMZLCF = false;}
      if(iXpnleUCPh == true){iXpnleUCPh = false;}
      if(sHXmgOHKYu == true){sHXmgOHKYu = false;}
      if(miqdaQbKCh == true){miqdaQbKCh = false;}
      if(XsyJcpgLVx == true){XsyJcpgLVx = false;}
      if(SyIoIqCTjS == true){SyIoIqCTjS = false;}
      if(EPlGRAfLNV == true){EPlGRAfLNV = false;}
      if(DKzgJzINKq == true){DKzgJzINKq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PZJMPHFFYK
{ 
  void SoBatAYiez()
  { 
      bool tzGpsTEiBe = false;
      bool VzFUPdIlQk = false;
      bool FdxzfeVblK = false;
      bool IFsAPjOEPB = false;
      bool ocdponKmNN = false;
      bool tgbtIQBGJO = false;
      bool xcwpVPGFFe = false;
      bool PDWOubzGJq = false;
      bool TiNYPbdyqy = false;
      bool hHtBEMgmTZ = false;
      bool hFghyqJMyg = false;
      bool QuwRuNOXZt = false;
      bool kHNGxHJVKi = false;
      bool uMmjJNBeZY = false;
      bool RQAKhmLbGq = false;
      bool yJMZnZrakp = false;
      bool gnCPkEHMIg = false;
      bool LjxBcTHCYK = false;
      bool wgPSklzNwi = false;
      bool VasjqcOkZc = false;
      string nXdxAhysEC;
      string PfysElPkhC;
      string rnsNfBswkG;
      string TUWVXkaWoM;
      string SyfRqFinOJ;
      string dEOnczeQQy;
      string nBHxiuJbUT;
      string KrwVjtHbWa;
      string SFhabOGhsY;
      string FKGpXQphhu;
      string uGStykIrEi;
      string qCsofiSgVT;
      string obiwNAlFxS;
      string DYsaeLmnbq;
      string GNcKgDQQKo;
      string yJMOGTXVfy;
      string zDtkeRIRKT;
      string ueEdNayXPK;
      string iUtIUtMeUK;
      string nQIcrDSreO;
      if(nXdxAhysEC == uGStykIrEi){tzGpsTEiBe = true;}
      else if(uGStykIrEi == nXdxAhysEC){hFghyqJMyg = true;}
      if(PfysElPkhC == qCsofiSgVT){VzFUPdIlQk = true;}
      else if(qCsofiSgVT == PfysElPkhC){QuwRuNOXZt = true;}
      if(rnsNfBswkG == obiwNAlFxS){FdxzfeVblK = true;}
      else if(obiwNAlFxS == rnsNfBswkG){kHNGxHJVKi = true;}
      if(TUWVXkaWoM == DYsaeLmnbq){IFsAPjOEPB = true;}
      else if(DYsaeLmnbq == TUWVXkaWoM){uMmjJNBeZY = true;}
      if(SyfRqFinOJ == GNcKgDQQKo){ocdponKmNN = true;}
      else if(GNcKgDQQKo == SyfRqFinOJ){RQAKhmLbGq = true;}
      if(dEOnczeQQy == yJMOGTXVfy){tgbtIQBGJO = true;}
      else if(yJMOGTXVfy == dEOnczeQQy){yJMZnZrakp = true;}
      if(nBHxiuJbUT == zDtkeRIRKT){xcwpVPGFFe = true;}
      else if(zDtkeRIRKT == nBHxiuJbUT){gnCPkEHMIg = true;}
      if(KrwVjtHbWa == ueEdNayXPK){PDWOubzGJq = true;}
      if(SFhabOGhsY == iUtIUtMeUK){TiNYPbdyqy = true;}
      if(FKGpXQphhu == nQIcrDSreO){hHtBEMgmTZ = true;}
      while(ueEdNayXPK == KrwVjtHbWa){LjxBcTHCYK = true;}
      while(iUtIUtMeUK == iUtIUtMeUK){wgPSklzNwi = true;}
      while(nQIcrDSreO == nQIcrDSreO){VasjqcOkZc = true;}
      if(tzGpsTEiBe == true){tzGpsTEiBe = false;}
      if(VzFUPdIlQk == true){VzFUPdIlQk = false;}
      if(FdxzfeVblK == true){FdxzfeVblK = false;}
      if(IFsAPjOEPB == true){IFsAPjOEPB = false;}
      if(ocdponKmNN == true){ocdponKmNN = false;}
      if(tgbtIQBGJO == true){tgbtIQBGJO = false;}
      if(xcwpVPGFFe == true){xcwpVPGFFe = false;}
      if(PDWOubzGJq == true){PDWOubzGJq = false;}
      if(TiNYPbdyqy == true){TiNYPbdyqy = false;}
      if(hHtBEMgmTZ == true){hHtBEMgmTZ = false;}
      if(hFghyqJMyg == true){hFghyqJMyg = false;}
      if(QuwRuNOXZt == true){QuwRuNOXZt = false;}
      if(kHNGxHJVKi == true){kHNGxHJVKi = false;}
      if(uMmjJNBeZY == true){uMmjJNBeZY = false;}
      if(RQAKhmLbGq == true){RQAKhmLbGq = false;}
      if(yJMZnZrakp == true){yJMZnZrakp = false;}
      if(gnCPkEHMIg == true){gnCPkEHMIg = false;}
      if(LjxBcTHCYK == true){LjxBcTHCYK = false;}
      if(wgPSklzNwi == true){wgPSklzNwi = false;}
      if(VasjqcOkZc == true){VasjqcOkZc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OAETMNXHNR
{ 
  void mdmzKRXAsj()
  { 
      bool rmopJHyFft = false;
      bool oCFjuJcZtZ = false;
      bool UWDrIPyAoA = false;
      bool XtuAOFLHKF = false;
      bool EhjnXIKEgn = false;
      bool mHtPNVjwgt = false;
      bool JxPfWtGFVp = false;
      bool xVOnUmlPUA = false;
      bool FsTANPAfls = false;
      bool UOZQuGREQc = false;
      bool CFippRrFta = false;
      bool iUbgoQdPSG = false;
      bool tZScViuyrO = false;
      bool VhyqyzKcJw = false;
      bool XtlLJAlrbe = false;
      bool bBlJiblGUH = false;
      bool VtNfEBWkio = false;
      bool GAFWiznHTU = false;
      bool wDWMlTunHV = false;
      bool NDICZcgGGH = false;
      string QEuNHtkOLP;
      string ksBlLtzPQQ;
      string qMqKiCnGtC;
      string nWBchsfRgf;
      string rqucttOTwV;
      string nQpnFDNkeF;
      string FdTIbNqYES;
      string NhikXAQNPn;
      string nUKYOWgMWd;
      string fulaZOWlBg;
      string CoPdHGhEQF;
      string lAUqRkTutF;
      string geCamSuHKI;
      string deMikyEfRI;
      string obuQqfZuJo;
      string ubquzXmoqn;
      string KZtHbHmlFT;
      string fmclPEikiG;
      string rAxUNCmQiM;
      string KPocxPgnoi;
      if(QEuNHtkOLP == CoPdHGhEQF){rmopJHyFft = true;}
      else if(CoPdHGhEQF == QEuNHtkOLP){CFippRrFta = true;}
      if(ksBlLtzPQQ == lAUqRkTutF){oCFjuJcZtZ = true;}
      else if(lAUqRkTutF == ksBlLtzPQQ){iUbgoQdPSG = true;}
      if(qMqKiCnGtC == geCamSuHKI){UWDrIPyAoA = true;}
      else if(geCamSuHKI == qMqKiCnGtC){tZScViuyrO = true;}
      if(nWBchsfRgf == deMikyEfRI){XtuAOFLHKF = true;}
      else if(deMikyEfRI == nWBchsfRgf){VhyqyzKcJw = true;}
      if(rqucttOTwV == obuQqfZuJo){EhjnXIKEgn = true;}
      else if(obuQqfZuJo == rqucttOTwV){XtlLJAlrbe = true;}
      if(nQpnFDNkeF == ubquzXmoqn){mHtPNVjwgt = true;}
      else if(ubquzXmoqn == nQpnFDNkeF){bBlJiblGUH = true;}
      if(FdTIbNqYES == KZtHbHmlFT){JxPfWtGFVp = true;}
      else if(KZtHbHmlFT == FdTIbNqYES){VtNfEBWkio = true;}
      if(NhikXAQNPn == fmclPEikiG){xVOnUmlPUA = true;}
      if(nUKYOWgMWd == rAxUNCmQiM){FsTANPAfls = true;}
      if(fulaZOWlBg == KPocxPgnoi){UOZQuGREQc = true;}
      while(fmclPEikiG == NhikXAQNPn){GAFWiznHTU = true;}
      while(rAxUNCmQiM == rAxUNCmQiM){wDWMlTunHV = true;}
      while(KPocxPgnoi == KPocxPgnoi){NDICZcgGGH = true;}
      if(rmopJHyFft == true){rmopJHyFft = false;}
      if(oCFjuJcZtZ == true){oCFjuJcZtZ = false;}
      if(UWDrIPyAoA == true){UWDrIPyAoA = false;}
      if(XtuAOFLHKF == true){XtuAOFLHKF = false;}
      if(EhjnXIKEgn == true){EhjnXIKEgn = false;}
      if(mHtPNVjwgt == true){mHtPNVjwgt = false;}
      if(JxPfWtGFVp == true){JxPfWtGFVp = false;}
      if(xVOnUmlPUA == true){xVOnUmlPUA = false;}
      if(FsTANPAfls == true){FsTANPAfls = false;}
      if(UOZQuGREQc == true){UOZQuGREQc = false;}
      if(CFippRrFta == true){CFippRrFta = false;}
      if(iUbgoQdPSG == true){iUbgoQdPSG = false;}
      if(tZScViuyrO == true){tZScViuyrO = false;}
      if(VhyqyzKcJw == true){VhyqyzKcJw = false;}
      if(XtlLJAlrbe == true){XtlLJAlrbe = false;}
      if(bBlJiblGUH == true){bBlJiblGUH = false;}
      if(VtNfEBWkio == true){VtNfEBWkio = false;}
      if(GAFWiznHTU == true){GAFWiznHTU = false;}
      if(wDWMlTunHV == true){wDWMlTunHV = false;}
      if(NDICZcgGGH == true){NDICZcgGGH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YVBHWGOMCR
{ 
  void FTStOIXCAG()
  { 
      bool bbMuXFEgja = false;
      bool jhqDZqurQY = false;
      bool cAinKFZYnG = false;
      bool HMzVyUdVeo = false;
      bool kdncNsziPm = false;
      bool rmsThbsYkS = false;
      bool eerdTXiUFE = false;
      bool MYNbmyLFZQ = false;
      bool FpJANTSfmW = false;
      bool bEkMJFlKJp = false;
      bool KedKZSZRfj = false;
      bool QmyUMnVqPz = false;
      bool ECgicInARh = false;
      bool LBwjFaMzZu = false;
      bool tBzDGoFQzU = false;
      bool jttKkIxuhR = false;
      bool eYzyXOqOOl = false;
      bool bWQUFDCdbm = false;
      bool CWAwxRGlyx = false;
      bool cQQIPUBBeq = false;
      string SQYFmfqkDM;
      string MWkOqueJGO;
      string ffLaCYCmKp;
      string CHgJjjQukH;
      string CXqigrziON;
      string GlPkemFyCN;
      string dEaNKFpbdH;
      string diXzLHnbsJ;
      string YTYxiPqudx;
      string eSsWcyxdYb;
      string CXEGiXOQYX;
      string ycEsnKecdt;
      string NtxfcLpAVN;
      string JKDRGBORNL;
      string BaWGzBOzhS;
      string STzdNIZXdX;
      string gNuLNnZlll;
      string uNPSsmXqga;
      string xpRCtoLYDy;
      string bpmpdmfKrL;
      if(SQYFmfqkDM == CXEGiXOQYX){bbMuXFEgja = true;}
      else if(CXEGiXOQYX == SQYFmfqkDM){KedKZSZRfj = true;}
      if(MWkOqueJGO == ycEsnKecdt){jhqDZqurQY = true;}
      else if(ycEsnKecdt == MWkOqueJGO){QmyUMnVqPz = true;}
      if(ffLaCYCmKp == NtxfcLpAVN){cAinKFZYnG = true;}
      else if(NtxfcLpAVN == ffLaCYCmKp){ECgicInARh = true;}
      if(CHgJjjQukH == JKDRGBORNL){HMzVyUdVeo = true;}
      else if(JKDRGBORNL == CHgJjjQukH){LBwjFaMzZu = true;}
      if(CXqigrziON == BaWGzBOzhS){kdncNsziPm = true;}
      else if(BaWGzBOzhS == CXqigrziON){tBzDGoFQzU = true;}
      if(GlPkemFyCN == STzdNIZXdX){rmsThbsYkS = true;}
      else if(STzdNIZXdX == GlPkemFyCN){jttKkIxuhR = true;}
      if(dEaNKFpbdH == gNuLNnZlll){eerdTXiUFE = true;}
      else if(gNuLNnZlll == dEaNKFpbdH){eYzyXOqOOl = true;}
      if(diXzLHnbsJ == uNPSsmXqga){MYNbmyLFZQ = true;}
      if(YTYxiPqudx == xpRCtoLYDy){FpJANTSfmW = true;}
      if(eSsWcyxdYb == bpmpdmfKrL){bEkMJFlKJp = true;}
      while(uNPSsmXqga == diXzLHnbsJ){bWQUFDCdbm = true;}
      while(xpRCtoLYDy == xpRCtoLYDy){CWAwxRGlyx = true;}
      while(bpmpdmfKrL == bpmpdmfKrL){cQQIPUBBeq = true;}
      if(bbMuXFEgja == true){bbMuXFEgja = false;}
      if(jhqDZqurQY == true){jhqDZqurQY = false;}
      if(cAinKFZYnG == true){cAinKFZYnG = false;}
      if(HMzVyUdVeo == true){HMzVyUdVeo = false;}
      if(kdncNsziPm == true){kdncNsziPm = false;}
      if(rmsThbsYkS == true){rmsThbsYkS = false;}
      if(eerdTXiUFE == true){eerdTXiUFE = false;}
      if(MYNbmyLFZQ == true){MYNbmyLFZQ = false;}
      if(FpJANTSfmW == true){FpJANTSfmW = false;}
      if(bEkMJFlKJp == true){bEkMJFlKJp = false;}
      if(KedKZSZRfj == true){KedKZSZRfj = false;}
      if(QmyUMnVqPz == true){QmyUMnVqPz = false;}
      if(ECgicInARh == true){ECgicInARh = false;}
      if(LBwjFaMzZu == true){LBwjFaMzZu = false;}
      if(tBzDGoFQzU == true){tBzDGoFQzU = false;}
      if(jttKkIxuhR == true){jttKkIxuhR = false;}
      if(eYzyXOqOOl == true){eYzyXOqOOl = false;}
      if(bWQUFDCdbm == true){bWQUFDCdbm = false;}
      if(CWAwxRGlyx == true){CWAwxRGlyx = false;}
      if(cQQIPUBBeq == true){cQQIPUBBeq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LLUFQKUBJR
{ 
  void DKdFJcGnMI()
  { 
      bool ozCCgXWWet = false;
      bool kYojafRKpB = false;
      bool oBHOXFVksi = false;
      bool eFqGYSyZHK = false;
      bool IIgOHMqYzM = false;
      bool ijHUHnrndb = false;
      bool BkcwPegiLH = false;
      bool gqCergTsGL = false;
      bool OwlgAtxWyk = false;
      bool aJwfJXfkyb = false;
      bool fINqeqhbba = false;
      bool TzhSNFywuV = false;
      bool hKjagXuWVT = false;
      bool AitdeGhtBJ = false;
      bool LUtWQbacRD = false;
      bool YioctUXmKG = false;
      bool uKGHZkEDmH = false;
      bool WDtlJMQMJE = false;
      bool wSESuRtQjK = false;
      bool hujtNAOlwf = false;
      string YntJwDuEwO;
      string RgjuLWrmMU;
      string ZGceXoEoYS;
      string CuVWuDaiCb;
      string tuxGLaPPVZ;
      string BVslYSLOkH;
      string yFfcLWKNzH;
      string uIkkadIVTx;
      string ZnTyRfCixc;
      string rLxtcnPTto;
      string PBPNdrkHLu;
      string JounhJlgLe;
      string UbPWPjKpHl;
      string NnbdgXYGUZ;
      string utXCSCQJMS;
      string CiIOUnlsxt;
      string guzZgVNQGK;
      string keLljZDNDN;
      string SBcoiefMJr;
      string lfJarAuyVx;
      if(YntJwDuEwO == PBPNdrkHLu){ozCCgXWWet = true;}
      else if(PBPNdrkHLu == YntJwDuEwO){fINqeqhbba = true;}
      if(RgjuLWrmMU == JounhJlgLe){kYojafRKpB = true;}
      else if(JounhJlgLe == RgjuLWrmMU){TzhSNFywuV = true;}
      if(ZGceXoEoYS == UbPWPjKpHl){oBHOXFVksi = true;}
      else if(UbPWPjKpHl == ZGceXoEoYS){hKjagXuWVT = true;}
      if(CuVWuDaiCb == NnbdgXYGUZ){eFqGYSyZHK = true;}
      else if(NnbdgXYGUZ == CuVWuDaiCb){AitdeGhtBJ = true;}
      if(tuxGLaPPVZ == utXCSCQJMS){IIgOHMqYzM = true;}
      else if(utXCSCQJMS == tuxGLaPPVZ){LUtWQbacRD = true;}
      if(BVslYSLOkH == CiIOUnlsxt){ijHUHnrndb = true;}
      else if(CiIOUnlsxt == BVslYSLOkH){YioctUXmKG = true;}
      if(yFfcLWKNzH == guzZgVNQGK){BkcwPegiLH = true;}
      else if(guzZgVNQGK == yFfcLWKNzH){uKGHZkEDmH = true;}
      if(uIkkadIVTx == keLljZDNDN){gqCergTsGL = true;}
      if(ZnTyRfCixc == SBcoiefMJr){OwlgAtxWyk = true;}
      if(rLxtcnPTto == lfJarAuyVx){aJwfJXfkyb = true;}
      while(keLljZDNDN == uIkkadIVTx){WDtlJMQMJE = true;}
      while(SBcoiefMJr == SBcoiefMJr){wSESuRtQjK = true;}
      while(lfJarAuyVx == lfJarAuyVx){hujtNAOlwf = true;}
      if(ozCCgXWWet == true){ozCCgXWWet = false;}
      if(kYojafRKpB == true){kYojafRKpB = false;}
      if(oBHOXFVksi == true){oBHOXFVksi = false;}
      if(eFqGYSyZHK == true){eFqGYSyZHK = false;}
      if(IIgOHMqYzM == true){IIgOHMqYzM = false;}
      if(ijHUHnrndb == true){ijHUHnrndb = false;}
      if(BkcwPegiLH == true){BkcwPegiLH = false;}
      if(gqCergTsGL == true){gqCergTsGL = false;}
      if(OwlgAtxWyk == true){OwlgAtxWyk = false;}
      if(aJwfJXfkyb == true){aJwfJXfkyb = false;}
      if(fINqeqhbba == true){fINqeqhbba = false;}
      if(TzhSNFywuV == true){TzhSNFywuV = false;}
      if(hKjagXuWVT == true){hKjagXuWVT = false;}
      if(AitdeGhtBJ == true){AitdeGhtBJ = false;}
      if(LUtWQbacRD == true){LUtWQbacRD = false;}
      if(YioctUXmKG == true){YioctUXmKG = false;}
      if(uKGHZkEDmH == true){uKGHZkEDmH = false;}
      if(WDtlJMQMJE == true){WDtlJMQMJE = false;}
      if(wSESuRtQjK == true){wSESuRtQjK = false;}
      if(hujtNAOlwf == true){hujtNAOlwf = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KCYRNXALGF
{ 
  void RPbCXnaomR()
  { 
      bool YGzVPVFkjW = false;
      bool RfAopctMjD = false;
      bool MPSWFwueih = false;
      bool ejgrskKzYi = false;
      bool IIlYEewpVr = false;
      bool NpmnWJmcQS = false;
      bool aZBBrhzIuF = false;
      bool StmAtEcsRZ = false;
      bool AEJpUowrUj = false;
      bool xgiFIiGjgW = false;
      bool fntPYSClsS = false;
      bool ftLpRVQKgD = false;
      bool kGmLIkLhrL = false;
      bool yrLttMKsQB = false;
      bool mKzCECVrmm = false;
      bool RuDdkYAiHm = false;
      bool HUtgUmhQwF = false;
      bool KWtePaGjHF = false;
      bool PStEsCohJV = false;
      bool MbkctSPRhQ = false;
      string luYeyKKytG;
      string OdfJWlycsJ;
      string wIfTBNTYad;
      string waHJJFpLCS;
      string wEUxcNKxBo;
      string czYEVcOFcj;
      string pkWPKmVWzR;
      string wlaZMrnhpm;
      string DWWlAwCDmf;
      string pjiRcKgEmo;
      string qnxjsqYyqc;
      string ofPofTOjON;
      string kSiTbLDIxB;
      string KiuAuPZmsJ;
      string XDLJpJUslI;
      string wwpYEYEcHg;
      string mQBoRGkOsP;
      string saqEogGYcR;
      string qJVnFOsVwW;
      string KwfLVHCWrj;
      if(luYeyKKytG == qnxjsqYyqc){YGzVPVFkjW = true;}
      else if(qnxjsqYyqc == luYeyKKytG){fntPYSClsS = true;}
      if(OdfJWlycsJ == ofPofTOjON){RfAopctMjD = true;}
      else if(ofPofTOjON == OdfJWlycsJ){ftLpRVQKgD = true;}
      if(wIfTBNTYad == kSiTbLDIxB){MPSWFwueih = true;}
      else if(kSiTbLDIxB == wIfTBNTYad){kGmLIkLhrL = true;}
      if(waHJJFpLCS == KiuAuPZmsJ){ejgrskKzYi = true;}
      else if(KiuAuPZmsJ == waHJJFpLCS){yrLttMKsQB = true;}
      if(wEUxcNKxBo == XDLJpJUslI){IIlYEewpVr = true;}
      else if(XDLJpJUslI == wEUxcNKxBo){mKzCECVrmm = true;}
      if(czYEVcOFcj == wwpYEYEcHg){NpmnWJmcQS = true;}
      else if(wwpYEYEcHg == czYEVcOFcj){RuDdkYAiHm = true;}
      if(pkWPKmVWzR == mQBoRGkOsP){aZBBrhzIuF = true;}
      else if(mQBoRGkOsP == pkWPKmVWzR){HUtgUmhQwF = true;}
      if(wlaZMrnhpm == saqEogGYcR){StmAtEcsRZ = true;}
      if(DWWlAwCDmf == qJVnFOsVwW){AEJpUowrUj = true;}
      if(pjiRcKgEmo == KwfLVHCWrj){xgiFIiGjgW = true;}
      while(saqEogGYcR == wlaZMrnhpm){KWtePaGjHF = true;}
      while(qJVnFOsVwW == qJVnFOsVwW){PStEsCohJV = true;}
      while(KwfLVHCWrj == KwfLVHCWrj){MbkctSPRhQ = true;}
      if(YGzVPVFkjW == true){YGzVPVFkjW = false;}
      if(RfAopctMjD == true){RfAopctMjD = false;}
      if(MPSWFwueih == true){MPSWFwueih = false;}
      if(ejgrskKzYi == true){ejgrskKzYi = false;}
      if(IIlYEewpVr == true){IIlYEewpVr = false;}
      if(NpmnWJmcQS == true){NpmnWJmcQS = false;}
      if(aZBBrhzIuF == true){aZBBrhzIuF = false;}
      if(StmAtEcsRZ == true){StmAtEcsRZ = false;}
      if(AEJpUowrUj == true){AEJpUowrUj = false;}
      if(xgiFIiGjgW == true){xgiFIiGjgW = false;}
      if(fntPYSClsS == true){fntPYSClsS = false;}
      if(ftLpRVQKgD == true){ftLpRVQKgD = false;}
      if(kGmLIkLhrL == true){kGmLIkLhrL = false;}
      if(yrLttMKsQB == true){yrLttMKsQB = false;}
      if(mKzCECVrmm == true){mKzCECVrmm = false;}
      if(RuDdkYAiHm == true){RuDdkYAiHm = false;}
      if(HUtgUmhQwF == true){HUtgUmhQwF = false;}
      if(KWtePaGjHF == true){KWtePaGjHF = false;}
      if(PStEsCohJV == true){PStEsCohJV = false;}
      if(MbkctSPRhQ == true){MbkctSPRhQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MXSZTILPUO
{ 
  void enjNsIMOUl()
  { 
      bool TnRcAeMsXc = false;
      bool BNgcxLRPKc = false;
      bool PjxHCweZHt = false;
      bool FGdOzWOggI = false;
      bool bDjjnoTJZL = false;
      bool KGZlHiqKaD = false;
      bool AlkcXtjRdZ = false;
      bool wkiBAMzMSM = false;
      bool bUzwjygrGP = false;
      bool fEGcIpSHMt = false;
      bool FBfAOYXXku = false;
      bool idqNbelnsY = false;
      bool NqLNtqeFsK = false;
      bool srAYGZnzgK = false;
      bool aIHKfBMrBc = false;
      bool DdNmgOpSFX = false;
      bool tReaBqdSHB = false;
      bool plGiyEjBhr = false;
      bool dlEPGPeMrq = false;
      bool jVsJPIdfIo = false;
      string UbHqtMtlen;
      string AfOXcMPydx;
      string DBCKKnlfHg;
      string lLnujkZZGm;
      string MRPZpKycUH;
      string dMRUASZxfG;
      string ZNdxBorSmq;
      string morqkklbTA;
      string oJLADZsStQ;
      string KctzNzmRXB;
      string ReocTnUZjP;
      string gxceQQBdtb;
      string uZXWgKurdl;
      string yTEShKTifP;
      string EShejNzOiw;
      string xJXuFrGSwI;
      string rbwyQMsKxF;
      string wRxeXbqrlS;
      string LgJgnnwINp;
      string DAmWWcWjOX;
      if(UbHqtMtlen == ReocTnUZjP){TnRcAeMsXc = true;}
      else if(ReocTnUZjP == UbHqtMtlen){FBfAOYXXku = true;}
      if(AfOXcMPydx == gxceQQBdtb){BNgcxLRPKc = true;}
      else if(gxceQQBdtb == AfOXcMPydx){idqNbelnsY = true;}
      if(DBCKKnlfHg == uZXWgKurdl){PjxHCweZHt = true;}
      else if(uZXWgKurdl == DBCKKnlfHg){NqLNtqeFsK = true;}
      if(lLnujkZZGm == yTEShKTifP){FGdOzWOggI = true;}
      else if(yTEShKTifP == lLnujkZZGm){srAYGZnzgK = true;}
      if(MRPZpKycUH == EShejNzOiw){bDjjnoTJZL = true;}
      else if(EShejNzOiw == MRPZpKycUH){aIHKfBMrBc = true;}
      if(dMRUASZxfG == xJXuFrGSwI){KGZlHiqKaD = true;}
      else if(xJXuFrGSwI == dMRUASZxfG){DdNmgOpSFX = true;}
      if(ZNdxBorSmq == rbwyQMsKxF){AlkcXtjRdZ = true;}
      else if(rbwyQMsKxF == ZNdxBorSmq){tReaBqdSHB = true;}
      if(morqkklbTA == wRxeXbqrlS){wkiBAMzMSM = true;}
      if(oJLADZsStQ == LgJgnnwINp){bUzwjygrGP = true;}
      if(KctzNzmRXB == DAmWWcWjOX){fEGcIpSHMt = true;}
      while(wRxeXbqrlS == morqkklbTA){plGiyEjBhr = true;}
      while(LgJgnnwINp == LgJgnnwINp){dlEPGPeMrq = true;}
      while(DAmWWcWjOX == DAmWWcWjOX){jVsJPIdfIo = true;}
      if(TnRcAeMsXc == true){TnRcAeMsXc = false;}
      if(BNgcxLRPKc == true){BNgcxLRPKc = false;}
      if(PjxHCweZHt == true){PjxHCweZHt = false;}
      if(FGdOzWOggI == true){FGdOzWOggI = false;}
      if(bDjjnoTJZL == true){bDjjnoTJZL = false;}
      if(KGZlHiqKaD == true){KGZlHiqKaD = false;}
      if(AlkcXtjRdZ == true){AlkcXtjRdZ = false;}
      if(wkiBAMzMSM == true){wkiBAMzMSM = false;}
      if(bUzwjygrGP == true){bUzwjygrGP = false;}
      if(fEGcIpSHMt == true){fEGcIpSHMt = false;}
      if(FBfAOYXXku == true){FBfAOYXXku = false;}
      if(idqNbelnsY == true){idqNbelnsY = false;}
      if(NqLNtqeFsK == true){NqLNtqeFsK = false;}
      if(srAYGZnzgK == true){srAYGZnzgK = false;}
      if(aIHKfBMrBc == true){aIHKfBMrBc = false;}
      if(DdNmgOpSFX == true){DdNmgOpSFX = false;}
      if(tReaBqdSHB == true){tReaBqdSHB = false;}
      if(plGiyEjBhr == true){plGiyEjBhr = false;}
      if(dlEPGPeMrq == true){dlEPGPeMrq = false;}
      if(jVsJPIdfIo == true){jVsJPIdfIo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ONZORLEWPG
{ 
  void sJaFsQisrS()
  { 
      bool NjFKuepVgG = false;
      bool JVLPoSaBft = false;
      bool TBGcHdJuYB = false;
      bool aYwGjIkTVN = false;
      bool CMSlknJgPF = false;
      bool YGPNOMccDQ = false;
      bool VuweejZFff = false;
      bool gHeSrGhUqS = false;
      bool UceTcUJiqW = false;
      bool eHQEqHQnHA = false;
      bool ODAjXRbwUI = false;
      bool dIlkwsPbCJ = false;
      bool aqNIAZpZaj = false;
      bool otNpEQcuwK = false;
      bool sMNhsWVfIy = false;
      bool rokxZWoqix = false;
      bool bBVkSjtufe = false;
      bool QTfrQNcfPc = false;
      bool DKRbrAIQQg = false;
      bool bkLyhrqcPI = false;
      string qRLeMKirdC;
      string XdVKfZTgIp;
      string imCwjzqdEm;
      string XIMEsQfWsh;
      string HTnYOmosqF;
      string aCEmmDOPuN;
      string SEnpeOQQQO;
      string txnahPzchw;
      string JymzLUEZRp;
      string BQbfODfxsz;
      string VYbbLdNdEW;
      string EkgyDURneJ;
      string JqyaNcUHLe;
      string jBjhZkVKmg;
      string HaVtkPidct;
      string lqSjpeZBNh;
      string JGSZmCBzXR;
      string zGAxCiUDoy;
      string sqeQsriAru;
      string KUwFXAHwHi;
      if(qRLeMKirdC == VYbbLdNdEW){NjFKuepVgG = true;}
      else if(VYbbLdNdEW == qRLeMKirdC){ODAjXRbwUI = true;}
      if(XdVKfZTgIp == EkgyDURneJ){JVLPoSaBft = true;}
      else if(EkgyDURneJ == XdVKfZTgIp){dIlkwsPbCJ = true;}
      if(imCwjzqdEm == JqyaNcUHLe){TBGcHdJuYB = true;}
      else if(JqyaNcUHLe == imCwjzqdEm){aqNIAZpZaj = true;}
      if(XIMEsQfWsh == jBjhZkVKmg){aYwGjIkTVN = true;}
      else if(jBjhZkVKmg == XIMEsQfWsh){otNpEQcuwK = true;}
      if(HTnYOmosqF == HaVtkPidct){CMSlknJgPF = true;}
      else if(HaVtkPidct == HTnYOmosqF){sMNhsWVfIy = true;}
      if(aCEmmDOPuN == lqSjpeZBNh){YGPNOMccDQ = true;}
      else if(lqSjpeZBNh == aCEmmDOPuN){rokxZWoqix = true;}
      if(SEnpeOQQQO == JGSZmCBzXR){VuweejZFff = true;}
      else if(JGSZmCBzXR == SEnpeOQQQO){bBVkSjtufe = true;}
      if(txnahPzchw == zGAxCiUDoy){gHeSrGhUqS = true;}
      if(JymzLUEZRp == sqeQsriAru){UceTcUJiqW = true;}
      if(BQbfODfxsz == KUwFXAHwHi){eHQEqHQnHA = true;}
      while(zGAxCiUDoy == txnahPzchw){QTfrQNcfPc = true;}
      while(sqeQsriAru == sqeQsriAru){DKRbrAIQQg = true;}
      while(KUwFXAHwHi == KUwFXAHwHi){bkLyhrqcPI = true;}
      if(NjFKuepVgG == true){NjFKuepVgG = false;}
      if(JVLPoSaBft == true){JVLPoSaBft = false;}
      if(TBGcHdJuYB == true){TBGcHdJuYB = false;}
      if(aYwGjIkTVN == true){aYwGjIkTVN = false;}
      if(CMSlknJgPF == true){CMSlknJgPF = false;}
      if(YGPNOMccDQ == true){YGPNOMccDQ = false;}
      if(VuweejZFff == true){VuweejZFff = false;}
      if(gHeSrGhUqS == true){gHeSrGhUqS = false;}
      if(UceTcUJiqW == true){UceTcUJiqW = false;}
      if(eHQEqHQnHA == true){eHQEqHQnHA = false;}
      if(ODAjXRbwUI == true){ODAjXRbwUI = false;}
      if(dIlkwsPbCJ == true){dIlkwsPbCJ = false;}
      if(aqNIAZpZaj == true){aqNIAZpZaj = false;}
      if(otNpEQcuwK == true){otNpEQcuwK = false;}
      if(sMNhsWVfIy == true){sMNhsWVfIy = false;}
      if(rokxZWoqix == true){rokxZWoqix = false;}
      if(bBVkSjtufe == true){bBVkSjtufe = false;}
      if(QTfrQNcfPc == true){QTfrQNcfPc = false;}
      if(DKRbrAIQQg == true){DKRbrAIQQg = false;}
      if(bkLyhrqcPI == true){bkLyhrqcPI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SSAJOGBGKC
{ 
  void kSgRWtscDs()
  { 
      bool qBCXjkpsWZ = false;
      bool aloTKAxXca = false;
      bool uoFFMGmJHx = false;
      bool igwVtnkPgi = false;
      bool BCTrrodNkf = false;
      bool OKYYpCwwKd = false;
      bool xflZqspSZV = false;
      bool OgqtZsUMNW = false;
      bool cYXahXHWfq = false;
      bool roEClUPtBy = false;
      bool zKnPQgTezw = false;
      bool giimBXHLqS = false;
      bool kSmhcZbnIE = false;
      bool daLOXRkIJB = false;
      bool EcXiKIFRBN = false;
      bool MEChMfWFlq = false;
      bool FeWuSGXTPi = false;
      bool lzBpOIuEXU = false;
      bool TKltqYXcBq = false;
      bool tfVSwYFcPD = false;
      string fqWJLBKJES;
      string FyHVUtEmJU;
      string yqfAfAOzuW;
      string fboRlkaJZk;
      string cMXXKFoRxi;
      string yynQrnKVdL;
      string lLOgmcJoii;
      string TuNYrlTHpZ;
      string LxBLYgnYYy;
      string ZEeNyQypkg;
      string uzhNPDkPXG;
      string UCfDAoJCIF;
      string ojPkIOPAaa;
      string mcWIdLPnwU;
      string wXAScWYbMJ;
      string GwgfqLixFA;
      string nkoxSqfWYo;
      string YVFnIRjwHk;
      string pnmQTquqRS;
      string bShwgLiCAl;
      if(fqWJLBKJES == uzhNPDkPXG){qBCXjkpsWZ = true;}
      else if(uzhNPDkPXG == fqWJLBKJES){zKnPQgTezw = true;}
      if(FyHVUtEmJU == UCfDAoJCIF){aloTKAxXca = true;}
      else if(UCfDAoJCIF == FyHVUtEmJU){giimBXHLqS = true;}
      if(yqfAfAOzuW == ojPkIOPAaa){uoFFMGmJHx = true;}
      else if(ojPkIOPAaa == yqfAfAOzuW){kSmhcZbnIE = true;}
      if(fboRlkaJZk == mcWIdLPnwU){igwVtnkPgi = true;}
      else if(mcWIdLPnwU == fboRlkaJZk){daLOXRkIJB = true;}
      if(cMXXKFoRxi == wXAScWYbMJ){BCTrrodNkf = true;}
      else if(wXAScWYbMJ == cMXXKFoRxi){EcXiKIFRBN = true;}
      if(yynQrnKVdL == GwgfqLixFA){OKYYpCwwKd = true;}
      else if(GwgfqLixFA == yynQrnKVdL){MEChMfWFlq = true;}
      if(lLOgmcJoii == nkoxSqfWYo){xflZqspSZV = true;}
      else if(nkoxSqfWYo == lLOgmcJoii){FeWuSGXTPi = true;}
      if(TuNYrlTHpZ == YVFnIRjwHk){OgqtZsUMNW = true;}
      if(LxBLYgnYYy == pnmQTquqRS){cYXahXHWfq = true;}
      if(ZEeNyQypkg == bShwgLiCAl){roEClUPtBy = true;}
      while(YVFnIRjwHk == TuNYrlTHpZ){lzBpOIuEXU = true;}
      while(pnmQTquqRS == pnmQTquqRS){TKltqYXcBq = true;}
      while(bShwgLiCAl == bShwgLiCAl){tfVSwYFcPD = true;}
      if(qBCXjkpsWZ == true){qBCXjkpsWZ = false;}
      if(aloTKAxXca == true){aloTKAxXca = false;}
      if(uoFFMGmJHx == true){uoFFMGmJHx = false;}
      if(igwVtnkPgi == true){igwVtnkPgi = false;}
      if(BCTrrodNkf == true){BCTrrodNkf = false;}
      if(OKYYpCwwKd == true){OKYYpCwwKd = false;}
      if(xflZqspSZV == true){xflZqspSZV = false;}
      if(OgqtZsUMNW == true){OgqtZsUMNW = false;}
      if(cYXahXHWfq == true){cYXahXHWfq = false;}
      if(roEClUPtBy == true){roEClUPtBy = false;}
      if(zKnPQgTezw == true){zKnPQgTezw = false;}
      if(giimBXHLqS == true){giimBXHLqS = false;}
      if(kSmhcZbnIE == true){kSmhcZbnIE = false;}
      if(daLOXRkIJB == true){daLOXRkIJB = false;}
      if(EcXiKIFRBN == true){EcXiKIFRBN = false;}
      if(MEChMfWFlq == true){MEChMfWFlq = false;}
      if(FeWuSGXTPi == true){FeWuSGXTPi = false;}
      if(lzBpOIuEXU == true){lzBpOIuEXU = false;}
      if(TKltqYXcBq == true){TKltqYXcBq = false;}
      if(tfVSwYFcPD == true){tfVSwYFcPD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TFFOZZAGOQ
{ 
  void DOJOCcVCRX()
  { 
      bool OYrOPdpeHf = false;
      bool RCOeyzKYxW = false;
      bool GXESxyqWFb = false;
      bool HTaUOoJXWA = false;
      bool BtcgsHVhfT = false;
      bool zBfFlpUZRR = false;
      bool lTYeYxbsQJ = false;
      bool olBxUMtWYo = false;
      bool cJqkcFnUiX = false;
      bool UjFENhUyIw = false;
      bool fbRVdMzikZ = false;
      bool uyBbncbnBS = false;
      bool JGKroFfwil = false;
      bool qwOcJAqUGQ = false;
      bool GEjIcWidQk = false;
      bool GgxolZQYgY = false;
      bool fCAyhziYSm = false;
      bool kHgkMFWBPS = false;
      bool WCfyfJyYpi = false;
      bool jRmhjDDkac = false;
      string DKjQHiAZQg;
      string KFInYTewwh;
      string SSIlPNMxSY;
      string aJGSSNfdpQ;
      string MtprfTsjTH;
      string bMbLZkCxBs;
      string WDGrGAzpUL;
      string mAFeVYbPFw;
      string UbdLwNhush;
      string KhbwtImylf;
      string TDOITaGwoA;
      string ImqFYuZtur;
      string SEwmAFYJkM;
      string kcdZbQMUrV;
      string GkFrYXpWWy;
      string irDsMPGbHN;
      string CxKbPfIQdH;
      string iEBYlgyDNc;
      string OCmldrYkML;
      string KjNeTJZdcs;
      if(DKjQHiAZQg == TDOITaGwoA){OYrOPdpeHf = true;}
      else if(TDOITaGwoA == DKjQHiAZQg){fbRVdMzikZ = true;}
      if(KFInYTewwh == ImqFYuZtur){RCOeyzKYxW = true;}
      else if(ImqFYuZtur == KFInYTewwh){uyBbncbnBS = true;}
      if(SSIlPNMxSY == SEwmAFYJkM){GXESxyqWFb = true;}
      else if(SEwmAFYJkM == SSIlPNMxSY){JGKroFfwil = true;}
      if(aJGSSNfdpQ == kcdZbQMUrV){HTaUOoJXWA = true;}
      else if(kcdZbQMUrV == aJGSSNfdpQ){qwOcJAqUGQ = true;}
      if(MtprfTsjTH == GkFrYXpWWy){BtcgsHVhfT = true;}
      else if(GkFrYXpWWy == MtprfTsjTH){GEjIcWidQk = true;}
      if(bMbLZkCxBs == irDsMPGbHN){zBfFlpUZRR = true;}
      else if(irDsMPGbHN == bMbLZkCxBs){GgxolZQYgY = true;}
      if(WDGrGAzpUL == CxKbPfIQdH){lTYeYxbsQJ = true;}
      else if(CxKbPfIQdH == WDGrGAzpUL){fCAyhziYSm = true;}
      if(mAFeVYbPFw == iEBYlgyDNc){olBxUMtWYo = true;}
      if(UbdLwNhush == OCmldrYkML){cJqkcFnUiX = true;}
      if(KhbwtImylf == KjNeTJZdcs){UjFENhUyIw = true;}
      while(iEBYlgyDNc == mAFeVYbPFw){kHgkMFWBPS = true;}
      while(OCmldrYkML == OCmldrYkML){WCfyfJyYpi = true;}
      while(KjNeTJZdcs == KjNeTJZdcs){jRmhjDDkac = true;}
      if(OYrOPdpeHf == true){OYrOPdpeHf = false;}
      if(RCOeyzKYxW == true){RCOeyzKYxW = false;}
      if(GXESxyqWFb == true){GXESxyqWFb = false;}
      if(HTaUOoJXWA == true){HTaUOoJXWA = false;}
      if(BtcgsHVhfT == true){BtcgsHVhfT = false;}
      if(zBfFlpUZRR == true){zBfFlpUZRR = false;}
      if(lTYeYxbsQJ == true){lTYeYxbsQJ = false;}
      if(olBxUMtWYo == true){olBxUMtWYo = false;}
      if(cJqkcFnUiX == true){cJqkcFnUiX = false;}
      if(UjFENhUyIw == true){UjFENhUyIw = false;}
      if(fbRVdMzikZ == true){fbRVdMzikZ = false;}
      if(uyBbncbnBS == true){uyBbncbnBS = false;}
      if(JGKroFfwil == true){JGKroFfwil = false;}
      if(qwOcJAqUGQ == true){qwOcJAqUGQ = false;}
      if(GEjIcWidQk == true){GEjIcWidQk = false;}
      if(GgxolZQYgY == true){GgxolZQYgY = false;}
      if(fCAyhziYSm == true){fCAyhziYSm = false;}
      if(kHgkMFWBPS == true){kHgkMFWBPS = false;}
      if(WCfyfJyYpi == true){WCfyfJyYpi = false;}
      if(jRmhjDDkac == true){jRmhjDDkac = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ROQHSXYZFV
{ 
  void syJrTdbPGD()
  { 
      bool QGuYddwEWi = false;
      bool VkDgqjcHlq = false;
      bool NkDlmFslPI = false;
      bool kmoseAdicH = false;
      bool hkjDRaSybS = false;
      bool pjeSlfCIkX = false;
      bool kKoOxhwJCp = false;
      bool GtddJbAzUM = false;
      bool mtJwRBIWGy = false;
      bool xUyFVWwVGj = false;
      bool eIwkLwFAJK = false;
      bool eDnYjqArww = false;
      bool ZZDDslwYHl = false;
      bool fdskgexYkU = false;
      bool uHNwStfYUe = false;
      bool WEZKJFFsqZ = false;
      bool AfrWdBKDqF = false;
      bool saHaARLGjz = false;
      bool FWEcPaEhab = false;
      bool ZDgKUOPpRR = false;
      string nUuKjhSnay;
      string ojnNQIjaUm;
      string GKeLqeZrDd;
      string rVpfwAUdZI;
      string LGyoPLVLzm;
      string LTnnAcpjTE;
      string IZkwGStIQQ;
      string mGBMaAMoue;
      string JNrPRXhBeT;
      string KkKUzkUBhw;
      string RMtiAFSaNA;
      string PAnnuthUeh;
      string jrcxGCugMo;
      string eLHyNwSIPM;
      string FGneDJwVYF;
      string DAVzMZjxcx;
      string JkOXUDrpaz;
      string bYERcdDysw;
      string GYHGldoHln;
      string MVfSCTrQJA;
      if(nUuKjhSnay == RMtiAFSaNA){QGuYddwEWi = true;}
      else if(RMtiAFSaNA == nUuKjhSnay){eIwkLwFAJK = true;}
      if(ojnNQIjaUm == PAnnuthUeh){VkDgqjcHlq = true;}
      else if(PAnnuthUeh == ojnNQIjaUm){eDnYjqArww = true;}
      if(GKeLqeZrDd == jrcxGCugMo){NkDlmFslPI = true;}
      else if(jrcxGCugMo == GKeLqeZrDd){ZZDDslwYHl = true;}
      if(rVpfwAUdZI == eLHyNwSIPM){kmoseAdicH = true;}
      else if(eLHyNwSIPM == rVpfwAUdZI){fdskgexYkU = true;}
      if(LGyoPLVLzm == FGneDJwVYF){hkjDRaSybS = true;}
      else if(FGneDJwVYF == LGyoPLVLzm){uHNwStfYUe = true;}
      if(LTnnAcpjTE == DAVzMZjxcx){pjeSlfCIkX = true;}
      else if(DAVzMZjxcx == LTnnAcpjTE){WEZKJFFsqZ = true;}
      if(IZkwGStIQQ == JkOXUDrpaz){kKoOxhwJCp = true;}
      else if(JkOXUDrpaz == IZkwGStIQQ){AfrWdBKDqF = true;}
      if(mGBMaAMoue == bYERcdDysw){GtddJbAzUM = true;}
      if(JNrPRXhBeT == GYHGldoHln){mtJwRBIWGy = true;}
      if(KkKUzkUBhw == MVfSCTrQJA){xUyFVWwVGj = true;}
      while(bYERcdDysw == mGBMaAMoue){saHaARLGjz = true;}
      while(GYHGldoHln == GYHGldoHln){FWEcPaEhab = true;}
      while(MVfSCTrQJA == MVfSCTrQJA){ZDgKUOPpRR = true;}
      if(QGuYddwEWi == true){QGuYddwEWi = false;}
      if(VkDgqjcHlq == true){VkDgqjcHlq = false;}
      if(NkDlmFslPI == true){NkDlmFslPI = false;}
      if(kmoseAdicH == true){kmoseAdicH = false;}
      if(hkjDRaSybS == true){hkjDRaSybS = false;}
      if(pjeSlfCIkX == true){pjeSlfCIkX = false;}
      if(kKoOxhwJCp == true){kKoOxhwJCp = false;}
      if(GtddJbAzUM == true){GtddJbAzUM = false;}
      if(mtJwRBIWGy == true){mtJwRBIWGy = false;}
      if(xUyFVWwVGj == true){xUyFVWwVGj = false;}
      if(eIwkLwFAJK == true){eIwkLwFAJK = false;}
      if(eDnYjqArww == true){eDnYjqArww = false;}
      if(ZZDDslwYHl == true){ZZDDslwYHl = false;}
      if(fdskgexYkU == true){fdskgexYkU = false;}
      if(uHNwStfYUe == true){uHNwStfYUe = false;}
      if(WEZKJFFsqZ == true){WEZKJFFsqZ = false;}
      if(AfrWdBKDqF == true){AfrWdBKDqF = false;}
      if(saHaARLGjz == true){saHaARLGjz = false;}
      if(FWEcPaEhab == true){FWEcPaEhab = false;}
      if(ZDgKUOPpRR == true){ZDgKUOPpRR = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JBUAFYQWYL
{ 
  void rUeAwjEcCh()
  { 
      bool PYhONUmUPX = false;
      bool mXOVuQXtQT = false;
      bool PqFdMMIuqe = false;
      bool sNPebMEkXL = false;
      bool RBpumwWUcq = false;
      bool fPWHmXTDCB = false;
      bool MqdjeZHjsh = false;
      bool LlWFBOAKpQ = false;
      bool omRzzcqIqE = false;
      bool sAiWyaWGPN = false;
      bool nmbufssQFY = false;
      bool WJNkLYZejB = false;
      bool izrooKDAGa = false;
      bool qttccCKWTG = false;
      bool jZDRgimxKY = false;
      bool ArVOenumGU = false;
      bool psKdhcPjIJ = false;
      bool ODLdRIwZIF = false;
      bool XFogazXeyn = false;
      bool GWPzQpPYjD = false;
      string IBydjWtoqe;
      string XRxVLEhZmX;
      string YFgLaddRqq;
      string XCBRZmDGRp;
      string AefQsomUsS;
      string DbdUeDTlDG;
      string sbyzHUQewY;
      string BaHxXsYPSk;
      string pJYXnwYfmV;
      string SQDLxXggJX;
      string XHYTRbTgwk;
      string eETRPjDZLK;
      string wjrZGjXwzA;
      string LrFuNijEGM;
      string brjBqNxkuN;
      string TGUzbmOyNh;
      string DXgwZmyXDg;
      string zzTziDAqir;
      string llAlfFWGSZ;
      string TlqYIDKmbD;
      if(IBydjWtoqe == XHYTRbTgwk){PYhONUmUPX = true;}
      else if(XHYTRbTgwk == IBydjWtoqe){nmbufssQFY = true;}
      if(XRxVLEhZmX == eETRPjDZLK){mXOVuQXtQT = true;}
      else if(eETRPjDZLK == XRxVLEhZmX){WJNkLYZejB = true;}
      if(YFgLaddRqq == wjrZGjXwzA){PqFdMMIuqe = true;}
      else if(wjrZGjXwzA == YFgLaddRqq){izrooKDAGa = true;}
      if(XCBRZmDGRp == LrFuNijEGM){sNPebMEkXL = true;}
      else if(LrFuNijEGM == XCBRZmDGRp){qttccCKWTG = true;}
      if(AefQsomUsS == brjBqNxkuN){RBpumwWUcq = true;}
      else if(brjBqNxkuN == AefQsomUsS){jZDRgimxKY = true;}
      if(DbdUeDTlDG == TGUzbmOyNh){fPWHmXTDCB = true;}
      else if(TGUzbmOyNh == DbdUeDTlDG){ArVOenumGU = true;}
      if(sbyzHUQewY == DXgwZmyXDg){MqdjeZHjsh = true;}
      else if(DXgwZmyXDg == sbyzHUQewY){psKdhcPjIJ = true;}
      if(BaHxXsYPSk == zzTziDAqir){LlWFBOAKpQ = true;}
      if(pJYXnwYfmV == llAlfFWGSZ){omRzzcqIqE = true;}
      if(SQDLxXggJX == TlqYIDKmbD){sAiWyaWGPN = true;}
      while(zzTziDAqir == BaHxXsYPSk){ODLdRIwZIF = true;}
      while(llAlfFWGSZ == llAlfFWGSZ){XFogazXeyn = true;}
      while(TlqYIDKmbD == TlqYIDKmbD){GWPzQpPYjD = true;}
      if(PYhONUmUPX == true){PYhONUmUPX = false;}
      if(mXOVuQXtQT == true){mXOVuQXtQT = false;}
      if(PqFdMMIuqe == true){PqFdMMIuqe = false;}
      if(sNPebMEkXL == true){sNPebMEkXL = false;}
      if(RBpumwWUcq == true){RBpumwWUcq = false;}
      if(fPWHmXTDCB == true){fPWHmXTDCB = false;}
      if(MqdjeZHjsh == true){MqdjeZHjsh = false;}
      if(LlWFBOAKpQ == true){LlWFBOAKpQ = false;}
      if(omRzzcqIqE == true){omRzzcqIqE = false;}
      if(sAiWyaWGPN == true){sAiWyaWGPN = false;}
      if(nmbufssQFY == true){nmbufssQFY = false;}
      if(WJNkLYZejB == true){WJNkLYZejB = false;}
      if(izrooKDAGa == true){izrooKDAGa = false;}
      if(qttccCKWTG == true){qttccCKWTG = false;}
      if(jZDRgimxKY == true){jZDRgimxKY = false;}
      if(ArVOenumGU == true){ArVOenumGU = false;}
      if(psKdhcPjIJ == true){psKdhcPjIJ = false;}
      if(ODLdRIwZIF == true){ODLdRIwZIF = false;}
      if(XFogazXeyn == true){XFogazXeyn = false;}
      if(GWPzQpPYjD == true){GWPzQpPYjD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VDWNVKRCFB
{ 
  void AjEDyMAVVa()
  { 
      bool HrkxJftZkn = false;
      bool VCVqOhcpAy = false;
      bool pFecdGKBrX = false;
      bool leMfgdqXKk = false;
      bool fBbXJmJSSK = false;
      bool pYwSiCNISC = false;
      bool VrgMCqopJk = false;
      bool qFeORocHTq = false;
      bool xrihhMitwe = false;
      bool QiEHPmTOUS = false;
      bool yTIhTjsgBt = false;
      bool YGnbFcJzdo = false;
      bool IuseUHAXzd = false;
      bool UNlBrciANx = false;
      bool TBrCFKtcCl = false;
      bool dUjfJCadNL = false;
      bool jVhWZLTmib = false;
      bool tIkfUGCqYJ = false;
      bool MXUMAzcFqp = false;
      bool CEmCNySVbt = false;
      string OQNHQklxBw;
      string MLRlWOpJit;
      string gUdGBGFkiY;
      string hlTfqVhlQY;
      string NweBphkVxX;
      string ldlSOtlHFK;
      string fAOcNarktA;
      string fDrKgkocPT;
      string UinLlDAARw;
      string jafGsNgCmO;
      string ozaxqhkakc;
      string EIqlowLDpJ;
      string ZqTyQFWlQS;
      string EmpAdVIqTE;
      string SYOoqaitQx;
      string BdLcoyfcgl;
      string LzKRXRSpIT;
      string mUilqAbdGo;
      string WcVTRZaNLe;
      string ofJhsTOxQZ;
      if(OQNHQklxBw == ozaxqhkakc){HrkxJftZkn = true;}
      else if(ozaxqhkakc == OQNHQklxBw){yTIhTjsgBt = true;}
      if(MLRlWOpJit == EIqlowLDpJ){VCVqOhcpAy = true;}
      else if(EIqlowLDpJ == MLRlWOpJit){YGnbFcJzdo = true;}
      if(gUdGBGFkiY == ZqTyQFWlQS){pFecdGKBrX = true;}
      else if(ZqTyQFWlQS == gUdGBGFkiY){IuseUHAXzd = true;}
      if(hlTfqVhlQY == EmpAdVIqTE){leMfgdqXKk = true;}
      else if(EmpAdVIqTE == hlTfqVhlQY){UNlBrciANx = true;}
      if(NweBphkVxX == SYOoqaitQx){fBbXJmJSSK = true;}
      else if(SYOoqaitQx == NweBphkVxX){TBrCFKtcCl = true;}
      if(ldlSOtlHFK == BdLcoyfcgl){pYwSiCNISC = true;}
      else if(BdLcoyfcgl == ldlSOtlHFK){dUjfJCadNL = true;}
      if(fAOcNarktA == LzKRXRSpIT){VrgMCqopJk = true;}
      else if(LzKRXRSpIT == fAOcNarktA){jVhWZLTmib = true;}
      if(fDrKgkocPT == mUilqAbdGo){qFeORocHTq = true;}
      if(UinLlDAARw == WcVTRZaNLe){xrihhMitwe = true;}
      if(jafGsNgCmO == ofJhsTOxQZ){QiEHPmTOUS = true;}
      while(mUilqAbdGo == fDrKgkocPT){tIkfUGCqYJ = true;}
      while(WcVTRZaNLe == WcVTRZaNLe){MXUMAzcFqp = true;}
      while(ofJhsTOxQZ == ofJhsTOxQZ){CEmCNySVbt = true;}
      if(HrkxJftZkn == true){HrkxJftZkn = false;}
      if(VCVqOhcpAy == true){VCVqOhcpAy = false;}
      if(pFecdGKBrX == true){pFecdGKBrX = false;}
      if(leMfgdqXKk == true){leMfgdqXKk = false;}
      if(fBbXJmJSSK == true){fBbXJmJSSK = false;}
      if(pYwSiCNISC == true){pYwSiCNISC = false;}
      if(VrgMCqopJk == true){VrgMCqopJk = false;}
      if(qFeORocHTq == true){qFeORocHTq = false;}
      if(xrihhMitwe == true){xrihhMitwe = false;}
      if(QiEHPmTOUS == true){QiEHPmTOUS = false;}
      if(yTIhTjsgBt == true){yTIhTjsgBt = false;}
      if(YGnbFcJzdo == true){YGnbFcJzdo = false;}
      if(IuseUHAXzd == true){IuseUHAXzd = false;}
      if(UNlBrciANx == true){UNlBrciANx = false;}
      if(TBrCFKtcCl == true){TBrCFKtcCl = false;}
      if(dUjfJCadNL == true){dUjfJCadNL = false;}
      if(jVhWZLTmib == true){jVhWZLTmib = false;}
      if(tIkfUGCqYJ == true){tIkfUGCqYJ = false;}
      if(MXUMAzcFqp == true){MXUMAzcFqp = false;}
      if(CEmCNySVbt == true){CEmCNySVbt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WXGYIQOGLC
{ 
  void cROZtVAuju()
  { 
      bool kujpataNpo = false;
      bool HwDsFkxPqp = false;
      bool LairXHBmqX = false;
      bool UYXkhIxiLI = false;
      bool IuizwPLsFp = false;
      bool ZIkdpJLqRT = false;
      bool smWRUiYSLx = false;
      bool nODhhPriuH = false;
      bool uHgAxMmstX = false;
      bool wFQqPUYQeN = false;
      bool qshICLEDwZ = false;
      bool RcwKXkGWUI = false;
      bool weXmAiyLSN = false;
      bool qaDhQCYYxl = false;
      bool HjxUGSJGTd = false;
      bool OCwWRLUuHT = false;
      bool NrjiiqYNsV = false;
      bool MgYhALhDfh = false;
      bool YqrozndgQF = false;
      bool hdQcXOzOkP = false;
      string NRaMornsCE;
      string sbmZpWFoYr;
      string YRUGGYFgVO;
      string yMyUeqjycM;
      string fShhgKtxPS;
      string mhzspuwUGQ;
      string lGXnGfcRty;
      string lGLMLFjlYu;
      string dIRCaQNEei;
      string tPrMaYusbB;
      string hbCQggVmhj;
      string LKwKlcoUMp;
      string SXmLDWOzkm;
      string KpKXNBGAVt;
      string tZWtXjTgZy;
      string iafkhJoqMJ;
      string RWCtZVzXxj;
      string WozSurBrmS;
      string DgYmDiJYoE;
      string MPmLbXXbUC;
      if(NRaMornsCE == hbCQggVmhj){kujpataNpo = true;}
      else if(hbCQggVmhj == NRaMornsCE){qshICLEDwZ = true;}
      if(sbmZpWFoYr == LKwKlcoUMp){HwDsFkxPqp = true;}
      else if(LKwKlcoUMp == sbmZpWFoYr){RcwKXkGWUI = true;}
      if(YRUGGYFgVO == SXmLDWOzkm){LairXHBmqX = true;}
      else if(SXmLDWOzkm == YRUGGYFgVO){weXmAiyLSN = true;}
      if(yMyUeqjycM == KpKXNBGAVt){UYXkhIxiLI = true;}
      else if(KpKXNBGAVt == yMyUeqjycM){qaDhQCYYxl = true;}
      if(fShhgKtxPS == tZWtXjTgZy){IuizwPLsFp = true;}
      else if(tZWtXjTgZy == fShhgKtxPS){HjxUGSJGTd = true;}
      if(mhzspuwUGQ == iafkhJoqMJ){ZIkdpJLqRT = true;}
      else if(iafkhJoqMJ == mhzspuwUGQ){OCwWRLUuHT = true;}
      if(lGXnGfcRty == RWCtZVzXxj){smWRUiYSLx = true;}
      else if(RWCtZVzXxj == lGXnGfcRty){NrjiiqYNsV = true;}
      if(lGLMLFjlYu == WozSurBrmS){nODhhPriuH = true;}
      if(dIRCaQNEei == DgYmDiJYoE){uHgAxMmstX = true;}
      if(tPrMaYusbB == MPmLbXXbUC){wFQqPUYQeN = true;}
      while(WozSurBrmS == lGLMLFjlYu){MgYhALhDfh = true;}
      while(DgYmDiJYoE == DgYmDiJYoE){YqrozndgQF = true;}
      while(MPmLbXXbUC == MPmLbXXbUC){hdQcXOzOkP = true;}
      if(kujpataNpo == true){kujpataNpo = false;}
      if(HwDsFkxPqp == true){HwDsFkxPqp = false;}
      if(LairXHBmqX == true){LairXHBmqX = false;}
      if(UYXkhIxiLI == true){UYXkhIxiLI = false;}
      if(IuizwPLsFp == true){IuizwPLsFp = false;}
      if(ZIkdpJLqRT == true){ZIkdpJLqRT = false;}
      if(smWRUiYSLx == true){smWRUiYSLx = false;}
      if(nODhhPriuH == true){nODhhPriuH = false;}
      if(uHgAxMmstX == true){uHgAxMmstX = false;}
      if(wFQqPUYQeN == true){wFQqPUYQeN = false;}
      if(qshICLEDwZ == true){qshICLEDwZ = false;}
      if(RcwKXkGWUI == true){RcwKXkGWUI = false;}
      if(weXmAiyLSN == true){weXmAiyLSN = false;}
      if(qaDhQCYYxl == true){qaDhQCYYxl = false;}
      if(HjxUGSJGTd == true){HjxUGSJGTd = false;}
      if(OCwWRLUuHT == true){OCwWRLUuHT = false;}
      if(NrjiiqYNsV == true){NrjiiqYNsV = false;}
      if(MgYhALhDfh == true){MgYhALhDfh = false;}
      if(YqrozndgQF == true){YqrozndgQF = false;}
      if(hdQcXOzOkP == true){hdQcXOzOkP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EZJBLFPRFJ
{ 
  void bHEogsRkmr()
  { 
      bool ZhQgijwNqi = false;
      bool UehuWpjSXS = false;
      bool IcCokwgURX = false;
      bool HbxdCMMYKI = false;
      bool abnzLnjdBJ = false;
      bool uxPYhxebwm = false;
      bool CbwSbrhEsK = false;
      bool cLHPrfnDdw = false;
      bool LCTHcKMXuE = false;
      bool gkbXFzeVXS = false;
      bool FfkqfjozgC = false;
      bool JbyAaAHnpR = false;
      bool fKIncrijJr = false;
      bool ASeKqfRbbs = false;
      bool UMqsuFlbxF = false;
      bool qqeGmuWwNm = false;
      bool AEpnBpwmJt = false;
      bool heMwwTSPrX = false;
      bool FHmEBJmYXu = false;
      bool IUOorPVCqE = false;
      string uZHuDTTOKt;
      string jzqhVbKZLB;
      string TowirfHmJW;
      string NEcgPtSZCp;
      string ZeDBlZBcIn;
      string UHXZsGdXDW;
      string LgEmgfVKSa;
      string IkXUanluwS;
      string eddUCGfCmJ;
      string QuVedrZPUG;
      string DqKheijDyE;
      string ThajrgCjkZ;
      string zTWIKNOrph;
      string OVZoPnTwpT;
      string WbDhohTklw;
      string ppetGjTBWR;
      string TqMQpyIkUE;
      string NGsQDXPnTb;
      string tdGsrXpfif;
      string TRYYzePmqp;
      if(uZHuDTTOKt == DqKheijDyE){ZhQgijwNqi = true;}
      else if(DqKheijDyE == uZHuDTTOKt){FfkqfjozgC = true;}
      if(jzqhVbKZLB == ThajrgCjkZ){UehuWpjSXS = true;}
      else if(ThajrgCjkZ == jzqhVbKZLB){JbyAaAHnpR = true;}
      if(TowirfHmJW == zTWIKNOrph){IcCokwgURX = true;}
      else if(zTWIKNOrph == TowirfHmJW){fKIncrijJr = true;}
      if(NEcgPtSZCp == OVZoPnTwpT){HbxdCMMYKI = true;}
      else if(OVZoPnTwpT == NEcgPtSZCp){ASeKqfRbbs = true;}
      if(ZeDBlZBcIn == WbDhohTklw){abnzLnjdBJ = true;}
      else if(WbDhohTklw == ZeDBlZBcIn){UMqsuFlbxF = true;}
      if(UHXZsGdXDW == ppetGjTBWR){uxPYhxebwm = true;}
      else if(ppetGjTBWR == UHXZsGdXDW){qqeGmuWwNm = true;}
      if(LgEmgfVKSa == TqMQpyIkUE){CbwSbrhEsK = true;}
      else if(TqMQpyIkUE == LgEmgfVKSa){AEpnBpwmJt = true;}
      if(IkXUanluwS == NGsQDXPnTb){cLHPrfnDdw = true;}
      if(eddUCGfCmJ == tdGsrXpfif){LCTHcKMXuE = true;}
      if(QuVedrZPUG == TRYYzePmqp){gkbXFzeVXS = true;}
      while(NGsQDXPnTb == IkXUanluwS){heMwwTSPrX = true;}
      while(tdGsrXpfif == tdGsrXpfif){FHmEBJmYXu = true;}
      while(TRYYzePmqp == TRYYzePmqp){IUOorPVCqE = true;}
      if(ZhQgijwNqi == true){ZhQgijwNqi = false;}
      if(UehuWpjSXS == true){UehuWpjSXS = false;}
      if(IcCokwgURX == true){IcCokwgURX = false;}
      if(HbxdCMMYKI == true){HbxdCMMYKI = false;}
      if(abnzLnjdBJ == true){abnzLnjdBJ = false;}
      if(uxPYhxebwm == true){uxPYhxebwm = false;}
      if(CbwSbrhEsK == true){CbwSbrhEsK = false;}
      if(cLHPrfnDdw == true){cLHPrfnDdw = false;}
      if(LCTHcKMXuE == true){LCTHcKMXuE = false;}
      if(gkbXFzeVXS == true){gkbXFzeVXS = false;}
      if(FfkqfjozgC == true){FfkqfjozgC = false;}
      if(JbyAaAHnpR == true){JbyAaAHnpR = false;}
      if(fKIncrijJr == true){fKIncrijJr = false;}
      if(ASeKqfRbbs == true){ASeKqfRbbs = false;}
      if(UMqsuFlbxF == true){UMqsuFlbxF = false;}
      if(qqeGmuWwNm == true){qqeGmuWwNm = false;}
      if(AEpnBpwmJt == true){AEpnBpwmJt = false;}
      if(heMwwTSPrX == true){heMwwTSPrX = false;}
      if(FHmEBJmYXu == true){FHmEBJmYXu = false;}
      if(IUOorPVCqE == true){IUOorPVCqE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TCKWWACTQC
{ 
  void cKWuzKSopu()
  { 
      bool ScwSdQjSiw = false;
      bool ySRusXjpaU = false;
      bool RPAkfnpRni = false;
      bool OCIcfSFqDU = false;
      bool MDlIFbkJiy = false;
      bool LcSNBCbLmo = false;
      bool aRezgwJnPa = false;
      bool RrmGHNdqeD = false;
      bool CfhpXliuac = false;
      bool qtENttYlbI = false;
      bool RxtIfSpMFn = false;
      bool Yjfjmtjnbu = false;
      bool NOMmjWnYLM = false;
      bool nqPVkFlZJE = false;
      bool uVomDGOTaA = false;
      bool KjKguDjGYI = false;
      bool dRBrzhTJyg = false;
      bool HKdWzqZZjc = false;
      bool ITyLCEfiah = false;
      bool UfABadUwmU = false;
      string LBUFmyUEEi;
      string RJzWrJXwGI;
      string KlNmUzEaGm;
      string WZlLVOGKPY;
      string BoCuxWqTcV;
      string bgISPYngkn;
      string tsxRLrXnAd;
      string DMfhGUjSpx;
      string HjyjKpRTuw;
      string ZZYaeIrjiF;
      string yCSySOAZLt;
      string nUpnfJuMGU;
      string ZDrEDdnFCy;
      string cAijyFRzRw;
      string pqHAHEiktV;
      string MecNeuKOtn;
      string auXMRnlGoa;
      string quuKeiwqwy;
      string uCKglxWVFD;
      string uDFcVuTtiS;
      if(LBUFmyUEEi == yCSySOAZLt){ScwSdQjSiw = true;}
      else if(yCSySOAZLt == LBUFmyUEEi){RxtIfSpMFn = true;}
      if(RJzWrJXwGI == nUpnfJuMGU){ySRusXjpaU = true;}
      else if(nUpnfJuMGU == RJzWrJXwGI){Yjfjmtjnbu = true;}
      if(KlNmUzEaGm == ZDrEDdnFCy){RPAkfnpRni = true;}
      else if(ZDrEDdnFCy == KlNmUzEaGm){NOMmjWnYLM = true;}
      if(WZlLVOGKPY == cAijyFRzRw){OCIcfSFqDU = true;}
      else if(cAijyFRzRw == WZlLVOGKPY){nqPVkFlZJE = true;}
      if(BoCuxWqTcV == pqHAHEiktV){MDlIFbkJiy = true;}
      else if(pqHAHEiktV == BoCuxWqTcV){uVomDGOTaA = true;}
      if(bgISPYngkn == MecNeuKOtn){LcSNBCbLmo = true;}
      else if(MecNeuKOtn == bgISPYngkn){KjKguDjGYI = true;}
      if(tsxRLrXnAd == auXMRnlGoa){aRezgwJnPa = true;}
      else if(auXMRnlGoa == tsxRLrXnAd){dRBrzhTJyg = true;}
      if(DMfhGUjSpx == quuKeiwqwy){RrmGHNdqeD = true;}
      if(HjyjKpRTuw == uCKglxWVFD){CfhpXliuac = true;}
      if(ZZYaeIrjiF == uDFcVuTtiS){qtENttYlbI = true;}
      while(quuKeiwqwy == DMfhGUjSpx){HKdWzqZZjc = true;}
      while(uCKglxWVFD == uCKglxWVFD){ITyLCEfiah = true;}
      while(uDFcVuTtiS == uDFcVuTtiS){UfABadUwmU = true;}
      if(ScwSdQjSiw == true){ScwSdQjSiw = false;}
      if(ySRusXjpaU == true){ySRusXjpaU = false;}
      if(RPAkfnpRni == true){RPAkfnpRni = false;}
      if(OCIcfSFqDU == true){OCIcfSFqDU = false;}
      if(MDlIFbkJiy == true){MDlIFbkJiy = false;}
      if(LcSNBCbLmo == true){LcSNBCbLmo = false;}
      if(aRezgwJnPa == true){aRezgwJnPa = false;}
      if(RrmGHNdqeD == true){RrmGHNdqeD = false;}
      if(CfhpXliuac == true){CfhpXliuac = false;}
      if(qtENttYlbI == true){qtENttYlbI = false;}
      if(RxtIfSpMFn == true){RxtIfSpMFn = false;}
      if(Yjfjmtjnbu == true){Yjfjmtjnbu = false;}
      if(NOMmjWnYLM == true){NOMmjWnYLM = false;}
      if(nqPVkFlZJE == true){nqPVkFlZJE = false;}
      if(uVomDGOTaA == true){uVomDGOTaA = false;}
      if(KjKguDjGYI == true){KjKguDjGYI = false;}
      if(dRBrzhTJyg == true){dRBrzhTJyg = false;}
      if(HKdWzqZZjc == true){HKdWzqZZjc = false;}
      if(ITyLCEfiah == true){ITyLCEfiah = false;}
      if(UfABadUwmU == true){UfABadUwmU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PRKOYBAONB
{ 
  void lmhkRJSHkE()
  { 
      bool rWFSQRHhHs = false;
      bool QykxSTaoAu = false;
      bool lAifRlSmVD = false;
      bool XzGrhQbLYL = false;
      bool XcOYgIOIfM = false;
      bool MsdIiqmZAe = false;
      bool zUMEUQrrOB = false;
      bool qOQoXJkKyJ = false;
      bool WJGJqeUxIq = false;
      bool dCLsMCQKDP = false;
      bool PWteUxbokZ = false;
      bool GcaqHoNCNp = false;
      bool wtiXXoDWbU = false;
      bool rZGisAPbos = false;
      bool DeLijBbTkW = false;
      bool oOxNTUnVHf = false;
      bool sEMQhgVqyH = false;
      bool ZkNTmQiAnW = false;
      bool NjlFecbsOX = false;
      bool MsUIzySuAC = false;
      string bGSMupCNKI;
      string sdTdOEUtpI;
      string UPCLNmUGwe;
      string BncwcOzBpJ;
      string qFzsYkWkOW;
      string hfDRPLEGyH;
      string VwReHEAWAA;
      string ESEfROQJmc;
      string QjrYwVkKgQ;
      string KrDhnunTlT;
      string jzjNSPbWAB;
      string uuTUpkDsik;
      string TopjpLOVrV;
      string awXSrQzdFf;
      string sNKeUPXhqC;
      string qukAAuJAXG;
      string AFeeayXBcY;
      string lZmCrGeKOZ;
      string mgdodfLWSx;
      string HzGzfnheEV;
      if(bGSMupCNKI == jzjNSPbWAB){rWFSQRHhHs = true;}
      else if(jzjNSPbWAB == bGSMupCNKI){PWteUxbokZ = true;}
      if(sdTdOEUtpI == uuTUpkDsik){QykxSTaoAu = true;}
      else if(uuTUpkDsik == sdTdOEUtpI){GcaqHoNCNp = true;}
      if(UPCLNmUGwe == TopjpLOVrV){lAifRlSmVD = true;}
      else if(TopjpLOVrV == UPCLNmUGwe){wtiXXoDWbU = true;}
      if(BncwcOzBpJ == awXSrQzdFf){XzGrhQbLYL = true;}
      else if(awXSrQzdFf == BncwcOzBpJ){rZGisAPbos = true;}
      if(qFzsYkWkOW == sNKeUPXhqC){XcOYgIOIfM = true;}
      else if(sNKeUPXhqC == qFzsYkWkOW){DeLijBbTkW = true;}
      if(hfDRPLEGyH == qukAAuJAXG){MsdIiqmZAe = true;}
      else if(qukAAuJAXG == hfDRPLEGyH){oOxNTUnVHf = true;}
      if(VwReHEAWAA == AFeeayXBcY){zUMEUQrrOB = true;}
      else if(AFeeayXBcY == VwReHEAWAA){sEMQhgVqyH = true;}
      if(ESEfROQJmc == lZmCrGeKOZ){qOQoXJkKyJ = true;}
      if(QjrYwVkKgQ == mgdodfLWSx){WJGJqeUxIq = true;}
      if(KrDhnunTlT == HzGzfnheEV){dCLsMCQKDP = true;}
      while(lZmCrGeKOZ == ESEfROQJmc){ZkNTmQiAnW = true;}
      while(mgdodfLWSx == mgdodfLWSx){NjlFecbsOX = true;}
      while(HzGzfnheEV == HzGzfnheEV){MsUIzySuAC = true;}
      if(rWFSQRHhHs == true){rWFSQRHhHs = false;}
      if(QykxSTaoAu == true){QykxSTaoAu = false;}
      if(lAifRlSmVD == true){lAifRlSmVD = false;}
      if(XzGrhQbLYL == true){XzGrhQbLYL = false;}
      if(XcOYgIOIfM == true){XcOYgIOIfM = false;}
      if(MsdIiqmZAe == true){MsdIiqmZAe = false;}
      if(zUMEUQrrOB == true){zUMEUQrrOB = false;}
      if(qOQoXJkKyJ == true){qOQoXJkKyJ = false;}
      if(WJGJqeUxIq == true){WJGJqeUxIq = false;}
      if(dCLsMCQKDP == true){dCLsMCQKDP = false;}
      if(PWteUxbokZ == true){PWteUxbokZ = false;}
      if(GcaqHoNCNp == true){GcaqHoNCNp = false;}
      if(wtiXXoDWbU == true){wtiXXoDWbU = false;}
      if(rZGisAPbos == true){rZGisAPbos = false;}
      if(DeLijBbTkW == true){DeLijBbTkW = false;}
      if(oOxNTUnVHf == true){oOxNTUnVHf = false;}
      if(sEMQhgVqyH == true){sEMQhgVqyH = false;}
      if(ZkNTmQiAnW == true){ZkNTmQiAnW = false;}
      if(NjlFecbsOX == true){NjlFecbsOX = false;}
      if(MsUIzySuAC == true){MsUIzySuAC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YZSGVJOMNP
{ 
  void sdzFKYlfUQ()
  { 
      bool edgOxsmZeD = false;
      bool PbxmbcPHYn = false;
      bool gBHMzKccbK = false;
      bool LXbaVsntSq = false;
      bool lSdYPQlabq = false;
      bool YYOBIcVTOl = false;
      bool QJxUqrAWQi = false;
      bool oUnhBOXhwh = false;
      bool XCBjlmIaRr = false;
      bool WTffabVIhX = false;
      bool QPJsCDjgzt = false;
      bool FmWfNNINiE = false;
      bool iYEYLUoIGY = false;
      bool sOAYRIrIgh = false;
      bool CaaWmjwzSM = false;
      bool THZcnqLkgh = false;
      bool ObaZBplyOF = false;
      bool VAfABIygkb = false;
      bool XoptFkoXkZ = false;
      bool xHazTaAAVx = false;
      string FHfEsTRCtQ;
      string zjGkuOfkTy;
      string aOgAMGerfT;
      string COAWxPEOdY;
      string BaMEfUBjKs;
      string IybYIqaeqm;
      string nZVaOkjjnf;
      string JQHtwzBcUr;
      string FtglKDlaNF;
      string nYXpkbElou;
      string kQweiqpgcJ;
      string oyKyhsrjPL;
      string yJWhqksRIT;
      string uOmZGnyBCS;
      string MFxEQpOYIr;
      string iUWdwbtOny;
      string yYXJOZKkrs;
      string tpgwysLRQB;
      string WUJQjZrJce;
      string sjNEUngnpp;
      if(FHfEsTRCtQ == kQweiqpgcJ){edgOxsmZeD = true;}
      else if(kQweiqpgcJ == FHfEsTRCtQ){QPJsCDjgzt = true;}
      if(zjGkuOfkTy == oyKyhsrjPL){PbxmbcPHYn = true;}
      else if(oyKyhsrjPL == zjGkuOfkTy){FmWfNNINiE = true;}
      if(aOgAMGerfT == yJWhqksRIT){gBHMzKccbK = true;}
      else if(yJWhqksRIT == aOgAMGerfT){iYEYLUoIGY = true;}
      if(COAWxPEOdY == uOmZGnyBCS){LXbaVsntSq = true;}
      else if(uOmZGnyBCS == COAWxPEOdY){sOAYRIrIgh = true;}
      if(BaMEfUBjKs == MFxEQpOYIr){lSdYPQlabq = true;}
      else if(MFxEQpOYIr == BaMEfUBjKs){CaaWmjwzSM = true;}
      if(IybYIqaeqm == iUWdwbtOny){YYOBIcVTOl = true;}
      else if(iUWdwbtOny == IybYIqaeqm){THZcnqLkgh = true;}
      if(nZVaOkjjnf == yYXJOZKkrs){QJxUqrAWQi = true;}
      else if(yYXJOZKkrs == nZVaOkjjnf){ObaZBplyOF = true;}
      if(JQHtwzBcUr == tpgwysLRQB){oUnhBOXhwh = true;}
      if(FtglKDlaNF == WUJQjZrJce){XCBjlmIaRr = true;}
      if(nYXpkbElou == sjNEUngnpp){WTffabVIhX = true;}
      while(tpgwysLRQB == JQHtwzBcUr){VAfABIygkb = true;}
      while(WUJQjZrJce == WUJQjZrJce){XoptFkoXkZ = true;}
      while(sjNEUngnpp == sjNEUngnpp){xHazTaAAVx = true;}
      if(edgOxsmZeD == true){edgOxsmZeD = false;}
      if(PbxmbcPHYn == true){PbxmbcPHYn = false;}
      if(gBHMzKccbK == true){gBHMzKccbK = false;}
      if(LXbaVsntSq == true){LXbaVsntSq = false;}
      if(lSdYPQlabq == true){lSdYPQlabq = false;}
      if(YYOBIcVTOl == true){YYOBIcVTOl = false;}
      if(QJxUqrAWQi == true){QJxUqrAWQi = false;}
      if(oUnhBOXhwh == true){oUnhBOXhwh = false;}
      if(XCBjlmIaRr == true){XCBjlmIaRr = false;}
      if(WTffabVIhX == true){WTffabVIhX = false;}
      if(QPJsCDjgzt == true){QPJsCDjgzt = false;}
      if(FmWfNNINiE == true){FmWfNNINiE = false;}
      if(iYEYLUoIGY == true){iYEYLUoIGY = false;}
      if(sOAYRIrIgh == true){sOAYRIrIgh = false;}
      if(CaaWmjwzSM == true){CaaWmjwzSM = false;}
      if(THZcnqLkgh == true){THZcnqLkgh = false;}
      if(ObaZBplyOF == true){ObaZBplyOF = false;}
      if(VAfABIygkb == true){VAfABIygkb = false;}
      if(XoptFkoXkZ == true){XoptFkoXkZ = false;}
      if(xHazTaAAVx == true){xHazTaAAVx = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TNCQGZMIJE
{ 
  void RDaFldsEFw()
  { 
      bool cKuVpDyIFn = false;
      bool ddssEnIhjn = false;
      bool BYbUQeLNFU = false;
      bool mBSocmUIcd = false;
      bool uXpUBMbWXS = false;
      bool TRQCpEiZEK = false;
      bool rkCkAcLDPR = false;
      bool TAFFrdhHSE = false;
      bool DoolAVjQzY = false;
      bool DtDRlJUETX = false;
      bool BXGGzTLKJL = false;
      bool VbHAHDwQje = false;
      bool suEqNZwnTi = false;
      bool JcmuDehimf = false;
      bool JYGmCbmCJh = false;
      bool NDUodFTPrY = false;
      bool XXpaCsnLxc = false;
      bool UNdDUUnQnl = false;
      bool mselVFIyQx = false;
      bool LSpNIlhekJ = false;
      string LkBOkBFlFA;
      string SkbyYHDfHS;
      string FjBYlwatmH;
      string dQPDiCtjGM;
      string hKpkUWuxaK;
      string hUEQMbjbbn;
      string ZLzxVBUYOW;
      string MSZukusPEO;
      string BdTcZJLzpH;
      string RSIRlWLrPS;
      string CVhExOtJfR;
      string hUWMmwqVXM;
      string pVhHEtKIIN;
      string iAzlMewPfL;
      string OBTYZxwudp;
      string foUDcgnazC;
      string DlBRhgUkAI;
      string AbpMxnCJgG;
      string yPdShtfwIf;
      string qUPcHbABpr;
      if(LkBOkBFlFA == CVhExOtJfR){cKuVpDyIFn = true;}
      else if(CVhExOtJfR == LkBOkBFlFA){BXGGzTLKJL = true;}
      if(SkbyYHDfHS == hUWMmwqVXM){ddssEnIhjn = true;}
      else if(hUWMmwqVXM == SkbyYHDfHS){VbHAHDwQje = true;}
      if(FjBYlwatmH == pVhHEtKIIN){BYbUQeLNFU = true;}
      else if(pVhHEtKIIN == FjBYlwatmH){suEqNZwnTi = true;}
      if(dQPDiCtjGM == iAzlMewPfL){mBSocmUIcd = true;}
      else if(iAzlMewPfL == dQPDiCtjGM){JcmuDehimf = true;}
      if(hKpkUWuxaK == OBTYZxwudp){uXpUBMbWXS = true;}
      else if(OBTYZxwudp == hKpkUWuxaK){JYGmCbmCJh = true;}
      if(hUEQMbjbbn == foUDcgnazC){TRQCpEiZEK = true;}
      else if(foUDcgnazC == hUEQMbjbbn){NDUodFTPrY = true;}
      if(ZLzxVBUYOW == DlBRhgUkAI){rkCkAcLDPR = true;}
      else if(DlBRhgUkAI == ZLzxVBUYOW){XXpaCsnLxc = true;}
      if(MSZukusPEO == AbpMxnCJgG){TAFFrdhHSE = true;}
      if(BdTcZJLzpH == yPdShtfwIf){DoolAVjQzY = true;}
      if(RSIRlWLrPS == qUPcHbABpr){DtDRlJUETX = true;}
      while(AbpMxnCJgG == MSZukusPEO){UNdDUUnQnl = true;}
      while(yPdShtfwIf == yPdShtfwIf){mselVFIyQx = true;}
      while(qUPcHbABpr == qUPcHbABpr){LSpNIlhekJ = true;}
      if(cKuVpDyIFn == true){cKuVpDyIFn = false;}
      if(ddssEnIhjn == true){ddssEnIhjn = false;}
      if(BYbUQeLNFU == true){BYbUQeLNFU = false;}
      if(mBSocmUIcd == true){mBSocmUIcd = false;}
      if(uXpUBMbWXS == true){uXpUBMbWXS = false;}
      if(TRQCpEiZEK == true){TRQCpEiZEK = false;}
      if(rkCkAcLDPR == true){rkCkAcLDPR = false;}
      if(TAFFrdhHSE == true){TAFFrdhHSE = false;}
      if(DoolAVjQzY == true){DoolAVjQzY = false;}
      if(DtDRlJUETX == true){DtDRlJUETX = false;}
      if(BXGGzTLKJL == true){BXGGzTLKJL = false;}
      if(VbHAHDwQje == true){VbHAHDwQje = false;}
      if(suEqNZwnTi == true){suEqNZwnTi = false;}
      if(JcmuDehimf == true){JcmuDehimf = false;}
      if(JYGmCbmCJh == true){JYGmCbmCJh = false;}
      if(NDUodFTPrY == true){NDUodFTPrY = false;}
      if(XXpaCsnLxc == true){XXpaCsnLxc = false;}
      if(UNdDUUnQnl == true){UNdDUUnQnl = false;}
      if(mselVFIyQx == true){mselVFIyQx = false;}
      if(LSpNIlhekJ == true){LSpNIlhekJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NTNXFGRUQG
{ 
  void lYnSjVUVCA()
  { 
      bool qbYEkhBQRq = false;
      bool KUZsrdtVad = false;
      bool GekBhIjccm = false;
      bool zRJihmYBRM = false;
      bool lYiiRrngjc = false;
      bool oTKXfMqWVc = false;
      bool xYnerrLqLE = false;
      bool ErDNqhuBjo = false;
      bool TDrzxeHlCU = false;
      bool uCeoPMkEqY = false;
      bool gxUeYxQwfy = false;
      bool RomfIsfngM = false;
      bool kZnqJCMNye = false;
      bool xpKKIWiMaz = false;
      bool OLFECFOzgV = false;
      bool fwWdmGIcVW = false;
      bool jCyABbebWk = false;
      bool KSNgxZTjnD = false;
      bool mNgbyASfXC = false;
      bool NFqDPMNFCa = false;
      string UecBKrduxp;
      string osqHmShhqA;
      string WRZXRSwOLP;
      string tnPjbyKIjm;
      string NeDpiBQhFR;
      string DEnxECFFjr;
      string wOiguBcHeo;
      string HZyDmqNjnP;
      string CNJsMpzReY;
      string sFXqqywVth;
      string YICPKdGkKM;
      string oVRUphFoTF;
      string CKcGwCBGMn;
      string uroGpsKtii;
      string iNHSjWmDAI;
      string sHBnfNewSH;
      string bzYVRwfLdw;
      string WLFjWnDKVY;
      string PyFDunpBZb;
      string NOabHiokBm;
      if(UecBKrduxp == YICPKdGkKM){qbYEkhBQRq = true;}
      else if(YICPKdGkKM == UecBKrduxp){gxUeYxQwfy = true;}
      if(osqHmShhqA == oVRUphFoTF){KUZsrdtVad = true;}
      else if(oVRUphFoTF == osqHmShhqA){RomfIsfngM = true;}
      if(WRZXRSwOLP == CKcGwCBGMn){GekBhIjccm = true;}
      else if(CKcGwCBGMn == WRZXRSwOLP){kZnqJCMNye = true;}
      if(tnPjbyKIjm == uroGpsKtii){zRJihmYBRM = true;}
      else if(uroGpsKtii == tnPjbyKIjm){xpKKIWiMaz = true;}
      if(NeDpiBQhFR == iNHSjWmDAI){lYiiRrngjc = true;}
      else if(iNHSjWmDAI == NeDpiBQhFR){OLFECFOzgV = true;}
      if(DEnxECFFjr == sHBnfNewSH){oTKXfMqWVc = true;}
      else if(sHBnfNewSH == DEnxECFFjr){fwWdmGIcVW = true;}
      if(wOiguBcHeo == bzYVRwfLdw){xYnerrLqLE = true;}
      else if(bzYVRwfLdw == wOiguBcHeo){jCyABbebWk = true;}
      if(HZyDmqNjnP == WLFjWnDKVY){ErDNqhuBjo = true;}
      if(CNJsMpzReY == PyFDunpBZb){TDrzxeHlCU = true;}
      if(sFXqqywVth == NOabHiokBm){uCeoPMkEqY = true;}
      while(WLFjWnDKVY == HZyDmqNjnP){KSNgxZTjnD = true;}
      while(PyFDunpBZb == PyFDunpBZb){mNgbyASfXC = true;}
      while(NOabHiokBm == NOabHiokBm){NFqDPMNFCa = true;}
      if(qbYEkhBQRq == true){qbYEkhBQRq = false;}
      if(KUZsrdtVad == true){KUZsrdtVad = false;}
      if(GekBhIjccm == true){GekBhIjccm = false;}
      if(zRJihmYBRM == true){zRJihmYBRM = false;}
      if(lYiiRrngjc == true){lYiiRrngjc = false;}
      if(oTKXfMqWVc == true){oTKXfMqWVc = false;}
      if(xYnerrLqLE == true){xYnerrLqLE = false;}
      if(ErDNqhuBjo == true){ErDNqhuBjo = false;}
      if(TDrzxeHlCU == true){TDrzxeHlCU = false;}
      if(uCeoPMkEqY == true){uCeoPMkEqY = false;}
      if(gxUeYxQwfy == true){gxUeYxQwfy = false;}
      if(RomfIsfngM == true){RomfIsfngM = false;}
      if(kZnqJCMNye == true){kZnqJCMNye = false;}
      if(xpKKIWiMaz == true){xpKKIWiMaz = false;}
      if(OLFECFOzgV == true){OLFECFOzgV = false;}
      if(fwWdmGIcVW == true){fwWdmGIcVW = false;}
      if(jCyABbebWk == true){jCyABbebWk = false;}
      if(KSNgxZTjnD == true){KSNgxZTjnD = false;}
      if(mNgbyASfXC == true){mNgbyASfXC = false;}
      if(NFqDPMNFCa == true){NFqDPMNFCa = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NXTMFYQWVF
{ 
  void YarFNeHaGV()
  { 
      bool ZzuBXiwmuw = false;
      bool gyOkljIJEj = false;
      bool wOeeVRLABE = false;
      bool OSYrHMYuiS = false;
      bool fwhYwBENRE = false;
      bool jnWgkKUPfF = false;
      bool WwlqgNZWcY = false;
      bool LNZlEAgFiU = false;
      bool NyhwTawqHx = false;
      bool qIPduiJqrh = false;
      bool bhkrrWNjcj = false;
      bool dPgKrYNpWG = false;
      bool RhOfgDywpV = false;
      bool qpNAreMoEx = false;
      bool MMNWSbbElW = false;
      bool UTTkQsSFLo = false;
      bool YRtHsnBgRM = false;
      bool LXlZtFUrMF = false;
      bool asJSAWVbiz = false;
      bool LHdOXtRGde = false;
      string QwiBIsykwQ;
      string fSkaoVloID;
      string KuUjbALjQq;
      string wDFtdHdtol;
      string kaTJQQBZzC;
      string UYEVZVazZI;
      string gNTFTbMpwg;
      string qrzVSjaLTK;
      string yljNVXyjUI;
      string geTfFWGNon;
      string EKcSLbYhCp;
      string tOePJucGjw;
      string dbVnzMXTQu;
      string cNWeFnIRCs;
      string yPheORzEWJ;
      string tAdsKbooSy;
      string EFRGMgrzJU;
      string lKDpxNtpZh;
      string MTwHBbjcUd;
      string zTzDosyaRg;
      if(QwiBIsykwQ == EKcSLbYhCp){ZzuBXiwmuw = true;}
      else if(EKcSLbYhCp == QwiBIsykwQ){bhkrrWNjcj = true;}
      if(fSkaoVloID == tOePJucGjw){gyOkljIJEj = true;}
      else if(tOePJucGjw == fSkaoVloID){dPgKrYNpWG = true;}
      if(KuUjbALjQq == dbVnzMXTQu){wOeeVRLABE = true;}
      else if(dbVnzMXTQu == KuUjbALjQq){RhOfgDywpV = true;}
      if(wDFtdHdtol == cNWeFnIRCs){OSYrHMYuiS = true;}
      else if(cNWeFnIRCs == wDFtdHdtol){qpNAreMoEx = true;}
      if(kaTJQQBZzC == yPheORzEWJ){fwhYwBENRE = true;}
      else if(yPheORzEWJ == kaTJQQBZzC){MMNWSbbElW = true;}
      if(UYEVZVazZI == tAdsKbooSy){jnWgkKUPfF = true;}
      else if(tAdsKbooSy == UYEVZVazZI){UTTkQsSFLo = true;}
      if(gNTFTbMpwg == EFRGMgrzJU){WwlqgNZWcY = true;}
      else if(EFRGMgrzJU == gNTFTbMpwg){YRtHsnBgRM = true;}
      if(qrzVSjaLTK == lKDpxNtpZh){LNZlEAgFiU = true;}
      if(yljNVXyjUI == MTwHBbjcUd){NyhwTawqHx = true;}
      if(geTfFWGNon == zTzDosyaRg){qIPduiJqrh = true;}
      while(lKDpxNtpZh == qrzVSjaLTK){LXlZtFUrMF = true;}
      while(MTwHBbjcUd == MTwHBbjcUd){asJSAWVbiz = true;}
      while(zTzDosyaRg == zTzDosyaRg){LHdOXtRGde = true;}
      if(ZzuBXiwmuw == true){ZzuBXiwmuw = false;}
      if(gyOkljIJEj == true){gyOkljIJEj = false;}
      if(wOeeVRLABE == true){wOeeVRLABE = false;}
      if(OSYrHMYuiS == true){OSYrHMYuiS = false;}
      if(fwhYwBENRE == true){fwhYwBENRE = false;}
      if(jnWgkKUPfF == true){jnWgkKUPfF = false;}
      if(WwlqgNZWcY == true){WwlqgNZWcY = false;}
      if(LNZlEAgFiU == true){LNZlEAgFiU = false;}
      if(NyhwTawqHx == true){NyhwTawqHx = false;}
      if(qIPduiJqrh == true){qIPduiJqrh = false;}
      if(bhkrrWNjcj == true){bhkrrWNjcj = false;}
      if(dPgKrYNpWG == true){dPgKrYNpWG = false;}
      if(RhOfgDywpV == true){RhOfgDywpV = false;}
      if(qpNAreMoEx == true){qpNAreMoEx = false;}
      if(MMNWSbbElW == true){MMNWSbbElW = false;}
      if(UTTkQsSFLo == true){UTTkQsSFLo = false;}
      if(YRtHsnBgRM == true){YRtHsnBgRM = false;}
      if(LXlZtFUrMF == true){LXlZtFUrMF = false;}
      if(asJSAWVbiz == true){asJSAWVbiz = false;}
      if(LHdOXtRGde == true){LHdOXtRGde = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NGSWTCRMHU
{ 
  void MtwzjkTOeQ()
  { 
      bool zAjLMYrUNS = false;
      bool cxhqeayjBG = false;
      bool gsezMielLb = false;
      bool qWXPQpnhiN = false;
      bool jhKdtPsPbL = false;
      bool fpxFyFVjyc = false;
      bool sGIkaGYRZn = false;
      bool lPAmVyiAFw = false;
      bool oPJAwBYSRK = false;
      bool UEiGTdYYDm = false;
      bool pbomYQoJiz = false;
      bool WUdBmdxIoZ = false;
      bool gGAPThftSr = false;
      bool ZKxRGNZVae = false;
      bool LOiSnzZgSp = false;
      bool fQicGjbYTq = false;
      bool juqqZLXMuZ = false;
      bool DbhJNRWhbU = false;
      bool HzacLIaTLV = false;
      bool lBBiuQXrrb = false;
      string NFgpQKEBdR;
      string kJncluyPwX;
      string UhGeUqFBMa;
      string xHYPzboTeq;
      string NxHQlrjPzt;
      string IFDxVfzCUZ;
      string FCoCTckYcJ;
      string FdjEJWaorc;
      string GpCRRrEbsT;
      string bmyFlKIEwJ;
      string HLlooeVyny;
      string FKKrTOzWkb;
      string XIeJhSedSy;
      string OVrhVtENcl;
      string VGnuORbGkW;
      string LlSTkygcGV;
      string zqfoiiRbWZ;
      string KlsyGRUjtP;
      string pYsumcCXxW;
      string MaGHmdZyLe;
      if(NFgpQKEBdR == HLlooeVyny){zAjLMYrUNS = true;}
      else if(HLlooeVyny == NFgpQKEBdR){pbomYQoJiz = true;}
      if(kJncluyPwX == FKKrTOzWkb){cxhqeayjBG = true;}
      else if(FKKrTOzWkb == kJncluyPwX){WUdBmdxIoZ = true;}
      if(UhGeUqFBMa == XIeJhSedSy){gsezMielLb = true;}
      else if(XIeJhSedSy == UhGeUqFBMa){gGAPThftSr = true;}
      if(xHYPzboTeq == OVrhVtENcl){qWXPQpnhiN = true;}
      else if(OVrhVtENcl == xHYPzboTeq){ZKxRGNZVae = true;}
      if(NxHQlrjPzt == VGnuORbGkW){jhKdtPsPbL = true;}
      else if(VGnuORbGkW == NxHQlrjPzt){LOiSnzZgSp = true;}
      if(IFDxVfzCUZ == LlSTkygcGV){fpxFyFVjyc = true;}
      else if(LlSTkygcGV == IFDxVfzCUZ){fQicGjbYTq = true;}
      if(FCoCTckYcJ == zqfoiiRbWZ){sGIkaGYRZn = true;}
      else if(zqfoiiRbWZ == FCoCTckYcJ){juqqZLXMuZ = true;}
      if(FdjEJWaorc == KlsyGRUjtP){lPAmVyiAFw = true;}
      if(GpCRRrEbsT == pYsumcCXxW){oPJAwBYSRK = true;}
      if(bmyFlKIEwJ == MaGHmdZyLe){UEiGTdYYDm = true;}
      while(KlsyGRUjtP == FdjEJWaorc){DbhJNRWhbU = true;}
      while(pYsumcCXxW == pYsumcCXxW){HzacLIaTLV = true;}
      while(MaGHmdZyLe == MaGHmdZyLe){lBBiuQXrrb = true;}
      if(zAjLMYrUNS == true){zAjLMYrUNS = false;}
      if(cxhqeayjBG == true){cxhqeayjBG = false;}
      if(gsezMielLb == true){gsezMielLb = false;}
      if(qWXPQpnhiN == true){qWXPQpnhiN = false;}
      if(jhKdtPsPbL == true){jhKdtPsPbL = false;}
      if(fpxFyFVjyc == true){fpxFyFVjyc = false;}
      if(sGIkaGYRZn == true){sGIkaGYRZn = false;}
      if(lPAmVyiAFw == true){lPAmVyiAFw = false;}
      if(oPJAwBYSRK == true){oPJAwBYSRK = false;}
      if(UEiGTdYYDm == true){UEiGTdYYDm = false;}
      if(pbomYQoJiz == true){pbomYQoJiz = false;}
      if(WUdBmdxIoZ == true){WUdBmdxIoZ = false;}
      if(gGAPThftSr == true){gGAPThftSr = false;}
      if(ZKxRGNZVae == true){ZKxRGNZVae = false;}
      if(LOiSnzZgSp == true){LOiSnzZgSp = false;}
      if(fQicGjbYTq == true){fQicGjbYTq = false;}
      if(juqqZLXMuZ == true){juqqZLXMuZ = false;}
      if(DbhJNRWhbU == true){DbhJNRWhbU = false;}
      if(HzacLIaTLV == true){HzacLIaTLV = false;}
      if(lBBiuQXrrb == true){lBBiuQXrrb = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FIDGIHDEEK
{ 
  void RBXIXnhbSm()
  { 
      bool ZQzFVAqsTy = false;
      bool ljxxmGXZwp = false;
      bool tPerBIxfuR = false;
      bool xTArxXCPkt = false;
      bool fXxYbWEDWp = false;
      bool yjQpVKlHDM = false;
      bool CLyBBRiFAO = false;
      bool DHnCJVRGPe = false;
      bool DtIFTMDJVm = false;
      bool luMYqTZuWK = false;
      bool nzDHlpwOgz = false;
      bool rfqYIIDsho = false;
      bool OZjMcIuOKo = false;
      bool wWiFqEqASc = false;
      bool QncVuGDnPi = false;
      bool QfNhuNPpec = false;
      bool ugDqDVwlhD = false;
      bool RCZEYZwNRW = false;
      bool FHzbXPIBJc = false;
      bool oAUphjkkSY = false;
      string jlOwEYEXLE;
      string kMqIqZZeIZ;
      string flwzTDIMVq;
      string FYlAkBPWxS;
      string MOCLQfkLpw;
      string ULLxlxpObA;
      string QrCTzLZqWp;
      string KDgpNRjkhn;
      string SwWYWtNnOn;
      string LAtwBTLhME;
      string FsnzlRdVjT;
      string MEgnhTVdxE;
      string ylbidJIjns;
      string FCYdhYIWmF;
      string PqEtaGTFOg;
      string yhkHFTRFVc;
      string IsgqNKfCGJ;
      string LypqnHhgHX;
      string oCVHtXQQXO;
      string HCBKwWKNgP;
      if(jlOwEYEXLE == FsnzlRdVjT){ZQzFVAqsTy = true;}
      else if(FsnzlRdVjT == jlOwEYEXLE){nzDHlpwOgz = true;}
      if(kMqIqZZeIZ == MEgnhTVdxE){ljxxmGXZwp = true;}
      else if(MEgnhTVdxE == kMqIqZZeIZ){rfqYIIDsho = true;}
      if(flwzTDIMVq == ylbidJIjns){tPerBIxfuR = true;}
      else if(ylbidJIjns == flwzTDIMVq){OZjMcIuOKo = true;}
      if(FYlAkBPWxS == FCYdhYIWmF){xTArxXCPkt = true;}
      else if(FCYdhYIWmF == FYlAkBPWxS){wWiFqEqASc = true;}
      if(MOCLQfkLpw == PqEtaGTFOg){fXxYbWEDWp = true;}
      else if(PqEtaGTFOg == MOCLQfkLpw){QncVuGDnPi = true;}
      if(ULLxlxpObA == yhkHFTRFVc){yjQpVKlHDM = true;}
      else if(yhkHFTRFVc == ULLxlxpObA){QfNhuNPpec = true;}
      if(QrCTzLZqWp == IsgqNKfCGJ){CLyBBRiFAO = true;}
      else if(IsgqNKfCGJ == QrCTzLZqWp){ugDqDVwlhD = true;}
      if(KDgpNRjkhn == LypqnHhgHX){DHnCJVRGPe = true;}
      if(SwWYWtNnOn == oCVHtXQQXO){DtIFTMDJVm = true;}
      if(LAtwBTLhME == HCBKwWKNgP){luMYqTZuWK = true;}
      while(LypqnHhgHX == KDgpNRjkhn){RCZEYZwNRW = true;}
      while(oCVHtXQQXO == oCVHtXQQXO){FHzbXPIBJc = true;}
      while(HCBKwWKNgP == HCBKwWKNgP){oAUphjkkSY = true;}
      if(ZQzFVAqsTy == true){ZQzFVAqsTy = false;}
      if(ljxxmGXZwp == true){ljxxmGXZwp = false;}
      if(tPerBIxfuR == true){tPerBIxfuR = false;}
      if(xTArxXCPkt == true){xTArxXCPkt = false;}
      if(fXxYbWEDWp == true){fXxYbWEDWp = false;}
      if(yjQpVKlHDM == true){yjQpVKlHDM = false;}
      if(CLyBBRiFAO == true){CLyBBRiFAO = false;}
      if(DHnCJVRGPe == true){DHnCJVRGPe = false;}
      if(DtIFTMDJVm == true){DtIFTMDJVm = false;}
      if(luMYqTZuWK == true){luMYqTZuWK = false;}
      if(nzDHlpwOgz == true){nzDHlpwOgz = false;}
      if(rfqYIIDsho == true){rfqYIIDsho = false;}
      if(OZjMcIuOKo == true){OZjMcIuOKo = false;}
      if(wWiFqEqASc == true){wWiFqEqASc = false;}
      if(QncVuGDnPi == true){QncVuGDnPi = false;}
      if(QfNhuNPpec == true){QfNhuNPpec = false;}
      if(ugDqDVwlhD == true){ugDqDVwlhD = false;}
      if(RCZEYZwNRW == true){RCZEYZwNRW = false;}
      if(FHzbXPIBJc == true){FHzbXPIBJc = false;}
      if(oAUphjkkSY == true){oAUphjkkSY = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SIGEAWKHYH
{ 
  void jjuaNEqUyd()
  { 
      bool WLwFjCyAHL = false;
      bool LCGbQTiStw = false;
      bool AVlubahEJO = false;
      bool NGphgkcYoA = false;
      bool KazImdOVhA = false;
      bool bhSegBZAdJ = false;
      bool UsqsyELYuf = false;
      bool qJdtkZVcIx = false;
      bool MLWIlenLAe = false;
      bool KbuffQjGJh = false;
      bool SoaCIREYsH = false;
      bool fwrfwIWhGT = false;
      bool QAYPUONyNU = false;
      bool pOSZeWkUAU = false;
      bool ZPSrzOjOiX = false;
      bool tBApRRQKgs = false;
      bool DGKYRQcRBZ = false;
      bool WBhksBhfPG = false;
      bool ezOpZnOcIM = false;
      bool GZmGQTEsai = false;
      string OJwAPskSdl;
      string ahtSSlVWjp;
      string LIaPfkLOIP;
      string HYGxzzBlDF;
      string NeiIkmQSCP;
      string raeMyGifTe;
      string hVKNdCaxTS;
      string kiSecfZRds;
      string UrMrWgstyr;
      string PUSYXqUUdi;
      string MSzieJHziR;
      string gQBaKpJRqx;
      string IfyaDPQtHk;
      string LEnufBsdNN;
      string YtLgKzSqOg;
      string hCdfagTXfO;
      string sNrxFbcGuI;
      string wfEcYOURQD;
      string xYeZDJaHUz;
      string gVxzTKBNah;
      if(OJwAPskSdl == MSzieJHziR){WLwFjCyAHL = true;}
      else if(MSzieJHziR == OJwAPskSdl){SoaCIREYsH = true;}
      if(ahtSSlVWjp == gQBaKpJRqx){LCGbQTiStw = true;}
      else if(gQBaKpJRqx == ahtSSlVWjp){fwrfwIWhGT = true;}
      if(LIaPfkLOIP == IfyaDPQtHk){AVlubahEJO = true;}
      else if(IfyaDPQtHk == LIaPfkLOIP){QAYPUONyNU = true;}
      if(HYGxzzBlDF == LEnufBsdNN){NGphgkcYoA = true;}
      else if(LEnufBsdNN == HYGxzzBlDF){pOSZeWkUAU = true;}
      if(NeiIkmQSCP == YtLgKzSqOg){KazImdOVhA = true;}
      else if(YtLgKzSqOg == NeiIkmQSCP){ZPSrzOjOiX = true;}
      if(raeMyGifTe == hCdfagTXfO){bhSegBZAdJ = true;}
      else if(hCdfagTXfO == raeMyGifTe){tBApRRQKgs = true;}
      if(hVKNdCaxTS == sNrxFbcGuI){UsqsyELYuf = true;}
      else if(sNrxFbcGuI == hVKNdCaxTS){DGKYRQcRBZ = true;}
      if(kiSecfZRds == wfEcYOURQD){qJdtkZVcIx = true;}
      if(UrMrWgstyr == xYeZDJaHUz){MLWIlenLAe = true;}
      if(PUSYXqUUdi == gVxzTKBNah){KbuffQjGJh = true;}
      while(wfEcYOURQD == kiSecfZRds){WBhksBhfPG = true;}
      while(xYeZDJaHUz == xYeZDJaHUz){ezOpZnOcIM = true;}
      while(gVxzTKBNah == gVxzTKBNah){GZmGQTEsai = true;}
      if(WLwFjCyAHL == true){WLwFjCyAHL = false;}
      if(LCGbQTiStw == true){LCGbQTiStw = false;}
      if(AVlubahEJO == true){AVlubahEJO = false;}
      if(NGphgkcYoA == true){NGphgkcYoA = false;}
      if(KazImdOVhA == true){KazImdOVhA = false;}
      if(bhSegBZAdJ == true){bhSegBZAdJ = false;}
      if(UsqsyELYuf == true){UsqsyELYuf = false;}
      if(qJdtkZVcIx == true){qJdtkZVcIx = false;}
      if(MLWIlenLAe == true){MLWIlenLAe = false;}
      if(KbuffQjGJh == true){KbuffQjGJh = false;}
      if(SoaCIREYsH == true){SoaCIREYsH = false;}
      if(fwrfwIWhGT == true){fwrfwIWhGT = false;}
      if(QAYPUONyNU == true){QAYPUONyNU = false;}
      if(pOSZeWkUAU == true){pOSZeWkUAU = false;}
      if(ZPSrzOjOiX == true){ZPSrzOjOiX = false;}
      if(tBApRRQKgs == true){tBApRRQKgs = false;}
      if(DGKYRQcRBZ == true){DGKYRQcRBZ = false;}
      if(WBhksBhfPG == true){WBhksBhfPG = false;}
      if(ezOpZnOcIM == true){ezOpZnOcIM = false;}
      if(GZmGQTEsai == true){GZmGQTEsai = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MPQWQGBWJS
{ 
  void obZhLTzoqy()
  { 
      bool AZicqwYPcq = false;
      bool oeJsSEVbYV = false;
      bool sRcEgitroL = false;
      bool aNlLQRCkNJ = false;
      bool OVKMEnliKc = false;
      bool JjzIrhxewH = false;
      bool CfZQPasycB = false;
      bool BkOOGXGPzr = false;
      bool xLhLBqOcSi = false;
      bool UWoFHiutxd = false;
      bool unGIbYVkBU = false;
      bool agfDKpxNPY = false;
      bool LQetgduIGX = false;
      bool bMSUxIFZjc = false;
      bool UdbyHmqltR = false;
      bool kgdXSdILay = false;
      bool cNddrNNLFC = false;
      bool kXdlEhngls = false;
      bool mJNolthuCS = false;
      bool zsykzHzxFt = false;
      string fKWLmtTzrS;
      string oSBQHWcTEH;
      string FUNDEFhqnO;
      string PASOgQtHdT;
      string EQMeDRnHXn;
      string fTuIgLwSGh;
      string LbLdDnAmjp;
      string bUojCnHizm;
      string QaSAiuFJhU;
      string TbBuHcFiaO;
      string RBWgLKeKFs;
      string DNSTQjIKuH;
      string ipFOWATscZ;
      string iYHkVPElnX;
      string QtDoQGIheW;
      string TtmTfauHJK;
      string pkheJeFhyB;
      string fWVWqnlQxQ;
      string rlOFGnknEt;
      string TwbchfMXel;
      if(fKWLmtTzrS == RBWgLKeKFs){AZicqwYPcq = true;}
      else if(RBWgLKeKFs == fKWLmtTzrS){unGIbYVkBU = true;}
      if(oSBQHWcTEH == DNSTQjIKuH){oeJsSEVbYV = true;}
      else if(DNSTQjIKuH == oSBQHWcTEH){agfDKpxNPY = true;}
      if(FUNDEFhqnO == ipFOWATscZ){sRcEgitroL = true;}
      else if(ipFOWATscZ == FUNDEFhqnO){LQetgduIGX = true;}
      if(PASOgQtHdT == iYHkVPElnX){aNlLQRCkNJ = true;}
      else if(iYHkVPElnX == PASOgQtHdT){bMSUxIFZjc = true;}
      if(EQMeDRnHXn == QtDoQGIheW){OVKMEnliKc = true;}
      else if(QtDoQGIheW == EQMeDRnHXn){UdbyHmqltR = true;}
      if(fTuIgLwSGh == TtmTfauHJK){JjzIrhxewH = true;}
      else if(TtmTfauHJK == fTuIgLwSGh){kgdXSdILay = true;}
      if(LbLdDnAmjp == pkheJeFhyB){CfZQPasycB = true;}
      else if(pkheJeFhyB == LbLdDnAmjp){cNddrNNLFC = true;}
      if(bUojCnHizm == fWVWqnlQxQ){BkOOGXGPzr = true;}
      if(QaSAiuFJhU == rlOFGnknEt){xLhLBqOcSi = true;}
      if(TbBuHcFiaO == TwbchfMXel){UWoFHiutxd = true;}
      while(fWVWqnlQxQ == bUojCnHizm){kXdlEhngls = true;}
      while(rlOFGnknEt == rlOFGnknEt){mJNolthuCS = true;}
      while(TwbchfMXel == TwbchfMXel){zsykzHzxFt = true;}
      if(AZicqwYPcq == true){AZicqwYPcq = false;}
      if(oeJsSEVbYV == true){oeJsSEVbYV = false;}
      if(sRcEgitroL == true){sRcEgitroL = false;}
      if(aNlLQRCkNJ == true){aNlLQRCkNJ = false;}
      if(OVKMEnliKc == true){OVKMEnliKc = false;}
      if(JjzIrhxewH == true){JjzIrhxewH = false;}
      if(CfZQPasycB == true){CfZQPasycB = false;}
      if(BkOOGXGPzr == true){BkOOGXGPzr = false;}
      if(xLhLBqOcSi == true){xLhLBqOcSi = false;}
      if(UWoFHiutxd == true){UWoFHiutxd = false;}
      if(unGIbYVkBU == true){unGIbYVkBU = false;}
      if(agfDKpxNPY == true){agfDKpxNPY = false;}
      if(LQetgduIGX == true){LQetgduIGX = false;}
      if(bMSUxIFZjc == true){bMSUxIFZjc = false;}
      if(UdbyHmqltR == true){UdbyHmqltR = false;}
      if(kgdXSdILay == true){kgdXSdILay = false;}
      if(cNddrNNLFC == true){cNddrNNLFC = false;}
      if(kXdlEhngls == true){kXdlEhngls = false;}
      if(mJNolthuCS == true){mJNolthuCS = false;}
      if(zsykzHzxFt == true){zsykzHzxFt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class INBKAXRKFY
{ 
  void EJaaFgXSzO()
  { 
      bool jNbbgXqBNb = false;
      bool tLGpOwDPjZ = false;
      bool WbkJaAEEfM = false;
      bool OZoUklFGct = false;
      bool ZfQaslMCDS = false;
      bool ToZVMHcRBf = false;
      bool SlUANUWkFc = false;
      bool qwmpApLJPn = false;
      bool mCBNsZzSIc = false;
      bool ImFwbmKPNX = false;
      bool PbwZbWCjVV = false;
      bool wTBMsQjHHm = false;
      bool UDbbTDqpOM = false;
      bool euurYLFalV = false;
      bool XPxmgokrCK = false;
      bool DDHZCLzUGX = false;
      bool fLVIpAXJnh = false;
      bool KIUlPFGote = false;
      bool SynDJbmUoM = false;
      bool MkUOoMtnwg = false;
      string VzuesYLIbd;
      string YzDmXXuBkD;
      string OUqIUMwkHJ;
      string DJzYYjdzzb;
      string uSDTLayQKK;
      string gXsxQBIlJg;
      string caARZPzeOm;
      string twCIHwQWxt;
      string NeZxcePCHW;
      string bqoSyURksw;
      string aTBWQjVjoU;
      string jkoPSxADnt;
      string zwXqLCGiKm;
      string dedAStdDfr;
      string JeMlmqCYAA;
      string UNxywewKKO;
      string dlZCpEHnxX;
      string TxWukTjkDU;
      string zgiJRQheDn;
      string xTrROXFKwi;
      if(VzuesYLIbd == aTBWQjVjoU){jNbbgXqBNb = true;}
      else if(aTBWQjVjoU == VzuesYLIbd){PbwZbWCjVV = true;}
      if(YzDmXXuBkD == jkoPSxADnt){tLGpOwDPjZ = true;}
      else if(jkoPSxADnt == YzDmXXuBkD){wTBMsQjHHm = true;}
      if(OUqIUMwkHJ == zwXqLCGiKm){WbkJaAEEfM = true;}
      else if(zwXqLCGiKm == OUqIUMwkHJ){UDbbTDqpOM = true;}
      if(DJzYYjdzzb == dedAStdDfr){OZoUklFGct = true;}
      else if(dedAStdDfr == DJzYYjdzzb){euurYLFalV = true;}
      if(uSDTLayQKK == JeMlmqCYAA){ZfQaslMCDS = true;}
      else if(JeMlmqCYAA == uSDTLayQKK){XPxmgokrCK = true;}
      if(gXsxQBIlJg == UNxywewKKO){ToZVMHcRBf = true;}
      else if(UNxywewKKO == gXsxQBIlJg){DDHZCLzUGX = true;}
      if(caARZPzeOm == dlZCpEHnxX){SlUANUWkFc = true;}
      else if(dlZCpEHnxX == caARZPzeOm){fLVIpAXJnh = true;}
      if(twCIHwQWxt == TxWukTjkDU){qwmpApLJPn = true;}
      if(NeZxcePCHW == zgiJRQheDn){mCBNsZzSIc = true;}
      if(bqoSyURksw == xTrROXFKwi){ImFwbmKPNX = true;}
      while(TxWukTjkDU == twCIHwQWxt){KIUlPFGote = true;}
      while(zgiJRQheDn == zgiJRQheDn){SynDJbmUoM = true;}
      while(xTrROXFKwi == xTrROXFKwi){MkUOoMtnwg = true;}
      if(jNbbgXqBNb == true){jNbbgXqBNb = false;}
      if(tLGpOwDPjZ == true){tLGpOwDPjZ = false;}
      if(WbkJaAEEfM == true){WbkJaAEEfM = false;}
      if(OZoUklFGct == true){OZoUklFGct = false;}
      if(ZfQaslMCDS == true){ZfQaslMCDS = false;}
      if(ToZVMHcRBf == true){ToZVMHcRBf = false;}
      if(SlUANUWkFc == true){SlUANUWkFc = false;}
      if(qwmpApLJPn == true){qwmpApLJPn = false;}
      if(mCBNsZzSIc == true){mCBNsZzSIc = false;}
      if(ImFwbmKPNX == true){ImFwbmKPNX = false;}
      if(PbwZbWCjVV == true){PbwZbWCjVV = false;}
      if(wTBMsQjHHm == true){wTBMsQjHHm = false;}
      if(UDbbTDqpOM == true){UDbbTDqpOM = false;}
      if(euurYLFalV == true){euurYLFalV = false;}
      if(XPxmgokrCK == true){XPxmgokrCK = false;}
      if(DDHZCLzUGX == true){DDHZCLzUGX = false;}
      if(fLVIpAXJnh == true){fLVIpAXJnh = false;}
      if(KIUlPFGote == true){KIUlPFGote = false;}
      if(SynDJbmUoM == true){SynDJbmUoM = false;}
      if(MkUOoMtnwg == true){MkUOoMtnwg = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class AUAODBBXSV
{ 
  void qEKntKMeAn()
  { 
      bool PcebaoDQKe = false;
      bool fwVAmUUKgE = false;
      bool yMkVKZwzsK = false;
      bool rttOBPyIne = false;
      bool XPJthKJHuL = false;
      bool nyehDCrJRA = false;
      bool sDXAmlBGyX = false;
      bool VMVCfhDpmA = false;
      bool PnEgVsPMLz = false;
      bool WoqmlUqJGw = false;
      bool IDJWobbbBU = false;
      bool zCVQkEzpds = false;
      bool VntzBgViPL = false;
      bool qsiqqMegrs = false;
      bool RmKqATSZCh = false;
      bool VBeMZUIJVH = false;
      bool GsMpdFSdZY = false;
      bool ZRxjbicDzg = false;
      bool NufhpoNWRW = false;
      bool DULjmXaUMt = false;
      string KDKNSkIAln;
      string HcOfVcsjZl;
      string FpHXRDNcaJ;
      string HVdCBbqyKa;
      string CIXtBHoMYy;
      string mJSzpTulwO;
      string XfYbglzMdt;
      string kuZyjwSNVK;
      string NADbWyCdMh;
      string fEBuDViuNP;
      string IcieMrjUPs;
      string BhQlBWcxEh;
      string iUzwNgsduN;
      string NWAGBjRYOe;
      string OhOSfxnPsf;
      string slpefmclQf;
      string yFYCfdhKjR;
      string FEhZTAGgQQ;
      string sdjOUbfUZO;
      string GfHhfJIMlB;
      if(KDKNSkIAln == IcieMrjUPs){PcebaoDQKe = true;}
      else if(IcieMrjUPs == KDKNSkIAln){IDJWobbbBU = true;}
      if(HcOfVcsjZl == BhQlBWcxEh){fwVAmUUKgE = true;}
      else if(BhQlBWcxEh == HcOfVcsjZl){zCVQkEzpds = true;}
      if(FpHXRDNcaJ == iUzwNgsduN){yMkVKZwzsK = true;}
      else if(iUzwNgsduN == FpHXRDNcaJ){VntzBgViPL = true;}
      if(HVdCBbqyKa == NWAGBjRYOe){rttOBPyIne = true;}
      else if(NWAGBjRYOe == HVdCBbqyKa){qsiqqMegrs = true;}
      if(CIXtBHoMYy == OhOSfxnPsf){XPJthKJHuL = true;}
      else if(OhOSfxnPsf == CIXtBHoMYy){RmKqATSZCh = true;}
      if(mJSzpTulwO == slpefmclQf){nyehDCrJRA = true;}
      else if(slpefmclQf == mJSzpTulwO){VBeMZUIJVH = true;}
      if(XfYbglzMdt == yFYCfdhKjR){sDXAmlBGyX = true;}
      else if(yFYCfdhKjR == XfYbglzMdt){GsMpdFSdZY = true;}
      if(kuZyjwSNVK == FEhZTAGgQQ){VMVCfhDpmA = true;}
      if(NADbWyCdMh == sdjOUbfUZO){PnEgVsPMLz = true;}
      if(fEBuDViuNP == GfHhfJIMlB){WoqmlUqJGw = true;}
      while(FEhZTAGgQQ == kuZyjwSNVK){ZRxjbicDzg = true;}
      while(sdjOUbfUZO == sdjOUbfUZO){NufhpoNWRW = true;}
      while(GfHhfJIMlB == GfHhfJIMlB){DULjmXaUMt = true;}
      if(PcebaoDQKe == true){PcebaoDQKe = false;}
      if(fwVAmUUKgE == true){fwVAmUUKgE = false;}
      if(yMkVKZwzsK == true){yMkVKZwzsK = false;}
      if(rttOBPyIne == true){rttOBPyIne = false;}
      if(XPJthKJHuL == true){XPJthKJHuL = false;}
      if(nyehDCrJRA == true){nyehDCrJRA = false;}
      if(sDXAmlBGyX == true){sDXAmlBGyX = false;}
      if(VMVCfhDpmA == true){VMVCfhDpmA = false;}
      if(PnEgVsPMLz == true){PnEgVsPMLz = false;}
      if(WoqmlUqJGw == true){WoqmlUqJGw = false;}
      if(IDJWobbbBU == true){IDJWobbbBU = false;}
      if(zCVQkEzpds == true){zCVQkEzpds = false;}
      if(VntzBgViPL == true){VntzBgViPL = false;}
      if(qsiqqMegrs == true){qsiqqMegrs = false;}
      if(RmKqATSZCh == true){RmKqATSZCh = false;}
      if(VBeMZUIJVH == true){VBeMZUIJVH = false;}
      if(GsMpdFSdZY == true){GsMpdFSdZY = false;}
      if(ZRxjbicDzg == true){ZRxjbicDzg = false;}
      if(NufhpoNWRW == true){NufhpoNWRW = false;}
      if(DULjmXaUMt == true){DULjmXaUMt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UPSTFOHAEC
{ 
  void TrdrHPkoTY()
  { 
      bool aftngXJwVf = false;
      bool OzylWjDTJV = false;
      bool bmBarQGHek = false;
      bool qPTwcTNiId = false;
      bool ipRjaDFytI = false;
      bool VyAXHmdSwQ = false;
      bool RBRknyeCID = false;
      bool WULelkVRzn = false;
      bool KHNVOrPwnp = false;
      bool otlqMgYguc = false;
      bool YCpplBmVkV = false;
      bool RLSHGJmwWw = false;
      bool ihORNchTRF = false;
      bool MoWWPHQXjM = false;
      bool FsLRYdadkD = false;
      bool sbJztpnXcI = false;
      bool TTDeJbnwry = false;
      bool rPMJpoVwsj = false;
      bool BDhOLjeXcY = false;
      bool TsuaslogIw = false;
      string mzqGzESNqA;
      string CUWlKztyCZ;
      string iarhtsGatj;
      string KiaGiFeTzD;
      string ZMmgyexNBn;
      string leCcKLHILG;
      string xrSHpTtQDE;
      string TazyzVnLjP;
      string wsYGxffywX;
      string yWHsGhteqd;
      string kgKxTLfDco;
      string XwYwKDSLKK;
      string muTeGxSFrR;
      string aIUGePibml;
      string zcVGzCyLiC;
      string OhUfpoMepV;
      string rZKLNPaRQt;
      string UqYXKKZMfE;
      string zHWpCrmnPp;
      string CEkQsZSJsj;
      if(mzqGzESNqA == kgKxTLfDco){aftngXJwVf = true;}
      else if(kgKxTLfDco == mzqGzESNqA){YCpplBmVkV = true;}
      if(CUWlKztyCZ == XwYwKDSLKK){OzylWjDTJV = true;}
      else if(XwYwKDSLKK == CUWlKztyCZ){RLSHGJmwWw = true;}
      if(iarhtsGatj == muTeGxSFrR){bmBarQGHek = true;}
      else if(muTeGxSFrR == iarhtsGatj){ihORNchTRF = true;}
      if(KiaGiFeTzD == aIUGePibml){qPTwcTNiId = true;}
      else if(aIUGePibml == KiaGiFeTzD){MoWWPHQXjM = true;}
      if(ZMmgyexNBn == zcVGzCyLiC){ipRjaDFytI = true;}
      else if(zcVGzCyLiC == ZMmgyexNBn){FsLRYdadkD = true;}
      if(leCcKLHILG == OhUfpoMepV){VyAXHmdSwQ = true;}
      else if(OhUfpoMepV == leCcKLHILG){sbJztpnXcI = true;}
      if(xrSHpTtQDE == rZKLNPaRQt){RBRknyeCID = true;}
      else if(rZKLNPaRQt == xrSHpTtQDE){TTDeJbnwry = true;}
      if(TazyzVnLjP == UqYXKKZMfE){WULelkVRzn = true;}
      if(wsYGxffywX == zHWpCrmnPp){KHNVOrPwnp = true;}
      if(yWHsGhteqd == CEkQsZSJsj){otlqMgYguc = true;}
      while(UqYXKKZMfE == TazyzVnLjP){rPMJpoVwsj = true;}
      while(zHWpCrmnPp == zHWpCrmnPp){BDhOLjeXcY = true;}
      while(CEkQsZSJsj == CEkQsZSJsj){TsuaslogIw = true;}
      if(aftngXJwVf == true){aftngXJwVf = false;}
      if(OzylWjDTJV == true){OzylWjDTJV = false;}
      if(bmBarQGHek == true){bmBarQGHek = false;}
      if(qPTwcTNiId == true){qPTwcTNiId = false;}
      if(ipRjaDFytI == true){ipRjaDFytI = false;}
      if(VyAXHmdSwQ == true){VyAXHmdSwQ = false;}
      if(RBRknyeCID == true){RBRknyeCID = false;}
      if(WULelkVRzn == true){WULelkVRzn = false;}
      if(KHNVOrPwnp == true){KHNVOrPwnp = false;}
      if(otlqMgYguc == true){otlqMgYguc = false;}
      if(YCpplBmVkV == true){YCpplBmVkV = false;}
      if(RLSHGJmwWw == true){RLSHGJmwWw = false;}
      if(ihORNchTRF == true){ihORNchTRF = false;}
      if(MoWWPHQXjM == true){MoWWPHQXjM = false;}
      if(FsLRYdadkD == true){FsLRYdadkD = false;}
      if(sbJztpnXcI == true){sbJztpnXcI = false;}
      if(TTDeJbnwry == true){TTDeJbnwry = false;}
      if(rPMJpoVwsj == true){rPMJpoVwsj = false;}
      if(BDhOLjeXcY == true){BDhOLjeXcY = false;}
      if(TsuaslogIw == true){TsuaslogIw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZEFIZQXRFR
{ 
  void bimNAJZHKq()
  { 
      bool KNAbuDWlOG = false;
      bool VcANJjzoFf = false;
      bool ENzDriKXZk = false;
      bool epAkdyywUA = false;
      bool sEBwKJjizJ = false;
      bool kQQJDxNwjL = false;
      bool syLdaecOiL = false;
      bool lDTElzLDsi = false;
      bool zaYQzNZpgf = false;
      bool kBLJHusXUe = false;
      bool gUAnXhxLXE = false;
      bool lJDBLRZudM = false;
      bool FlgDakqKxq = false;
      bool FmhNBeakrR = false;
      bool QNSctayKxo = false;
      bool VytIZtnwNm = false;
      bool pmOfyybpzg = false;
      bool qoCLwqSEIz = false;
      bool lgMHVHsZmH = false;
      bool qdWFbdymdu = false;
      string DcAOUWxKaj;
      string TNooWmtHWj;
      string uFcSWfFLTf;
      string qZNIuVwbhd;
      string DIAJiddkgp;
      string jdcGEyzhrn;
      string RkqtrtAgtA;
      string qSYrglESjs;
      string CUeduNwGEx;
      string VOjsMRRGQo;
      string cwpfyREkZR;
      string sgyzkxGMze;
      string xtcxdcifeP;
      string UyCuPGdyed;
      string ILglDoQjHO;
      string KEHkriTsBV;
      string iXFJwudDje;
      string THhUVkNpsn;
      string LyYQEQqQDf;
      string mjxJtkGWpI;
      if(DcAOUWxKaj == cwpfyREkZR){KNAbuDWlOG = true;}
      else if(cwpfyREkZR == DcAOUWxKaj){gUAnXhxLXE = true;}
      if(TNooWmtHWj == sgyzkxGMze){VcANJjzoFf = true;}
      else if(sgyzkxGMze == TNooWmtHWj){lJDBLRZudM = true;}
      if(uFcSWfFLTf == xtcxdcifeP){ENzDriKXZk = true;}
      else if(xtcxdcifeP == uFcSWfFLTf){FlgDakqKxq = true;}
      if(qZNIuVwbhd == UyCuPGdyed){epAkdyywUA = true;}
      else if(UyCuPGdyed == qZNIuVwbhd){FmhNBeakrR = true;}
      if(DIAJiddkgp == ILglDoQjHO){sEBwKJjizJ = true;}
      else if(ILglDoQjHO == DIAJiddkgp){QNSctayKxo = true;}
      if(jdcGEyzhrn == KEHkriTsBV){kQQJDxNwjL = true;}
      else if(KEHkriTsBV == jdcGEyzhrn){VytIZtnwNm = true;}
      if(RkqtrtAgtA == iXFJwudDje){syLdaecOiL = true;}
      else if(iXFJwudDje == RkqtrtAgtA){pmOfyybpzg = true;}
      if(qSYrglESjs == THhUVkNpsn){lDTElzLDsi = true;}
      if(CUeduNwGEx == LyYQEQqQDf){zaYQzNZpgf = true;}
      if(VOjsMRRGQo == mjxJtkGWpI){kBLJHusXUe = true;}
      while(THhUVkNpsn == qSYrglESjs){qoCLwqSEIz = true;}
      while(LyYQEQqQDf == LyYQEQqQDf){lgMHVHsZmH = true;}
      while(mjxJtkGWpI == mjxJtkGWpI){qdWFbdymdu = true;}
      if(KNAbuDWlOG == true){KNAbuDWlOG = false;}
      if(VcANJjzoFf == true){VcANJjzoFf = false;}
      if(ENzDriKXZk == true){ENzDriKXZk = false;}
      if(epAkdyywUA == true){epAkdyywUA = false;}
      if(sEBwKJjizJ == true){sEBwKJjizJ = false;}
      if(kQQJDxNwjL == true){kQQJDxNwjL = false;}
      if(syLdaecOiL == true){syLdaecOiL = false;}
      if(lDTElzLDsi == true){lDTElzLDsi = false;}
      if(zaYQzNZpgf == true){zaYQzNZpgf = false;}
      if(kBLJHusXUe == true){kBLJHusXUe = false;}
      if(gUAnXhxLXE == true){gUAnXhxLXE = false;}
      if(lJDBLRZudM == true){lJDBLRZudM = false;}
      if(FlgDakqKxq == true){FlgDakqKxq = false;}
      if(FmhNBeakrR == true){FmhNBeakrR = false;}
      if(QNSctayKxo == true){QNSctayKxo = false;}
      if(VytIZtnwNm == true){VytIZtnwNm = false;}
      if(pmOfyybpzg == true){pmOfyybpzg = false;}
      if(qoCLwqSEIz == true){qoCLwqSEIz = false;}
      if(lgMHVHsZmH == true){lgMHVHsZmH = false;}
      if(qdWFbdymdu == true){qdWFbdymdu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WSQJKFVROT
{ 
  void AnViqtzyuy()
  { 
      bool nIDmmzOoDm = false;
      bool YFLGMdEXBs = false;
      bool yQhkKVwHie = false;
      bool AHYzyFriym = false;
      bool xnIEgHoaMq = false;
      bool QLUUsWaScQ = false;
      bool DGFXlQgLHG = false;
      bool kVYOuKHcEC = false;
      bool gUuqNolDGb = false;
      bool eYFgjqhkSP = false;
      bool acTFFulGYi = false;
      bool OxNajhLNJw = false;
      bool ecFebMPreE = false;
      bool bWKoaEBolS = false;
      bool DfeWMgKexB = false;
      bool fTaVkyWnds = false;
      bool etaUGqmYze = false;
      bool gVbqsyyiSt = false;
      bool BktcYIwnDc = false;
      bool gNLsnJEKQV = false;
      string XUhuoEFPSd;
      string ENYEfobxKy;
      string NNMhuDKfMK;
      string egKgbRVrae;
      string EdAxVutqJp;
      string zckwxKHAzl;
      string sYCIyXCrcN;
      string SjVftiNOLA;
      string ISVWKCwNiQ;
      string KVsokebqzx;
      string rCOwWPHECN;
      string xsnydaCfyu;
      string aoILDwpguO;
      string HVPNIGDZzt;
      string urKUTQhtKJ;
      string busDWCsDra;
      string ZZInGbcZpX;
      string WcgDAidmql;
      string NwVklMcMqF;
      string NAgYUUiWOr;
      if(XUhuoEFPSd == rCOwWPHECN){nIDmmzOoDm = true;}
      else if(rCOwWPHECN == XUhuoEFPSd){acTFFulGYi = true;}
      if(ENYEfobxKy == xsnydaCfyu){YFLGMdEXBs = true;}
      else if(xsnydaCfyu == ENYEfobxKy){OxNajhLNJw = true;}
      if(NNMhuDKfMK == aoILDwpguO){yQhkKVwHie = true;}
      else if(aoILDwpguO == NNMhuDKfMK){ecFebMPreE = true;}
      if(egKgbRVrae == HVPNIGDZzt){AHYzyFriym = true;}
      else if(HVPNIGDZzt == egKgbRVrae){bWKoaEBolS = true;}
      if(EdAxVutqJp == urKUTQhtKJ){xnIEgHoaMq = true;}
      else if(urKUTQhtKJ == EdAxVutqJp){DfeWMgKexB = true;}
      if(zckwxKHAzl == busDWCsDra){QLUUsWaScQ = true;}
      else if(busDWCsDra == zckwxKHAzl){fTaVkyWnds = true;}
      if(sYCIyXCrcN == ZZInGbcZpX){DGFXlQgLHG = true;}
      else if(ZZInGbcZpX == sYCIyXCrcN){etaUGqmYze = true;}
      if(SjVftiNOLA == WcgDAidmql){kVYOuKHcEC = true;}
      if(ISVWKCwNiQ == NwVklMcMqF){gUuqNolDGb = true;}
      if(KVsokebqzx == NAgYUUiWOr){eYFgjqhkSP = true;}
      while(WcgDAidmql == SjVftiNOLA){gVbqsyyiSt = true;}
      while(NwVklMcMqF == NwVklMcMqF){BktcYIwnDc = true;}
      while(NAgYUUiWOr == NAgYUUiWOr){gNLsnJEKQV = true;}
      if(nIDmmzOoDm == true){nIDmmzOoDm = false;}
      if(YFLGMdEXBs == true){YFLGMdEXBs = false;}
      if(yQhkKVwHie == true){yQhkKVwHie = false;}
      if(AHYzyFriym == true){AHYzyFriym = false;}
      if(xnIEgHoaMq == true){xnIEgHoaMq = false;}
      if(QLUUsWaScQ == true){QLUUsWaScQ = false;}
      if(DGFXlQgLHG == true){DGFXlQgLHG = false;}
      if(kVYOuKHcEC == true){kVYOuKHcEC = false;}
      if(gUuqNolDGb == true){gUuqNolDGb = false;}
      if(eYFgjqhkSP == true){eYFgjqhkSP = false;}
      if(acTFFulGYi == true){acTFFulGYi = false;}
      if(OxNajhLNJw == true){OxNajhLNJw = false;}
      if(ecFebMPreE == true){ecFebMPreE = false;}
      if(bWKoaEBolS == true){bWKoaEBolS = false;}
      if(DfeWMgKexB == true){DfeWMgKexB = false;}
      if(fTaVkyWnds == true){fTaVkyWnds = false;}
      if(etaUGqmYze == true){etaUGqmYze = false;}
      if(gVbqsyyiSt == true){gVbqsyyiSt = false;}
      if(BktcYIwnDc == true){BktcYIwnDc = false;}
      if(gNLsnJEKQV == true){gNLsnJEKQV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DOVCGRUWPD
{ 
  void EuEKXLAmGu()
  { 
      bool HKydcTPNSP = false;
      bool xeZwnERkQj = false;
      bool zQlyokTeff = false;
      bool TqEAsPhEsf = false;
      bool llFybwygoR = false;
      bool AhPskpYzor = false;
      bool AkgmwGaZCX = false;
      bool WMrkyMLnWr = false;
      bool mloOelaKPD = false;
      bool qhPAfAFRCV = false;
      bool VXdQtmoeuX = false;
      bool ejcxFNuRHJ = false;
      bool fPKzfnxrFx = false;
      bool HskBVOIYTr = false;
      bool wuOqXGFSuS = false;
      bool FmJrpFNejG = false;
      bool pxZyQkzbaq = false;
      bool FWxPNjBSaZ = false;
      bool hewlrUisLP = false;
      bool AcnNByVQal = false;
      string gTDhzqiIjR;
      string DOPlFrnpkj;
      string SXQDcpdqNd;
      string FyOnLebtKu;
      string fTftVqnJUS;
      string hJGjKICzEJ;
      string NruFMFAaZZ;
      string PsepDyqWhl;
      string CTVuwwqdcC;
      string bcoBgmYZgQ;
      string qMiuCodQgM;
      string ReRSGtWAJP;
      string NVyWxmDcJP;
      string ZQpQtpjZrH;
      string SaxPkwLWtm;
      string QXQQCUNLxP;
      string kAHQlAoAXQ;
      string JwHCxmXhlB;
      string aghVzCAYNY;
      string bXccVRSgdk;
      if(gTDhzqiIjR == qMiuCodQgM){HKydcTPNSP = true;}
      else if(qMiuCodQgM == gTDhzqiIjR){VXdQtmoeuX = true;}
      if(DOPlFrnpkj == ReRSGtWAJP){xeZwnERkQj = true;}
      else if(ReRSGtWAJP == DOPlFrnpkj){ejcxFNuRHJ = true;}
      if(SXQDcpdqNd == NVyWxmDcJP){zQlyokTeff = true;}
      else if(NVyWxmDcJP == SXQDcpdqNd){fPKzfnxrFx = true;}
      if(FyOnLebtKu == ZQpQtpjZrH){TqEAsPhEsf = true;}
      else if(ZQpQtpjZrH == FyOnLebtKu){HskBVOIYTr = true;}
      if(fTftVqnJUS == SaxPkwLWtm){llFybwygoR = true;}
      else if(SaxPkwLWtm == fTftVqnJUS){wuOqXGFSuS = true;}
      if(hJGjKICzEJ == QXQQCUNLxP){AhPskpYzor = true;}
      else if(QXQQCUNLxP == hJGjKICzEJ){FmJrpFNejG = true;}
      if(NruFMFAaZZ == kAHQlAoAXQ){AkgmwGaZCX = true;}
      else if(kAHQlAoAXQ == NruFMFAaZZ){pxZyQkzbaq = true;}
      if(PsepDyqWhl == JwHCxmXhlB){WMrkyMLnWr = true;}
      if(CTVuwwqdcC == aghVzCAYNY){mloOelaKPD = true;}
      if(bcoBgmYZgQ == bXccVRSgdk){qhPAfAFRCV = true;}
      while(JwHCxmXhlB == PsepDyqWhl){FWxPNjBSaZ = true;}
      while(aghVzCAYNY == aghVzCAYNY){hewlrUisLP = true;}
      while(bXccVRSgdk == bXccVRSgdk){AcnNByVQal = true;}
      if(HKydcTPNSP == true){HKydcTPNSP = false;}
      if(xeZwnERkQj == true){xeZwnERkQj = false;}
      if(zQlyokTeff == true){zQlyokTeff = false;}
      if(TqEAsPhEsf == true){TqEAsPhEsf = false;}
      if(llFybwygoR == true){llFybwygoR = false;}
      if(AhPskpYzor == true){AhPskpYzor = false;}
      if(AkgmwGaZCX == true){AkgmwGaZCX = false;}
      if(WMrkyMLnWr == true){WMrkyMLnWr = false;}
      if(mloOelaKPD == true){mloOelaKPD = false;}
      if(qhPAfAFRCV == true){qhPAfAFRCV = false;}
      if(VXdQtmoeuX == true){VXdQtmoeuX = false;}
      if(ejcxFNuRHJ == true){ejcxFNuRHJ = false;}
      if(fPKzfnxrFx == true){fPKzfnxrFx = false;}
      if(HskBVOIYTr == true){HskBVOIYTr = false;}
      if(wuOqXGFSuS == true){wuOqXGFSuS = false;}
      if(FmJrpFNejG == true){FmJrpFNejG = false;}
      if(pxZyQkzbaq == true){pxZyQkzbaq = false;}
      if(FWxPNjBSaZ == true){FWxPNjBSaZ = false;}
      if(hewlrUisLP == true){hewlrUisLP = false;}
      if(AcnNByVQal == true){AcnNByVQal = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MGAIMYGYOM
{ 
  void woQboWQKDs()
  { 
      bool zxIPblyiIr = false;
      bool qnWWgAboan = false;
      bool DNVRFAyEPe = false;
      bool YbxTkHHrxd = false;
      bool mnCJjrBFYb = false;
      bool XJYqGlbpJb = false;
      bool ntTdTOqDlZ = false;
      bool sjrwBOKMhM = false;
      bool yrfmGPDGNy = false;
      bool fRhUprUYoU = false;
      bool ByTyinQbpq = false;
      bool aOIwcuOfWE = false;
      bool LkomsaBWGI = false;
      bool slXMLAYgER = false;
      bool yaQVXQJkru = false;
      bool doBjJQIfEH = false;
      bool mtVgyoqukb = false;
      bool VgbFGjTTPb = false;
      bool RPdLdEjmCu = false;
      bool rsbxeDFiXi = false;
      string blFZyNrPmW;
      string wZSUkDWXIw;
      string fpHzDMacFW;
      string ZMrHwcRNYk;
      string pWdCiUMmZz;
      string ssLceldGWk;
      string lOJPxjhLXI;
      string lJyXFIMEQh;
      string AnCGdfAObK;
      string yKqnyaMNFz;
      string mBORNjumWV;
      string ZYonkyMDlQ;
      string iTiNJaSKZe;
      string uyzRtyYkst;
      string kMqGjYVCCA;
      string NJanPSRAHm;
      string QjNraaZXtl;
      string YbKrfWbccj;
      string FPIYQlXLBb;
      string gYcFcAyzfJ;
      if(blFZyNrPmW == mBORNjumWV){zxIPblyiIr = true;}
      else if(mBORNjumWV == blFZyNrPmW){ByTyinQbpq = true;}
      if(wZSUkDWXIw == ZYonkyMDlQ){qnWWgAboan = true;}
      else if(ZYonkyMDlQ == wZSUkDWXIw){aOIwcuOfWE = true;}
      if(fpHzDMacFW == iTiNJaSKZe){DNVRFAyEPe = true;}
      else if(iTiNJaSKZe == fpHzDMacFW){LkomsaBWGI = true;}
      if(ZMrHwcRNYk == uyzRtyYkst){YbxTkHHrxd = true;}
      else if(uyzRtyYkst == ZMrHwcRNYk){slXMLAYgER = true;}
      if(pWdCiUMmZz == kMqGjYVCCA){mnCJjrBFYb = true;}
      else if(kMqGjYVCCA == pWdCiUMmZz){yaQVXQJkru = true;}
      if(ssLceldGWk == NJanPSRAHm){XJYqGlbpJb = true;}
      else if(NJanPSRAHm == ssLceldGWk){doBjJQIfEH = true;}
      if(lOJPxjhLXI == QjNraaZXtl){ntTdTOqDlZ = true;}
      else if(QjNraaZXtl == lOJPxjhLXI){mtVgyoqukb = true;}
      if(lJyXFIMEQh == YbKrfWbccj){sjrwBOKMhM = true;}
      if(AnCGdfAObK == FPIYQlXLBb){yrfmGPDGNy = true;}
      if(yKqnyaMNFz == gYcFcAyzfJ){fRhUprUYoU = true;}
      while(YbKrfWbccj == lJyXFIMEQh){VgbFGjTTPb = true;}
      while(FPIYQlXLBb == FPIYQlXLBb){RPdLdEjmCu = true;}
      while(gYcFcAyzfJ == gYcFcAyzfJ){rsbxeDFiXi = true;}
      if(zxIPblyiIr == true){zxIPblyiIr = false;}
      if(qnWWgAboan == true){qnWWgAboan = false;}
      if(DNVRFAyEPe == true){DNVRFAyEPe = false;}
      if(YbxTkHHrxd == true){YbxTkHHrxd = false;}
      if(mnCJjrBFYb == true){mnCJjrBFYb = false;}
      if(XJYqGlbpJb == true){XJYqGlbpJb = false;}
      if(ntTdTOqDlZ == true){ntTdTOqDlZ = false;}
      if(sjrwBOKMhM == true){sjrwBOKMhM = false;}
      if(yrfmGPDGNy == true){yrfmGPDGNy = false;}
      if(fRhUprUYoU == true){fRhUprUYoU = false;}
      if(ByTyinQbpq == true){ByTyinQbpq = false;}
      if(aOIwcuOfWE == true){aOIwcuOfWE = false;}
      if(LkomsaBWGI == true){LkomsaBWGI = false;}
      if(slXMLAYgER == true){slXMLAYgER = false;}
      if(yaQVXQJkru == true){yaQVXQJkru = false;}
      if(doBjJQIfEH == true){doBjJQIfEH = false;}
      if(mtVgyoqukb == true){mtVgyoqukb = false;}
      if(VgbFGjTTPb == true){VgbFGjTTPb = false;}
      if(RPdLdEjmCu == true){RPdLdEjmCu = false;}
      if(rsbxeDFiXi == true){rsbxeDFiXi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KQKRRQASUC
{ 
  void EZwYprBISb()
  { 
      bool jCsJcharVZ = false;
      bool DGctbEgDbP = false;
      bool CZgjHxaerK = false;
      bool XdRoIFsDIN = false;
      bool KKJfQjpFAM = false;
      bool AYWEqJUJBe = false;
      bool HuzKCZomlV = false;
      bool LctOErQBTW = false;
      bool GUdeKyHMyy = false;
      bool kiCdFPKydp = false;
      bool uDSdBWeHzz = false;
      bool sTpdYlQTAE = false;
      bool RbnnkbaQIo = false;
      bool gRXcbCBAdn = false;
      bool TzNzEQsXES = false;
      bool yqxYXSOesY = false;
      bool uNzKSawMCV = false;
      bool jQpMkSOoiU = false;
      bool hDLpqpUyEI = false;
      bool nWHErFACfc = false;
      string hxXEwnFEHs;
      string dFeoaQjacN;
      string pxZbPdQnnD;
      string dYFhuJHsKK;
      string lTXyAhupjr;
      string ShlErjXwnR;
      string WQQDPFAxtR;
      string JpPSaWYabh;
      string haEwkulApK;
      string kKErnVCUht;
      string gyuSibSTdU;
      string KxUGiuMUSU;
      string SAGifykWdF;
      string TErgBjelMI;
      string cSWUznpyJr;
      string IhBNagrMlj;
      string quoXBFEmEz;
      string PZHJrAckjr;
      string ImNkwkCeGU;
      string qSzcRgcyTt;
      if(hxXEwnFEHs == gyuSibSTdU){jCsJcharVZ = true;}
      else if(gyuSibSTdU == hxXEwnFEHs){uDSdBWeHzz = true;}
      if(dFeoaQjacN == KxUGiuMUSU){DGctbEgDbP = true;}
      else if(KxUGiuMUSU == dFeoaQjacN){sTpdYlQTAE = true;}
      if(pxZbPdQnnD == SAGifykWdF){CZgjHxaerK = true;}
      else if(SAGifykWdF == pxZbPdQnnD){RbnnkbaQIo = true;}
      if(dYFhuJHsKK == TErgBjelMI){XdRoIFsDIN = true;}
      else if(TErgBjelMI == dYFhuJHsKK){gRXcbCBAdn = true;}
      if(lTXyAhupjr == cSWUznpyJr){KKJfQjpFAM = true;}
      else if(cSWUznpyJr == lTXyAhupjr){TzNzEQsXES = true;}
      if(ShlErjXwnR == IhBNagrMlj){AYWEqJUJBe = true;}
      else if(IhBNagrMlj == ShlErjXwnR){yqxYXSOesY = true;}
      if(WQQDPFAxtR == quoXBFEmEz){HuzKCZomlV = true;}
      else if(quoXBFEmEz == WQQDPFAxtR){uNzKSawMCV = true;}
      if(JpPSaWYabh == PZHJrAckjr){LctOErQBTW = true;}
      if(haEwkulApK == ImNkwkCeGU){GUdeKyHMyy = true;}
      if(kKErnVCUht == qSzcRgcyTt){kiCdFPKydp = true;}
      while(PZHJrAckjr == JpPSaWYabh){jQpMkSOoiU = true;}
      while(ImNkwkCeGU == ImNkwkCeGU){hDLpqpUyEI = true;}
      while(qSzcRgcyTt == qSzcRgcyTt){nWHErFACfc = true;}
      if(jCsJcharVZ == true){jCsJcharVZ = false;}
      if(DGctbEgDbP == true){DGctbEgDbP = false;}
      if(CZgjHxaerK == true){CZgjHxaerK = false;}
      if(XdRoIFsDIN == true){XdRoIFsDIN = false;}
      if(KKJfQjpFAM == true){KKJfQjpFAM = false;}
      if(AYWEqJUJBe == true){AYWEqJUJBe = false;}
      if(HuzKCZomlV == true){HuzKCZomlV = false;}
      if(LctOErQBTW == true){LctOErQBTW = false;}
      if(GUdeKyHMyy == true){GUdeKyHMyy = false;}
      if(kiCdFPKydp == true){kiCdFPKydp = false;}
      if(uDSdBWeHzz == true){uDSdBWeHzz = false;}
      if(sTpdYlQTAE == true){sTpdYlQTAE = false;}
      if(RbnnkbaQIo == true){RbnnkbaQIo = false;}
      if(gRXcbCBAdn == true){gRXcbCBAdn = false;}
      if(TzNzEQsXES == true){TzNzEQsXES = false;}
      if(yqxYXSOesY == true){yqxYXSOesY = false;}
      if(uNzKSawMCV == true){uNzKSawMCV = false;}
      if(jQpMkSOoiU == true){jQpMkSOoiU = false;}
      if(hDLpqpUyEI == true){hDLpqpUyEI = false;}
      if(nWHErFACfc == true){nWHErFACfc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BRXDSGCZKT
{ 
  void YyzAHsBBoN()
  { 
      bool FCwJbaQSZi = false;
      bool UsrDHBrPwZ = false;
      bool tuXiLxHXgD = false;
      bool pYeVTGCrIk = false;
      bool WEnUUATaAj = false;
      bool wmTgPKdMye = false;
      bool SskWrwNViB = false;
      bool zzGLCoHDOL = false;
      bool cgXxHDExGJ = false;
      bool botcRdTRbY = false;
      bool lxqyjwfHeZ = false;
      bool CZymBkUmOo = false;
      bool qQPYZJFdsY = false;
      bool VoapYfpZMK = false;
      bool WwSfHBqHbg = false;
      bool IfCmgVhRYa = false;
      bool qCOFENzINU = false;
      bool uuaDRtxiRX = false;
      bool SxBMPplmID = false;
      bool yXBlTUhNbs = false;
      string ZRIBVITltu;
      string DlbhZcwyiP;
      string SAxtJcBuFj;
      string VmUBZbCjmk;
      string ghAUonOlSd;
      string eYROiZTOrr;
      string HYfJZQRaHy;
      string aLLxhkzcFU;
      string NOcbFxVFSA;
      string HnrtcIBpuX;
      string VneUDTMTgx;
      string ZqJWesuEEb;
      string uMHGpmSRUY;
      string jIggyZjIYl;
      string jBXUaPeUOj;
      string NwkScJfXdQ;
      string AuzHNQKbYF;
      string IlfSrsnmfZ;
      string IyCjLmWOEo;
      string sLeIknCPHJ;
      if(ZRIBVITltu == VneUDTMTgx){FCwJbaQSZi = true;}
      else if(VneUDTMTgx == ZRIBVITltu){lxqyjwfHeZ = true;}
      if(DlbhZcwyiP == ZqJWesuEEb){UsrDHBrPwZ = true;}
      else if(ZqJWesuEEb == DlbhZcwyiP){CZymBkUmOo = true;}
      if(SAxtJcBuFj == uMHGpmSRUY){tuXiLxHXgD = true;}
      else if(uMHGpmSRUY == SAxtJcBuFj){qQPYZJFdsY = true;}
      if(VmUBZbCjmk == jIggyZjIYl){pYeVTGCrIk = true;}
      else if(jIggyZjIYl == VmUBZbCjmk){VoapYfpZMK = true;}
      if(ghAUonOlSd == jBXUaPeUOj){WEnUUATaAj = true;}
      else if(jBXUaPeUOj == ghAUonOlSd){WwSfHBqHbg = true;}
      if(eYROiZTOrr == NwkScJfXdQ){wmTgPKdMye = true;}
      else if(NwkScJfXdQ == eYROiZTOrr){IfCmgVhRYa = true;}
      if(HYfJZQRaHy == AuzHNQKbYF){SskWrwNViB = true;}
      else if(AuzHNQKbYF == HYfJZQRaHy){qCOFENzINU = true;}
      if(aLLxhkzcFU == IlfSrsnmfZ){zzGLCoHDOL = true;}
      if(NOcbFxVFSA == IyCjLmWOEo){cgXxHDExGJ = true;}
      if(HnrtcIBpuX == sLeIknCPHJ){botcRdTRbY = true;}
      while(IlfSrsnmfZ == aLLxhkzcFU){uuaDRtxiRX = true;}
      while(IyCjLmWOEo == IyCjLmWOEo){SxBMPplmID = true;}
      while(sLeIknCPHJ == sLeIknCPHJ){yXBlTUhNbs = true;}
      if(FCwJbaQSZi == true){FCwJbaQSZi = false;}
      if(UsrDHBrPwZ == true){UsrDHBrPwZ = false;}
      if(tuXiLxHXgD == true){tuXiLxHXgD = false;}
      if(pYeVTGCrIk == true){pYeVTGCrIk = false;}
      if(WEnUUATaAj == true){WEnUUATaAj = false;}
      if(wmTgPKdMye == true){wmTgPKdMye = false;}
      if(SskWrwNViB == true){SskWrwNViB = false;}
      if(zzGLCoHDOL == true){zzGLCoHDOL = false;}
      if(cgXxHDExGJ == true){cgXxHDExGJ = false;}
      if(botcRdTRbY == true){botcRdTRbY = false;}
      if(lxqyjwfHeZ == true){lxqyjwfHeZ = false;}
      if(CZymBkUmOo == true){CZymBkUmOo = false;}
      if(qQPYZJFdsY == true){qQPYZJFdsY = false;}
      if(VoapYfpZMK == true){VoapYfpZMK = false;}
      if(WwSfHBqHbg == true){WwSfHBqHbg = false;}
      if(IfCmgVhRYa == true){IfCmgVhRYa = false;}
      if(qCOFENzINU == true){qCOFENzINU = false;}
      if(uuaDRtxiRX == true){uuaDRtxiRX = false;}
      if(SxBMPplmID == true){SxBMPplmID = false;}
      if(yXBlTUhNbs == true){yXBlTUhNbs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GGRFRXNHRB
{ 
  void hOlLyBsPwF()
  { 
      bool lDWXDwgRYW = false;
      bool HLppUbZiDg = false;
      bool bHrwsSfTOx = false;
      bool gDDMiRQZyf = false;
      bool XFyxRxQMMF = false;
      bool AsiCGdKaej = false;
      bool QNDaoEIcGC = false;
      bool KzkWJVJtdu = false;
      bool BTHIxmTAGE = false;
      bool HptNPJCXIN = false;
      bool omFKtAILrk = false;
      bool UPwihxGtcC = false;
      bool jHWBuUEWHz = false;
      bool TklPGONwOx = false;
      bool eIHUjqsLeS = false;
      bool DHtFdhQEPZ = false;
      bool ukErQidfcT = false;
      bool iJffWazBRD = false;
      bool asrnwLqflY = false;
      bool yJgoBsEPDA = false;
      string LxfhMAXKtQ;
      string UIVxDPicAZ;
      string ThOLHhXOoJ;
      string CTnOyzzNQY;
      string mapeFbPOCq;
      string QIqdSdiUZd;
      string bJUjOAUGyD;
      string PULamsqjFf;
      string RIMxrscKwU;
      string QenmwcFFQE;
      string HJPyawwPpZ;
      string lyOtLSxXrU;
      string XJJdmkgYeJ;
      string mlTBXIsNWw;
      string xNTWiRPsac;
      string fhNugxzlKo;
      string OXjglAeMWC;
      string ZiAuWJpmuU;
      string UmxBcEXXUZ;
      string JHCggZcmjh;
      if(LxfhMAXKtQ == HJPyawwPpZ){lDWXDwgRYW = true;}
      else if(HJPyawwPpZ == LxfhMAXKtQ){omFKtAILrk = true;}
      if(UIVxDPicAZ == lyOtLSxXrU){HLppUbZiDg = true;}
      else if(lyOtLSxXrU == UIVxDPicAZ){UPwihxGtcC = true;}
      if(ThOLHhXOoJ == XJJdmkgYeJ){bHrwsSfTOx = true;}
      else if(XJJdmkgYeJ == ThOLHhXOoJ){jHWBuUEWHz = true;}
      if(CTnOyzzNQY == mlTBXIsNWw){gDDMiRQZyf = true;}
      else if(mlTBXIsNWw == CTnOyzzNQY){TklPGONwOx = true;}
      if(mapeFbPOCq == xNTWiRPsac){XFyxRxQMMF = true;}
      else if(xNTWiRPsac == mapeFbPOCq){eIHUjqsLeS = true;}
      if(QIqdSdiUZd == fhNugxzlKo){AsiCGdKaej = true;}
      else if(fhNugxzlKo == QIqdSdiUZd){DHtFdhQEPZ = true;}
      if(bJUjOAUGyD == OXjglAeMWC){QNDaoEIcGC = true;}
      else if(OXjglAeMWC == bJUjOAUGyD){ukErQidfcT = true;}
      if(PULamsqjFf == ZiAuWJpmuU){KzkWJVJtdu = true;}
      if(RIMxrscKwU == UmxBcEXXUZ){BTHIxmTAGE = true;}
      if(QenmwcFFQE == JHCggZcmjh){HptNPJCXIN = true;}
      while(ZiAuWJpmuU == PULamsqjFf){iJffWazBRD = true;}
      while(UmxBcEXXUZ == UmxBcEXXUZ){asrnwLqflY = true;}
      while(JHCggZcmjh == JHCggZcmjh){yJgoBsEPDA = true;}
      if(lDWXDwgRYW == true){lDWXDwgRYW = false;}
      if(HLppUbZiDg == true){HLppUbZiDg = false;}
      if(bHrwsSfTOx == true){bHrwsSfTOx = false;}
      if(gDDMiRQZyf == true){gDDMiRQZyf = false;}
      if(XFyxRxQMMF == true){XFyxRxQMMF = false;}
      if(AsiCGdKaej == true){AsiCGdKaej = false;}
      if(QNDaoEIcGC == true){QNDaoEIcGC = false;}
      if(KzkWJVJtdu == true){KzkWJVJtdu = false;}
      if(BTHIxmTAGE == true){BTHIxmTAGE = false;}
      if(HptNPJCXIN == true){HptNPJCXIN = false;}
      if(omFKtAILrk == true){omFKtAILrk = false;}
      if(UPwihxGtcC == true){UPwihxGtcC = false;}
      if(jHWBuUEWHz == true){jHWBuUEWHz = false;}
      if(TklPGONwOx == true){TklPGONwOx = false;}
      if(eIHUjqsLeS == true){eIHUjqsLeS = false;}
      if(DHtFdhQEPZ == true){DHtFdhQEPZ = false;}
      if(ukErQidfcT == true){ukErQidfcT = false;}
      if(iJffWazBRD == true){iJffWazBRD = false;}
      if(asrnwLqflY == true){asrnwLqflY = false;}
      if(yJgoBsEPDA == true){yJgoBsEPDA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CCZWVKEXCX
{ 
  void nxPxHwmlhG()
  { 
      bool lXCFtrXQzD = false;
      bool GDAiQBwimZ = false;
      bool TxqcfWJupB = false;
      bool fJuNnDmJjb = false;
      bool zBgAKJgbLM = false;
      bool IfoohofYZW = false;
      bool icayWxImuN = false;
      bool NmzKaSQpSW = false;
      bool ogicrATFfz = false;
      bool WOjMrVZhdN = false;
      bool BtgxzVgeOO = false;
      bool YoGDgBsxFW = false;
      bool cCGwqzClRE = false;
      bool rhMEUtAkdW = false;
      bool SnqQahHfeq = false;
      bool cbcHjymjGz = false;
      bool StMJWpcChl = false;
      bool AHThDnKpUJ = false;
      bool rVHabPEFUV = false;
      bool HPeeEYbhtj = false;
      string cqJFtMFEkY;
      string ZzMPeBHqAG;
      string ClLkgItrDt;
      string GlDIBJpXGU;
      string SqUwLJreVw;
      string MMUdcpeXYl;
      string SNAqRnuENU;
      string KjjemKQPZW;
      string PPVGhtQZeL;
      string eRyUCEpddW;
      string NnRAkcbopR;
      string YOyCVDqsJX;
      string XVYBpskPch;
      string TmlDIbxVUF;
      string RbjNmPSuzz;
      string NwUFzrOPKr;
      string qMMHxkcuss;
      string FUYDGulLOa;
      string aoPdUWRfDQ;
      string MhLcNFObPB;
      if(cqJFtMFEkY == NnRAkcbopR){lXCFtrXQzD = true;}
      else if(NnRAkcbopR == cqJFtMFEkY){BtgxzVgeOO = true;}
      if(ZzMPeBHqAG == YOyCVDqsJX){GDAiQBwimZ = true;}
      else if(YOyCVDqsJX == ZzMPeBHqAG){YoGDgBsxFW = true;}
      if(ClLkgItrDt == XVYBpskPch){TxqcfWJupB = true;}
      else if(XVYBpskPch == ClLkgItrDt){cCGwqzClRE = true;}
      if(GlDIBJpXGU == TmlDIbxVUF){fJuNnDmJjb = true;}
      else if(TmlDIbxVUF == GlDIBJpXGU){rhMEUtAkdW = true;}
      if(SqUwLJreVw == RbjNmPSuzz){zBgAKJgbLM = true;}
      else if(RbjNmPSuzz == SqUwLJreVw){SnqQahHfeq = true;}
      if(MMUdcpeXYl == NwUFzrOPKr){IfoohofYZW = true;}
      else if(NwUFzrOPKr == MMUdcpeXYl){cbcHjymjGz = true;}
      if(SNAqRnuENU == qMMHxkcuss){icayWxImuN = true;}
      else if(qMMHxkcuss == SNAqRnuENU){StMJWpcChl = true;}
      if(KjjemKQPZW == FUYDGulLOa){NmzKaSQpSW = true;}
      if(PPVGhtQZeL == aoPdUWRfDQ){ogicrATFfz = true;}
      if(eRyUCEpddW == MhLcNFObPB){WOjMrVZhdN = true;}
      while(FUYDGulLOa == KjjemKQPZW){AHThDnKpUJ = true;}
      while(aoPdUWRfDQ == aoPdUWRfDQ){rVHabPEFUV = true;}
      while(MhLcNFObPB == MhLcNFObPB){HPeeEYbhtj = true;}
      if(lXCFtrXQzD == true){lXCFtrXQzD = false;}
      if(GDAiQBwimZ == true){GDAiQBwimZ = false;}
      if(TxqcfWJupB == true){TxqcfWJupB = false;}
      if(fJuNnDmJjb == true){fJuNnDmJjb = false;}
      if(zBgAKJgbLM == true){zBgAKJgbLM = false;}
      if(IfoohofYZW == true){IfoohofYZW = false;}
      if(icayWxImuN == true){icayWxImuN = false;}
      if(NmzKaSQpSW == true){NmzKaSQpSW = false;}
      if(ogicrATFfz == true){ogicrATFfz = false;}
      if(WOjMrVZhdN == true){WOjMrVZhdN = false;}
      if(BtgxzVgeOO == true){BtgxzVgeOO = false;}
      if(YoGDgBsxFW == true){YoGDgBsxFW = false;}
      if(cCGwqzClRE == true){cCGwqzClRE = false;}
      if(rhMEUtAkdW == true){rhMEUtAkdW = false;}
      if(SnqQahHfeq == true){SnqQahHfeq = false;}
      if(cbcHjymjGz == true){cbcHjymjGz = false;}
      if(StMJWpcChl == true){StMJWpcChl = false;}
      if(AHThDnKpUJ == true){AHThDnKpUJ = false;}
      if(rVHabPEFUV == true){rVHabPEFUV = false;}
      if(HPeeEYbhtj == true){HPeeEYbhtj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OOHZWAFAJN
{ 
  void yTjcikexEX()
  { 
      bool kmeVuegnRq = false;
      bool HgcNsNXzrN = false;
      bool OYhMPyrDZk = false;
      bool wBKTwEaXtS = false;
      bool OjhOVboCgz = false;
      bool yobygMESFw = false;
      bool lcNgtFVJQo = false;
      bool lzFZydpTpn = false;
      bool IpValHYLlj = false;
      bool AHRxflWBUP = false;
      bool GfkrMyjCzC = false;
      bool VkcRnwGmtm = false;
      bool fJXgodgHtM = false;
      bool HoyZgcEdNn = false;
      bool XWMiNOeAJj = false;
      bool ejGgiwIDDi = false;
      bool RbHmuflCjK = false;
      bool qcYGOJxnSZ = false;
      bool zgxJhXiRpk = false;
      bool UBZeVuPZnh = false;
      string jmJOewzEVN;
      string oYrduZZAPO;
      string WxyGyjPitU;
      string YEENJwEgcX;
      string WMyHdfaMkb;
      string pejSaloCPu;
      string gQKAnkTeUz;
      string VsXrqJyqcS;
      string IDxSetUllW;
      string HBRhpGCySs;
      string LwqlFwHqaQ;
      string nxYwldSqVt;
      string PhryqTboCk;
      string GAYjHMLKMh;
      string mdKViJbUuM;
      string mLbYcdDruk;
      string URopJmYooZ;
      string RIenGlWWPg;
      string qGtoxuYzGc;
      string JSPaNVoULu;
      if(jmJOewzEVN == LwqlFwHqaQ){kmeVuegnRq = true;}
      else if(LwqlFwHqaQ == jmJOewzEVN){GfkrMyjCzC = true;}
      if(oYrduZZAPO == nxYwldSqVt){HgcNsNXzrN = true;}
      else if(nxYwldSqVt == oYrduZZAPO){VkcRnwGmtm = true;}
      if(WxyGyjPitU == PhryqTboCk){OYhMPyrDZk = true;}
      else if(PhryqTboCk == WxyGyjPitU){fJXgodgHtM = true;}
      if(YEENJwEgcX == GAYjHMLKMh){wBKTwEaXtS = true;}
      else if(GAYjHMLKMh == YEENJwEgcX){HoyZgcEdNn = true;}
      if(WMyHdfaMkb == mdKViJbUuM){OjhOVboCgz = true;}
      else if(mdKViJbUuM == WMyHdfaMkb){XWMiNOeAJj = true;}
      if(pejSaloCPu == mLbYcdDruk){yobygMESFw = true;}
      else if(mLbYcdDruk == pejSaloCPu){ejGgiwIDDi = true;}
      if(gQKAnkTeUz == URopJmYooZ){lcNgtFVJQo = true;}
      else if(URopJmYooZ == gQKAnkTeUz){RbHmuflCjK = true;}
      if(VsXrqJyqcS == RIenGlWWPg){lzFZydpTpn = true;}
      if(IDxSetUllW == qGtoxuYzGc){IpValHYLlj = true;}
      if(HBRhpGCySs == JSPaNVoULu){AHRxflWBUP = true;}
      while(RIenGlWWPg == VsXrqJyqcS){qcYGOJxnSZ = true;}
      while(qGtoxuYzGc == qGtoxuYzGc){zgxJhXiRpk = true;}
      while(JSPaNVoULu == JSPaNVoULu){UBZeVuPZnh = true;}
      if(kmeVuegnRq == true){kmeVuegnRq = false;}
      if(HgcNsNXzrN == true){HgcNsNXzrN = false;}
      if(OYhMPyrDZk == true){OYhMPyrDZk = false;}
      if(wBKTwEaXtS == true){wBKTwEaXtS = false;}
      if(OjhOVboCgz == true){OjhOVboCgz = false;}
      if(yobygMESFw == true){yobygMESFw = false;}
      if(lcNgtFVJQo == true){lcNgtFVJQo = false;}
      if(lzFZydpTpn == true){lzFZydpTpn = false;}
      if(IpValHYLlj == true){IpValHYLlj = false;}
      if(AHRxflWBUP == true){AHRxflWBUP = false;}
      if(GfkrMyjCzC == true){GfkrMyjCzC = false;}
      if(VkcRnwGmtm == true){VkcRnwGmtm = false;}
      if(fJXgodgHtM == true){fJXgodgHtM = false;}
      if(HoyZgcEdNn == true){HoyZgcEdNn = false;}
      if(XWMiNOeAJj == true){XWMiNOeAJj = false;}
      if(ejGgiwIDDi == true){ejGgiwIDDi = false;}
      if(RbHmuflCjK == true){RbHmuflCjK = false;}
      if(qcYGOJxnSZ == true){qcYGOJxnSZ = false;}
      if(zgxJhXiRpk == true){zgxJhXiRpk = false;}
      if(UBZeVuPZnh == true){UBZeVuPZnh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BEOTCKWIYG
{ 
  void kDdcPWWwLX()
  { 
      bool VFeQPqQmRX = false;
      bool UgntmwOqSR = false;
      bool HdTPSjuDuf = false;
      bool VyJDHTVIey = false;
      bool ZeypwPkIzS = false;
      bool GpIJDqlOye = false;
      bool uNqkCWWjmT = false;
      bool aGMUrbfcTC = false;
      bool zmEbZeoAAP = false;
      bool gGKzCMmMnZ = false;
      bool sAUerAcmRb = false;
      bool LJuueHIOLW = false;
      bool gVjXmTwMSe = false;
      bool nSntJzSYjZ = false;
      bool BBiXEXLOTt = false;
      bool DYBRrWPxrY = false;
      bool EQUlnrUBkf = false;
      bool oiekUelMSP = false;
      bool IgsXaPQWBl = false;
      bool iUgLVkgxjL = false;
      string jFlSgbOlDa;
      string DmEddprCKa;
      string HGKcFBJxWG;
      string LJFrDoSVne;
      string fMSuqlzdFx;
      string hHkhBctINe;
      string SQPwPtgLZn;
      string WWNftumPlS;
      string BfrZYDXXqa;
      string uKIILOwWfw;
      string FmDQZLsFmG;
      string pdWuclYoXL;
      string VJUzsMNFTe;
      string yQjoMOcazM;
      string JSjjjQmNfY;
      string pGyTauRDXM;
      string xnkIIYtghO;
      string JypKBDZkOc;
      string ZAYtoqRPNR;
      string wYClPVgbaK;
      if(jFlSgbOlDa == FmDQZLsFmG){VFeQPqQmRX = true;}
      else if(FmDQZLsFmG == jFlSgbOlDa){sAUerAcmRb = true;}
      if(DmEddprCKa == pdWuclYoXL){UgntmwOqSR = true;}
      else if(pdWuclYoXL == DmEddprCKa){LJuueHIOLW = true;}
      if(HGKcFBJxWG == VJUzsMNFTe){HdTPSjuDuf = true;}
      else if(VJUzsMNFTe == HGKcFBJxWG){gVjXmTwMSe = true;}
      if(LJFrDoSVne == yQjoMOcazM){VyJDHTVIey = true;}
      else if(yQjoMOcazM == LJFrDoSVne){nSntJzSYjZ = true;}
      if(fMSuqlzdFx == JSjjjQmNfY){ZeypwPkIzS = true;}
      else if(JSjjjQmNfY == fMSuqlzdFx){BBiXEXLOTt = true;}
      if(hHkhBctINe == pGyTauRDXM){GpIJDqlOye = true;}
      else if(pGyTauRDXM == hHkhBctINe){DYBRrWPxrY = true;}
      if(SQPwPtgLZn == xnkIIYtghO){uNqkCWWjmT = true;}
      else if(xnkIIYtghO == SQPwPtgLZn){EQUlnrUBkf = true;}
      if(WWNftumPlS == JypKBDZkOc){aGMUrbfcTC = true;}
      if(BfrZYDXXqa == ZAYtoqRPNR){zmEbZeoAAP = true;}
      if(uKIILOwWfw == wYClPVgbaK){gGKzCMmMnZ = true;}
      while(JypKBDZkOc == WWNftumPlS){oiekUelMSP = true;}
      while(ZAYtoqRPNR == ZAYtoqRPNR){IgsXaPQWBl = true;}
      while(wYClPVgbaK == wYClPVgbaK){iUgLVkgxjL = true;}
      if(VFeQPqQmRX == true){VFeQPqQmRX = false;}
      if(UgntmwOqSR == true){UgntmwOqSR = false;}
      if(HdTPSjuDuf == true){HdTPSjuDuf = false;}
      if(VyJDHTVIey == true){VyJDHTVIey = false;}
      if(ZeypwPkIzS == true){ZeypwPkIzS = false;}
      if(GpIJDqlOye == true){GpIJDqlOye = false;}
      if(uNqkCWWjmT == true){uNqkCWWjmT = false;}
      if(aGMUrbfcTC == true){aGMUrbfcTC = false;}
      if(zmEbZeoAAP == true){zmEbZeoAAP = false;}
      if(gGKzCMmMnZ == true){gGKzCMmMnZ = false;}
      if(sAUerAcmRb == true){sAUerAcmRb = false;}
      if(LJuueHIOLW == true){LJuueHIOLW = false;}
      if(gVjXmTwMSe == true){gVjXmTwMSe = false;}
      if(nSntJzSYjZ == true){nSntJzSYjZ = false;}
      if(BBiXEXLOTt == true){BBiXEXLOTt = false;}
      if(DYBRrWPxrY == true){DYBRrWPxrY = false;}
      if(EQUlnrUBkf == true){EQUlnrUBkf = false;}
      if(oiekUelMSP == true){oiekUelMSP = false;}
      if(IgsXaPQWBl == true){IgsXaPQWBl = false;}
      if(iUgLVkgxjL == true){iUgLVkgxjL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SPDFKYABGP
{ 
  void cdCKrJukBl()
  { 
      bool HPSWtiZoSf = false;
      bool aIUzFUUpkT = false;
      bool hyYtGGHgOt = false;
      bool HCAOilMHuS = false;
      bool ZNhTRUkOPw = false;
      bool CaxQmJpYPn = false;
      bool asaXXXHFPT = false;
      bool INtSsTQFbk = false;
      bool UsQRrgEiaZ = false;
      bool unqHqgmKus = false;
      bool NezwnofeWT = false;
      bool rkwdShbIfq = false;
      bool TXdoXTVmaM = false;
      bool fGKhXotUdD = false;
      bool AYLHhKrMoI = false;
      bool EeeDZoBJXT = false;
      bool SsDKmlZNFc = false;
      bool nHebpuMWHM = false;
      bool bIySIBbGIi = false;
      bool njwJjHUBTw = false;
      string ScXaKnAmlp;
      string bVdZiozpgq;
      string zQgKLpKdgg;
      string udzkRpgNlH;
      string COYUaoeDiG;
      string ZnDihBwNOU;
      string dVFFuyWsHr;
      string IlDZnkrLiW;
      string KGuZGGaPCQ;
      string sSzRcfITZc;
      string Rlwpxnjctx;
      string JkerJWafyn;
      string WGqqzpmlwH;
      string cXkFEYKEcS;
      string VxVFGHnGHG;
      string FcUkFLtEpH;
      string ByfmTkzYmk;
      string LVCfRYkljq;
      string BfdpCADAzs;
      string dFNVRXRikV;
      if(ScXaKnAmlp == Rlwpxnjctx){HPSWtiZoSf = true;}
      else if(Rlwpxnjctx == ScXaKnAmlp){NezwnofeWT = true;}
      if(bVdZiozpgq == JkerJWafyn){aIUzFUUpkT = true;}
      else if(JkerJWafyn == bVdZiozpgq){rkwdShbIfq = true;}
      if(zQgKLpKdgg == WGqqzpmlwH){hyYtGGHgOt = true;}
      else if(WGqqzpmlwH == zQgKLpKdgg){TXdoXTVmaM = true;}
      if(udzkRpgNlH == cXkFEYKEcS){HCAOilMHuS = true;}
      else if(cXkFEYKEcS == udzkRpgNlH){fGKhXotUdD = true;}
      if(COYUaoeDiG == VxVFGHnGHG){ZNhTRUkOPw = true;}
      else if(VxVFGHnGHG == COYUaoeDiG){AYLHhKrMoI = true;}
      if(ZnDihBwNOU == FcUkFLtEpH){CaxQmJpYPn = true;}
      else if(FcUkFLtEpH == ZnDihBwNOU){EeeDZoBJXT = true;}
      if(dVFFuyWsHr == ByfmTkzYmk){asaXXXHFPT = true;}
      else if(ByfmTkzYmk == dVFFuyWsHr){SsDKmlZNFc = true;}
      if(IlDZnkrLiW == LVCfRYkljq){INtSsTQFbk = true;}
      if(KGuZGGaPCQ == BfdpCADAzs){UsQRrgEiaZ = true;}
      if(sSzRcfITZc == dFNVRXRikV){unqHqgmKus = true;}
      while(LVCfRYkljq == IlDZnkrLiW){nHebpuMWHM = true;}
      while(BfdpCADAzs == BfdpCADAzs){bIySIBbGIi = true;}
      while(dFNVRXRikV == dFNVRXRikV){njwJjHUBTw = true;}
      if(HPSWtiZoSf == true){HPSWtiZoSf = false;}
      if(aIUzFUUpkT == true){aIUzFUUpkT = false;}
      if(hyYtGGHgOt == true){hyYtGGHgOt = false;}
      if(HCAOilMHuS == true){HCAOilMHuS = false;}
      if(ZNhTRUkOPw == true){ZNhTRUkOPw = false;}
      if(CaxQmJpYPn == true){CaxQmJpYPn = false;}
      if(asaXXXHFPT == true){asaXXXHFPT = false;}
      if(INtSsTQFbk == true){INtSsTQFbk = false;}
      if(UsQRrgEiaZ == true){UsQRrgEiaZ = false;}
      if(unqHqgmKus == true){unqHqgmKus = false;}
      if(NezwnofeWT == true){NezwnofeWT = false;}
      if(rkwdShbIfq == true){rkwdShbIfq = false;}
      if(TXdoXTVmaM == true){TXdoXTVmaM = false;}
      if(fGKhXotUdD == true){fGKhXotUdD = false;}
      if(AYLHhKrMoI == true){AYLHhKrMoI = false;}
      if(EeeDZoBJXT == true){EeeDZoBJXT = false;}
      if(SsDKmlZNFc == true){SsDKmlZNFc = false;}
      if(nHebpuMWHM == true){nHebpuMWHM = false;}
      if(bIySIBbGIi == true){bIySIBbGIi = false;}
      if(njwJjHUBTw == true){njwJjHUBTw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ICSLWSGPSP
{ 
  void BSYbIPwNCt()
  { 
      bool CQSeFAPMoC = false;
      bool WFUaBxxRmh = false;
      bool EGyDyhoPCn = false;
      bool FNJidGqcKV = false;
      bool uOFToWAPmx = false;
      bool pqWzDwSPgX = false;
      bool LAJAxJulxu = false;
      bool FTGRlpAdNa = false;
      bool gYcFyOpJeg = false;
      bool nbwmJuFFWf = false;
      bool bOSqqVkENN = false;
      bool unCFplXDBh = false;
      bool SkGPHdXSHq = false;
      bool kuGhzRGkOc = false;
      bool dlwQkzYpwX = false;
      bool XBDnMpICjN = false;
      bool xRxVczhTFY = false;
      bool floYEgoodI = false;
      bool EgViTirRVJ = false;
      bool JDABeLnZml = false;
      string QixXVeIrLu;
      string fnNGbPaXkC;
      string OFXfFcNkfI;
      string IcNrRaMYAN;
      string kxLYXLlisk;
      string DseeqLKRRr;
      string RXLDlCZUnA;
      string DBXWQRszIt;
      string qdrtEPyhBG;
      string PtHHwMkImU;
      string lQtrgCVHNZ;
      string RLIXBmfRzV;
      string EqTgjItPZE;
      string fkLmHZizMC;
      string tpTboDliGJ;
      string gTJQQtxNHY;
      string HGhJsfAUPa;
      string DfExbNWrrg;
      string WKIQWLbtWM;
      string cVqntmIJhI;
      if(QixXVeIrLu == lQtrgCVHNZ){CQSeFAPMoC = true;}
      else if(lQtrgCVHNZ == QixXVeIrLu){bOSqqVkENN = true;}
      if(fnNGbPaXkC == RLIXBmfRzV){WFUaBxxRmh = true;}
      else if(RLIXBmfRzV == fnNGbPaXkC){unCFplXDBh = true;}
      if(OFXfFcNkfI == EqTgjItPZE){EGyDyhoPCn = true;}
      else if(EqTgjItPZE == OFXfFcNkfI){SkGPHdXSHq = true;}
      if(IcNrRaMYAN == fkLmHZizMC){FNJidGqcKV = true;}
      else if(fkLmHZizMC == IcNrRaMYAN){kuGhzRGkOc = true;}
      if(kxLYXLlisk == tpTboDliGJ){uOFToWAPmx = true;}
      else if(tpTboDliGJ == kxLYXLlisk){dlwQkzYpwX = true;}
      if(DseeqLKRRr == gTJQQtxNHY){pqWzDwSPgX = true;}
      else if(gTJQQtxNHY == DseeqLKRRr){XBDnMpICjN = true;}
      if(RXLDlCZUnA == HGhJsfAUPa){LAJAxJulxu = true;}
      else if(HGhJsfAUPa == RXLDlCZUnA){xRxVczhTFY = true;}
      if(DBXWQRszIt == DfExbNWrrg){FTGRlpAdNa = true;}
      if(qdrtEPyhBG == WKIQWLbtWM){gYcFyOpJeg = true;}
      if(PtHHwMkImU == cVqntmIJhI){nbwmJuFFWf = true;}
      while(DfExbNWrrg == DBXWQRszIt){floYEgoodI = true;}
      while(WKIQWLbtWM == WKIQWLbtWM){EgViTirRVJ = true;}
      while(cVqntmIJhI == cVqntmIJhI){JDABeLnZml = true;}
      if(CQSeFAPMoC == true){CQSeFAPMoC = false;}
      if(WFUaBxxRmh == true){WFUaBxxRmh = false;}
      if(EGyDyhoPCn == true){EGyDyhoPCn = false;}
      if(FNJidGqcKV == true){FNJidGqcKV = false;}
      if(uOFToWAPmx == true){uOFToWAPmx = false;}
      if(pqWzDwSPgX == true){pqWzDwSPgX = false;}
      if(LAJAxJulxu == true){LAJAxJulxu = false;}
      if(FTGRlpAdNa == true){FTGRlpAdNa = false;}
      if(gYcFyOpJeg == true){gYcFyOpJeg = false;}
      if(nbwmJuFFWf == true){nbwmJuFFWf = false;}
      if(bOSqqVkENN == true){bOSqqVkENN = false;}
      if(unCFplXDBh == true){unCFplXDBh = false;}
      if(SkGPHdXSHq == true){SkGPHdXSHq = false;}
      if(kuGhzRGkOc == true){kuGhzRGkOc = false;}
      if(dlwQkzYpwX == true){dlwQkzYpwX = false;}
      if(XBDnMpICjN == true){XBDnMpICjN = false;}
      if(xRxVczhTFY == true){xRxVczhTFY = false;}
      if(floYEgoodI == true){floYEgoodI = false;}
      if(EgViTirRVJ == true){EgViTirRVJ = false;}
      if(JDABeLnZml == true){JDABeLnZml = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YZPVFINQGO
{ 
  void NSNZiDLuPA()
  { 
      bool rEVipQSTUr = false;
      bool ThPpFLbuzb = false;
      bool rtMAeYwISq = false;
      bool ORHVPdRfgS = false;
      bool JVjFcmOddX = false;
      bool WPsxMyqUER = false;
      bool TJAddCZtaa = false;
      bool tuOfRWJHXq = false;
      bool KSduUyXKJQ = false;
      bool lwbOzAUGba = false;
      bool NTDCdaBMeo = false;
      bool njxNHBeFIe = false;
      bool dPjJoxTzgR = false;
      bool CDexcpOaQs = false;
      bool wLeTcBzOHx = false;
      bool UPxwwyitNg = false;
      bool HCfMmjEAjR = false;
      bool mDoPiSjQgm = false;
      bool xEqjxmGiAe = false;
      bool GUNilsfSfr = false;
      string eltsFZaGcz;
      string ulIulyMbyI;
      string MMNdaMaXiz;
      string mdTbAeILKQ;
      string VyoEoiEAFa;
      string IGQoGSXQRr;
      string yCXdMRRfuH;
      string oBQhlNXInr;
      string IZRHqfMeVh;
      string HrsSRTaKrW;
      string QYEykGoiCr;
      string tPbZjXkPmg;
      string EmKNzxHyQS;
      string mMQQajVTWj;
      string pdcdmqPTis;
      string hKXLSdOqym;
      string zqSjEhzKyA;
      string CRrDpZJgaI;
      string hmPWjboEyx;
      string VdUijyLrYE;
      if(eltsFZaGcz == QYEykGoiCr){rEVipQSTUr = true;}
      else if(QYEykGoiCr == eltsFZaGcz){NTDCdaBMeo = true;}
      if(ulIulyMbyI == tPbZjXkPmg){ThPpFLbuzb = true;}
      else if(tPbZjXkPmg == ulIulyMbyI){njxNHBeFIe = true;}
      if(MMNdaMaXiz == EmKNzxHyQS){rtMAeYwISq = true;}
      else if(EmKNzxHyQS == MMNdaMaXiz){dPjJoxTzgR = true;}
      if(mdTbAeILKQ == mMQQajVTWj){ORHVPdRfgS = true;}
      else if(mMQQajVTWj == mdTbAeILKQ){CDexcpOaQs = true;}
      if(VyoEoiEAFa == pdcdmqPTis){JVjFcmOddX = true;}
      else if(pdcdmqPTis == VyoEoiEAFa){wLeTcBzOHx = true;}
      if(IGQoGSXQRr == hKXLSdOqym){WPsxMyqUER = true;}
      else if(hKXLSdOqym == IGQoGSXQRr){UPxwwyitNg = true;}
      if(yCXdMRRfuH == zqSjEhzKyA){TJAddCZtaa = true;}
      else if(zqSjEhzKyA == yCXdMRRfuH){HCfMmjEAjR = true;}
      if(oBQhlNXInr == CRrDpZJgaI){tuOfRWJHXq = true;}
      if(IZRHqfMeVh == hmPWjboEyx){KSduUyXKJQ = true;}
      if(HrsSRTaKrW == VdUijyLrYE){lwbOzAUGba = true;}
      while(CRrDpZJgaI == oBQhlNXInr){mDoPiSjQgm = true;}
      while(hmPWjboEyx == hmPWjboEyx){xEqjxmGiAe = true;}
      while(VdUijyLrYE == VdUijyLrYE){GUNilsfSfr = true;}
      if(rEVipQSTUr == true){rEVipQSTUr = false;}
      if(ThPpFLbuzb == true){ThPpFLbuzb = false;}
      if(rtMAeYwISq == true){rtMAeYwISq = false;}
      if(ORHVPdRfgS == true){ORHVPdRfgS = false;}
      if(JVjFcmOddX == true){JVjFcmOddX = false;}
      if(WPsxMyqUER == true){WPsxMyqUER = false;}
      if(TJAddCZtaa == true){TJAddCZtaa = false;}
      if(tuOfRWJHXq == true){tuOfRWJHXq = false;}
      if(KSduUyXKJQ == true){KSduUyXKJQ = false;}
      if(lwbOzAUGba == true){lwbOzAUGba = false;}
      if(NTDCdaBMeo == true){NTDCdaBMeo = false;}
      if(njxNHBeFIe == true){njxNHBeFIe = false;}
      if(dPjJoxTzgR == true){dPjJoxTzgR = false;}
      if(CDexcpOaQs == true){CDexcpOaQs = false;}
      if(wLeTcBzOHx == true){wLeTcBzOHx = false;}
      if(UPxwwyitNg == true){UPxwwyitNg = false;}
      if(HCfMmjEAjR == true){HCfMmjEAjR = false;}
      if(mDoPiSjQgm == true){mDoPiSjQgm = false;}
      if(xEqjxmGiAe == true){xEqjxmGiAe = false;}
      if(GUNilsfSfr == true){GUNilsfSfr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BQUQMLXPCP
{ 
  void dKbCwfWRWZ()
  { 
      bool jZpJXYxztN = false;
      bool EnNmPNckGy = false;
      bool NAQBAxdHdU = false;
      bool GWMDIpYOXL = false;
      bool xiOqPTRqMN = false;
      bool iMNzdjIoJa = false;
      bool tpVZGgaKdY = false;
      bool oOMuBdkZlg = false;
      bool fltQktJMZY = false;
      bool gjFxQHNojh = false;
      bool rGNnYUzRyz = false;
      bool ziDbtIuFnM = false;
      bool WOkFqEIPes = false;
      bool QwoDnYYGqQ = false;
      bool tqNZYxgcCI = false;
      bool LIBXKdVHwT = false;
      bool LjmEhDtznV = false;
      bool ioNWSfAKfs = false;
      bool WfNVhItRrK = false;
      bool DQKWEJDJqV = false;
      string gjZbzVZZHT;
      string rKGQStPWcP;
      string TrgMdcZaik;
      string SBpnMqyrBN;
      string zQmYqRUuuk;
      string aMsXZoCMxV;
      string uypVBqTREb;
      string uWUOQCFIVM;
      string IeexKFIROW;
      string nkueRAZQXA;
      string xCpToEqdHW;
      string jtqjNQTrLu;
      string hBDhmKapxQ;
      string cnkVOGnKIF;
      string NdKrmstLRA;
      string ghFbRuyIge;
      string dBgpsuJUUH;
      string cIjFgEqKts;
      string PPYwjuhJET;
      string HedcbMxTyq;
      if(gjZbzVZZHT == xCpToEqdHW){jZpJXYxztN = true;}
      else if(xCpToEqdHW == gjZbzVZZHT){rGNnYUzRyz = true;}
      if(rKGQStPWcP == jtqjNQTrLu){EnNmPNckGy = true;}
      else if(jtqjNQTrLu == rKGQStPWcP){ziDbtIuFnM = true;}
      if(TrgMdcZaik == hBDhmKapxQ){NAQBAxdHdU = true;}
      else if(hBDhmKapxQ == TrgMdcZaik){WOkFqEIPes = true;}
      if(SBpnMqyrBN == cnkVOGnKIF){GWMDIpYOXL = true;}
      else if(cnkVOGnKIF == SBpnMqyrBN){QwoDnYYGqQ = true;}
      if(zQmYqRUuuk == NdKrmstLRA){xiOqPTRqMN = true;}
      else if(NdKrmstLRA == zQmYqRUuuk){tqNZYxgcCI = true;}
      if(aMsXZoCMxV == ghFbRuyIge){iMNzdjIoJa = true;}
      else if(ghFbRuyIge == aMsXZoCMxV){LIBXKdVHwT = true;}
      if(uypVBqTREb == dBgpsuJUUH){tpVZGgaKdY = true;}
      else if(dBgpsuJUUH == uypVBqTREb){LjmEhDtznV = true;}
      if(uWUOQCFIVM == cIjFgEqKts){oOMuBdkZlg = true;}
      if(IeexKFIROW == PPYwjuhJET){fltQktJMZY = true;}
      if(nkueRAZQXA == HedcbMxTyq){gjFxQHNojh = true;}
      while(cIjFgEqKts == uWUOQCFIVM){ioNWSfAKfs = true;}
      while(PPYwjuhJET == PPYwjuhJET){WfNVhItRrK = true;}
      while(HedcbMxTyq == HedcbMxTyq){DQKWEJDJqV = true;}
      if(jZpJXYxztN == true){jZpJXYxztN = false;}
      if(EnNmPNckGy == true){EnNmPNckGy = false;}
      if(NAQBAxdHdU == true){NAQBAxdHdU = false;}
      if(GWMDIpYOXL == true){GWMDIpYOXL = false;}
      if(xiOqPTRqMN == true){xiOqPTRqMN = false;}
      if(iMNzdjIoJa == true){iMNzdjIoJa = false;}
      if(tpVZGgaKdY == true){tpVZGgaKdY = false;}
      if(oOMuBdkZlg == true){oOMuBdkZlg = false;}
      if(fltQktJMZY == true){fltQktJMZY = false;}
      if(gjFxQHNojh == true){gjFxQHNojh = false;}
      if(rGNnYUzRyz == true){rGNnYUzRyz = false;}
      if(ziDbtIuFnM == true){ziDbtIuFnM = false;}
      if(WOkFqEIPes == true){WOkFqEIPes = false;}
      if(QwoDnYYGqQ == true){QwoDnYYGqQ = false;}
      if(tqNZYxgcCI == true){tqNZYxgcCI = false;}
      if(LIBXKdVHwT == true){LIBXKdVHwT = false;}
      if(LjmEhDtznV == true){LjmEhDtznV = false;}
      if(ioNWSfAKfs == true){ioNWSfAKfs = false;}
      if(WfNVhItRrK == true){WfNVhItRrK = false;}
      if(DQKWEJDJqV == true){DQKWEJDJqV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ORFVUOZHLQ
{ 
  void uwSeizCHfD()
  { 
      bool hyDbBuGeGj = false;
      bool YaFIIIAHwB = false;
      bool QgMpUIOsxk = false;
      bool TUqVKBMIeT = false;
      bool SynEREqtjJ = false;
      bool PEoRFBlMHr = false;
      bool fMhVQEiiyH = false;
      bool uqcchSsHYP = false;
      bool QUImUOeZPJ = false;
      bool PzAlmGPSnM = false;
      bool VIucOXaLim = false;
      bool IdnrbLEtAB = false;
      bool ZITorupqpb = false;
      bool CaTLtIYeYx = false;
      bool nSqbBlLIJb = false;
      bool CURlmMnGkS = false;
      bool gjVGjhJcxS = false;
      bool lYVDNGRqmF = false;
      bool jggNZgIhXg = false;
      bool TStGPExjqT = false;
      string fIYxQDsjGJ;
      string JjYMSiwaFk;
      string isULkqBJhj;
      string HzrfVaXDTK;
      string ViVKHciyxV;
      string SGWEsmmMMY;
      string CjBWCJrrqq;
      string LrHuOzWKEH;
      string HCLMQAcmmh;
      string mKptHlhzIx;
      string jpHGKZzbos;
      string QcIFRAXoFB;
      string gEFJDBBNmc;
      string RWBGWQtFCp;
      string BAytQKBkcG;
      string NrtGQfkxCI;
      string YknLatbChA;
      string FRlOAfGKrs;
      string AQkfOYctoq;
      string exDsRPMlSW;
      if(fIYxQDsjGJ == jpHGKZzbos){hyDbBuGeGj = true;}
      else if(jpHGKZzbos == fIYxQDsjGJ){VIucOXaLim = true;}
      if(JjYMSiwaFk == QcIFRAXoFB){YaFIIIAHwB = true;}
      else if(QcIFRAXoFB == JjYMSiwaFk){IdnrbLEtAB = true;}
      if(isULkqBJhj == gEFJDBBNmc){QgMpUIOsxk = true;}
      else if(gEFJDBBNmc == isULkqBJhj){ZITorupqpb = true;}
      if(HzrfVaXDTK == RWBGWQtFCp){TUqVKBMIeT = true;}
      else if(RWBGWQtFCp == HzrfVaXDTK){CaTLtIYeYx = true;}
      if(ViVKHciyxV == BAytQKBkcG){SynEREqtjJ = true;}
      else if(BAytQKBkcG == ViVKHciyxV){nSqbBlLIJb = true;}
      if(SGWEsmmMMY == NrtGQfkxCI){PEoRFBlMHr = true;}
      else if(NrtGQfkxCI == SGWEsmmMMY){CURlmMnGkS = true;}
      if(CjBWCJrrqq == YknLatbChA){fMhVQEiiyH = true;}
      else if(YknLatbChA == CjBWCJrrqq){gjVGjhJcxS = true;}
      if(LrHuOzWKEH == FRlOAfGKrs){uqcchSsHYP = true;}
      if(HCLMQAcmmh == AQkfOYctoq){QUImUOeZPJ = true;}
      if(mKptHlhzIx == exDsRPMlSW){PzAlmGPSnM = true;}
      while(FRlOAfGKrs == LrHuOzWKEH){lYVDNGRqmF = true;}
      while(AQkfOYctoq == AQkfOYctoq){jggNZgIhXg = true;}
      while(exDsRPMlSW == exDsRPMlSW){TStGPExjqT = true;}
      if(hyDbBuGeGj == true){hyDbBuGeGj = false;}
      if(YaFIIIAHwB == true){YaFIIIAHwB = false;}
      if(QgMpUIOsxk == true){QgMpUIOsxk = false;}
      if(TUqVKBMIeT == true){TUqVKBMIeT = false;}
      if(SynEREqtjJ == true){SynEREqtjJ = false;}
      if(PEoRFBlMHr == true){PEoRFBlMHr = false;}
      if(fMhVQEiiyH == true){fMhVQEiiyH = false;}
      if(uqcchSsHYP == true){uqcchSsHYP = false;}
      if(QUImUOeZPJ == true){QUImUOeZPJ = false;}
      if(PzAlmGPSnM == true){PzAlmGPSnM = false;}
      if(VIucOXaLim == true){VIucOXaLim = false;}
      if(IdnrbLEtAB == true){IdnrbLEtAB = false;}
      if(ZITorupqpb == true){ZITorupqpb = false;}
      if(CaTLtIYeYx == true){CaTLtIYeYx = false;}
      if(nSqbBlLIJb == true){nSqbBlLIJb = false;}
      if(CURlmMnGkS == true){CURlmMnGkS = false;}
      if(gjVGjhJcxS == true){gjVGjhJcxS = false;}
      if(lYVDNGRqmF == true){lYVDNGRqmF = false;}
      if(jggNZgIhXg == true){jggNZgIhXg = false;}
      if(TStGPExjqT == true){TStGPExjqT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XLUSNYYBZA
{ 
  void ySrQyCPtGB()
  { 
      bool rsobKAawee = false;
      bool YCtjUaNzXC = false;
      bool ORTPflrAVQ = false;
      bool ICKSLLrgbj = false;
      bool oSDFpXCVsx = false;
      bool XKSWOaYYuX = false;
      bool IaZNVVraXJ = false;
      bool ifXnNuNNZI = false;
      bool jVGXsxjmEj = false;
      bool lSlDaOrpjU = false;
      bool faxjNrmeJS = false;
      bool NKzmmlTCTX = false;
      bool lEWnWQtTMz = false;
      bool DXdEcIYVJB = false;
      bool KEckWaSzQo = false;
      bool toVLhaEJMw = false;
      bool xSRKPofuJd = false;
      bool MxrSMgbkQw = false;
      bool yuDXENCSVB = false;
      bool EPJMDqlRVg = false;
      string RIJpUTuuLK;
      string woSjjCYlMq;
      string DMjdjbQVdp;
      string cWGesePmtS;
      string QInakrBjpO;
      string wGKQRFXeFr;
      string CYRVhZhnpE;
      string lkrtZuXIsr;
      string ZgfPsVLoNn;
      string HjhBPqqSsL;
      string KNskbLNQqV;
      string hOlMSVrBbL;
      string NraCwCCqei;
      string EEPhgMmtmh;
      string IuyqFrxUhs;
      string yophxRlpoY;
      string IWkHRczEeG;
      string iyOtDSsjwE;
      string EgIljPOhjP;
      string ZwFOYlJpZc;
      if(RIJpUTuuLK == KNskbLNQqV){rsobKAawee = true;}
      else if(KNskbLNQqV == RIJpUTuuLK){faxjNrmeJS = true;}
      if(woSjjCYlMq == hOlMSVrBbL){YCtjUaNzXC = true;}
      else if(hOlMSVrBbL == woSjjCYlMq){NKzmmlTCTX = true;}
      if(DMjdjbQVdp == NraCwCCqei){ORTPflrAVQ = true;}
      else if(NraCwCCqei == DMjdjbQVdp){lEWnWQtTMz = true;}
      if(cWGesePmtS == EEPhgMmtmh){ICKSLLrgbj = true;}
      else if(EEPhgMmtmh == cWGesePmtS){DXdEcIYVJB = true;}
      if(QInakrBjpO == IuyqFrxUhs){oSDFpXCVsx = true;}
      else if(IuyqFrxUhs == QInakrBjpO){KEckWaSzQo = true;}
      if(wGKQRFXeFr == yophxRlpoY){XKSWOaYYuX = true;}
      else if(yophxRlpoY == wGKQRFXeFr){toVLhaEJMw = true;}
      if(CYRVhZhnpE == IWkHRczEeG){IaZNVVraXJ = true;}
      else if(IWkHRczEeG == CYRVhZhnpE){xSRKPofuJd = true;}
      if(lkrtZuXIsr == iyOtDSsjwE){ifXnNuNNZI = true;}
      if(ZgfPsVLoNn == EgIljPOhjP){jVGXsxjmEj = true;}
      if(HjhBPqqSsL == ZwFOYlJpZc){lSlDaOrpjU = true;}
      while(iyOtDSsjwE == lkrtZuXIsr){MxrSMgbkQw = true;}
      while(EgIljPOhjP == EgIljPOhjP){yuDXENCSVB = true;}
      while(ZwFOYlJpZc == ZwFOYlJpZc){EPJMDqlRVg = true;}
      if(rsobKAawee == true){rsobKAawee = false;}
      if(YCtjUaNzXC == true){YCtjUaNzXC = false;}
      if(ORTPflrAVQ == true){ORTPflrAVQ = false;}
      if(ICKSLLrgbj == true){ICKSLLrgbj = false;}
      if(oSDFpXCVsx == true){oSDFpXCVsx = false;}
      if(XKSWOaYYuX == true){XKSWOaYYuX = false;}
      if(IaZNVVraXJ == true){IaZNVVraXJ = false;}
      if(ifXnNuNNZI == true){ifXnNuNNZI = false;}
      if(jVGXsxjmEj == true){jVGXsxjmEj = false;}
      if(lSlDaOrpjU == true){lSlDaOrpjU = false;}
      if(faxjNrmeJS == true){faxjNrmeJS = false;}
      if(NKzmmlTCTX == true){NKzmmlTCTX = false;}
      if(lEWnWQtTMz == true){lEWnWQtTMz = false;}
      if(DXdEcIYVJB == true){DXdEcIYVJB = false;}
      if(KEckWaSzQo == true){KEckWaSzQo = false;}
      if(toVLhaEJMw == true){toVLhaEJMw = false;}
      if(xSRKPofuJd == true){xSRKPofuJd = false;}
      if(MxrSMgbkQw == true){MxrSMgbkQw = false;}
      if(yuDXENCSVB == true){yuDXENCSVB = false;}
      if(EPJMDqlRVg == true){EPJMDqlRVg = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LZDSRJGBAY
{ 
  void fgXukfqqJm()
  { 
      bool RRyorrOIsr = false;
      bool mKwnWUkDfd = false;
      bool ZDnhUPJqoy = false;
      bool ZWWjdGbNSi = false;
      bool duEulkCWwx = false;
      bool ImbPROdPUc = false;
      bool hWSuRPmMJQ = false;
      bool sXjxzdyswi = false;
      bool NluroluHSP = false;
      bool bMrpAGcJcV = false;
      bool ypNfEjDdPn = false;
      bool PkTkOubaFf = false;
      bool Uestnffswr = false;
      bool BoPZkXYPmE = false;
      bool iQrezqsGbF = false;
      bool eBDxBSwSrX = false;
      bool EJhWlaUsAq = false;
      bool WMmAwDmOtw = false;
      bool mYRqNzkSXR = false;
      bool bFWOzYhUrX = false;
      string YDdXsVbdRo;
      string zYDBKJuLqD;
      string TNTZTbmQcD;
      string flUPgoXJKd;
      string mBCVuQwqVA;
      string NzwKIKiEDR;
      string iKBNINEnTV;
      string llBNPSKxeD;
      string euubpgoJGV;
      string FTFrtbzoNh;
      string LIXANbxiup;
      string AlYzlmPYWa;
      string XtgyPSfISm;
      string SVjxqWXuOO;
      string iGOQVTtFFH;
      string MZRlJRaYju;
      string uoPHWdBbHP;
      string OUKIrReQbw;
      string YZDLasEymZ;
      string uzKSStqHsq;
      if(YDdXsVbdRo == LIXANbxiup){RRyorrOIsr = true;}
      else if(LIXANbxiup == YDdXsVbdRo){ypNfEjDdPn = true;}
      if(zYDBKJuLqD == AlYzlmPYWa){mKwnWUkDfd = true;}
      else if(AlYzlmPYWa == zYDBKJuLqD){PkTkOubaFf = true;}
      if(TNTZTbmQcD == XtgyPSfISm){ZDnhUPJqoy = true;}
      else if(XtgyPSfISm == TNTZTbmQcD){Uestnffswr = true;}
      if(flUPgoXJKd == SVjxqWXuOO){ZWWjdGbNSi = true;}
      else if(SVjxqWXuOO == flUPgoXJKd){BoPZkXYPmE = true;}
      if(mBCVuQwqVA == iGOQVTtFFH){duEulkCWwx = true;}
      else if(iGOQVTtFFH == mBCVuQwqVA){iQrezqsGbF = true;}
      if(NzwKIKiEDR == MZRlJRaYju){ImbPROdPUc = true;}
      else if(MZRlJRaYju == NzwKIKiEDR){eBDxBSwSrX = true;}
      if(iKBNINEnTV == uoPHWdBbHP){hWSuRPmMJQ = true;}
      else if(uoPHWdBbHP == iKBNINEnTV){EJhWlaUsAq = true;}
      if(llBNPSKxeD == OUKIrReQbw){sXjxzdyswi = true;}
      if(euubpgoJGV == YZDLasEymZ){NluroluHSP = true;}
      if(FTFrtbzoNh == uzKSStqHsq){bMrpAGcJcV = true;}
      while(OUKIrReQbw == llBNPSKxeD){WMmAwDmOtw = true;}
      while(YZDLasEymZ == YZDLasEymZ){mYRqNzkSXR = true;}
      while(uzKSStqHsq == uzKSStqHsq){bFWOzYhUrX = true;}
      if(RRyorrOIsr == true){RRyorrOIsr = false;}
      if(mKwnWUkDfd == true){mKwnWUkDfd = false;}
      if(ZDnhUPJqoy == true){ZDnhUPJqoy = false;}
      if(ZWWjdGbNSi == true){ZWWjdGbNSi = false;}
      if(duEulkCWwx == true){duEulkCWwx = false;}
      if(ImbPROdPUc == true){ImbPROdPUc = false;}
      if(hWSuRPmMJQ == true){hWSuRPmMJQ = false;}
      if(sXjxzdyswi == true){sXjxzdyswi = false;}
      if(NluroluHSP == true){NluroluHSP = false;}
      if(bMrpAGcJcV == true){bMrpAGcJcV = false;}
      if(ypNfEjDdPn == true){ypNfEjDdPn = false;}
      if(PkTkOubaFf == true){PkTkOubaFf = false;}
      if(Uestnffswr == true){Uestnffswr = false;}
      if(BoPZkXYPmE == true){BoPZkXYPmE = false;}
      if(iQrezqsGbF == true){iQrezqsGbF = false;}
      if(eBDxBSwSrX == true){eBDxBSwSrX = false;}
      if(EJhWlaUsAq == true){EJhWlaUsAq = false;}
      if(WMmAwDmOtw == true){WMmAwDmOtw = false;}
      if(mYRqNzkSXR == true){mYRqNzkSXR = false;}
      if(bFWOzYhUrX == true){bFWOzYhUrX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XRWAVWOOQZ
{ 
  void rDkBszAAHK()
  { 
      bool pdxOkYafwo = false;
      bool PiKbeUhgin = false;
      bool sKOWHaxLNN = false;
      bool XweqlYobTr = false;
      bool IxrYExWAOx = false;
      bool CSHoIfOZHJ = false;
      bool bPRiRTEToG = false;
      bool eFNqfwRpSK = false;
      bool EwwQLEnoiG = false;
      bool JPUJQrfJHk = false;
      bool rCLIRxFQJL = false;
      bool OYHwFeTAXM = false;
      bool ePkiwPjEZm = false;
      bool PHhhSBkmyN = false;
      bool zXfWqkHXDk = false;
      bool YeltFbRcMB = false;
      bool MApVuGpZdD = false;
      bool tfBJNGjNqK = false;
      bool IMAoOkUmbp = false;
      bool jIawBygJaj = false;
      string KxRtecrhjt;
      string jtLDspsUew;
      string qqZLclbbhT;
      string IzDIDyTRJp;
      string WNimJYHUUO;
      string KfZrRTWGRr;
      string BPAPkjpCAu;
      string WbLnYcHmBE;
      string dGHTJcYLqj;
      string aMAQoHNSCY;
      string hEUZqiTkSn;
      string LjFMNtHgGH;
      string LeLQTJatzR;
      string PojbVZkhbZ;
      string JmrCXpHnDt;
      string zfLtNCsDFh;
      string HYlBXWzcpn;
      string IZFfyYmJrc;
      string CmqERicoNT;
      string fhctSjAqdB;
      if(KxRtecrhjt == hEUZqiTkSn){pdxOkYafwo = true;}
      else if(hEUZqiTkSn == KxRtecrhjt){rCLIRxFQJL = true;}
      if(jtLDspsUew == LjFMNtHgGH){PiKbeUhgin = true;}
      else if(LjFMNtHgGH == jtLDspsUew){OYHwFeTAXM = true;}
      if(qqZLclbbhT == LeLQTJatzR){sKOWHaxLNN = true;}
      else if(LeLQTJatzR == qqZLclbbhT){ePkiwPjEZm = true;}
      if(IzDIDyTRJp == PojbVZkhbZ){XweqlYobTr = true;}
      else if(PojbVZkhbZ == IzDIDyTRJp){PHhhSBkmyN = true;}
      if(WNimJYHUUO == JmrCXpHnDt){IxrYExWAOx = true;}
      else if(JmrCXpHnDt == WNimJYHUUO){zXfWqkHXDk = true;}
      if(KfZrRTWGRr == zfLtNCsDFh){CSHoIfOZHJ = true;}
      else if(zfLtNCsDFh == KfZrRTWGRr){YeltFbRcMB = true;}
      if(BPAPkjpCAu == HYlBXWzcpn){bPRiRTEToG = true;}
      else if(HYlBXWzcpn == BPAPkjpCAu){MApVuGpZdD = true;}
      if(WbLnYcHmBE == IZFfyYmJrc){eFNqfwRpSK = true;}
      if(dGHTJcYLqj == CmqERicoNT){EwwQLEnoiG = true;}
      if(aMAQoHNSCY == fhctSjAqdB){JPUJQrfJHk = true;}
      while(IZFfyYmJrc == WbLnYcHmBE){tfBJNGjNqK = true;}
      while(CmqERicoNT == CmqERicoNT){IMAoOkUmbp = true;}
      while(fhctSjAqdB == fhctSjAqdB){jIawBygJaj = true;}
      if(pdxOkYafwo == true){pdxOkYafwo = false;}
      if(PiKbeUhgin == true){PiKbeUhgin = false;}
      if(sKOWHaxLNN == true){sKOWHaxLNN = false;}
      if(XweqlYobTr == true){XweqlYobTr = false;}
      if(IxrYExWAOx == true){IxrYExWAOx = false;}
      if(CSHoIfOZHJ == true){CSHoIfOZHJ = false;}
      if(bPRiRTEToG == true){bPRiRTEToG = false;}
      if(eFNqfwRpSK == true){eFNqfwRpSK = false;}
      if(EwwQLEnoiG == true){EwwQLEnoiG = false;}
      if(JPUJQrfJHk == true){JPUJQrfJHk = false;}
      if(rCLIRxFQJL == true){rCLIRxFQJL = false;}
      if(OYHwFeTAXM == true){OYHwFeTAXM = false;}
      if(ePkiwPjEZm == true){ePkiwPjEZm = false;}
      if(PHhhSBkmyN == true){PHhhSBkmyN = false;}
      if(zXfWqkHXDk == true){zXfWqkHXDk = false;}
      if(YeltFbRcMB == true){YeltFbRcMB = false;}
      if(MApVuGpZdD == true){MApVuGpZdD = false;}
      if(tfBJNGjNqK == true){tfBJNGjNqK = false;}
      if(IMAoOkUmbp == true){IMAoOkUmbp = false;}
      if(jIawBygJaj == true){jIawBygJaj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SYFDOJICNB
{ 
  void BjUMhZiEzz()
  { 
      bool NaKmRHTUtV = false;
      bool DwlbYwVaQa = false;
      bool zJpHokijqf = false;
      bool qHiyuEeynm = false;
      bool lrKeLThrjI = false;
      bool UGerVEjCxg = false;
      bool YjdOewMnfT = false;
      bool RXmdnGyLwV = false;
      bool CpAhLjbpbq = false;
      bool UbAHOCOwzO = false;
      bool GtMPpwSBDO = false;
      bool FXFbdhiuDt = false;
      bool TVLozoXfOl = false;
      bool rCMgOHdjqO = false;
      bool RamMlwVXcg = false;
      bool STsEujhXQB = false;
      bool YllRWDLYNV = false;
      bool EzDncriyDD = false;
      bool SzDhSXIHld = false;
      bool YGTppenUph = false;
      string QysbSmxpxR;
      string WEtoQcEtuj;
      string XjBZtdYIUH;
      string MLSmJMYHCQ;
      string JmxquEKocG;
      string FcmrkfwShn;
      string OoUpELlGfD;
      string DwrdakmJeo;
      string TieOBjXZCc;
      string QGZNCURGli;
      string SDIYCJkIRA;
      string smnrUeJFoU;
      string nLEDmxWdbF;
      string SfWwVPZGMr;
      string yOIBidSgrW;
      string KMRRuHzFkE;
      string lHgLhNPqWF;
      string FDrxWYDCTJ;
      string eokFtGNFAO;
      string PTUIFphwTV;
      if(QysbSmxpxR == SDIYCJkIRA){NaKmRHTUtV = true;}
      else if(SDIYCJkIRA == QysbSmxpxR){GtMPpwSBDO = true;}
      if(WEtoQcEtuj == smnrUeJFoU){DwlbYwVaQa = true;}
      else if(smnrUeJFoU == WEtoQcEtuj){FXFbdhiuDt = true;}
      if(XjBZtdYIUH == nLEDmxWdbF){zJpHokijqf = true;}
      else if(nLEDmxWdbF == XjBZtdYIUH){TVLozoXfOl = true;}
      if(MLSmJMYHCQ == SfWwVPZGMr){qHiyuEeynm = true;}
      else if(SfWwVPZGMr == MLSmJMYHCQ){rCMgOHdjqO = true;}
      if(JmxquEKocG == yOIBidSgrW){lrKeLThrjI = true;}
      else if(yOIBidSgrW == JmxquEKocG){RamMlwVXcg = true;}
      if(FcmrkfwShn == KMRRuHzFkE){UGerVEjCxg = true;}
      else if(KMRRuHzFkE == FcmrkfwShn){STsEujhXQB = true;}
      if(OoUpELlGfD == lHgLhNPqWF){YjdOewMnfT = true;}
      else if(lHgLhNPqWF == OoUpELlGfD){YllRWDLYNV = true;}
      if(DwrdakmJeo == FDrxWYDCTJ){RXmdnGyLwV = true;}
      if(TieOBjXZCc == eokFtGNFAO){CpAhLjbpbq = true;}
      if(QGZNCURGli == PTUIFphwTV){UbAHOCOwzO = true;}
      while(FDrxWYDCTJ == DwrdakmJeo){EzDncriyDD = true;}
      while(eokFtGNFAO == eokFtGNFAO){SzDhSXIHld = true;}
      while(PTUIFphwTV == PTUIFphwTV){YGTppenUph = true;}
      if(NaKmRHTUtV == true){NaKmRHTUtV = false;}
      if(DwlbYwVaQa == true){DwlbYwVaQa = false;}
      if(zJpHokijqf == true){zJpHokijqf = false;}
      if(qHiyuEeynm == true){qHiyuEeynm = false;}
      if(lrKeLThrjI == true){lrKeLThrjI = false;}
      if(UGerVEjCxg == true){UGerVEjCxg = false;}
      if(YjdOewMnfT == true){YjdOewMnfT = false;}
      if(RXmdnGyLwV == true){RXmdnGyLwV = false;}
      if(CpAhLjbpbq == true){CpAhLjbpbq = false;}
      if(UbAHOCOwzO == true){UbAHOCOwzO = false;}
      if(GtMPpwSBDO == true){GtMPpwSBDO = false;}
      if(FXFbdhiuDt == true){FXFbdhiuDt = false;}
      if(TVLozoXfOl == true){TVLozoXfOl = false;}
      if(rCMgOHdjqO == true){rCMgOHdjqO = false;}
      if(RamMlwVXcg == true){RamMlwVXcg = false;}
      if(STsEujhXQB == true){STsEujhXQB = false;}
      if(YllRWDLYNV == true){YllRWDLYNV = false;}
      if(EzDncriyDD == true){EzDncriyDD = false;}
      if(SzDhSXIHld == true){SzDhSXIHld = false;}
      if(YGTppenUph == true){YGTppenUph = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XXEBYSQUSQ
{ 
  void cfFkRBELaF()
  { 
      bool MDcKUgaSbZ = false;
      bool DelLQFEHxo = false;
      bool LbSVoUuQgK = false;
      bool cWUZQhweWS = false;
      bool EkRutkKYFA = false;
      bool IiPskkkGUz = false;
      bool kJkQxgJLth = false;
      bool IYjdjZKWbE = false;
      bool JWLgBtGbnf = false;
      bool xpKWgtzAxN = false;
      bool aydWqjylJL = false;
      bool aNePpOyEOD = false;
      bool ZhXOjSONwo = false;
      bool ZJYOdKrnoR = false;
      bool uUScEjLtgS = false;
      bool uultmsaPlY = false;
      bool VkoTkRnQle = false;
      bool VKOxIFPuJA = false;
      bool GTfdseXeak = false;
      bool jbBQwKfyqm = false;
      string ApKsxeUchg;
      string HVUazqKsQe;
      string HizpQDKnMU;
      string phIxEnQhPc;
      string zorYZwNNMG;
      string feQofQpUbt;
      string TgQMqYNXWK;
      string ZccmeRpaPU;
      string owaFnKRTQm;
      string ywFVJSYegA;
      string hpqBfDCcJN;
      string aPEHEWFLYg;
      string HtGiMdXHdM;
      string sKsPlzFZYk;
      string IoJDMhrxQl;
      string MNGAsniIoC;
      string euywUPVnUF;
      string RflmXSjDWd;
      string lZpqzfEsVy;
      string PwjVFNpMhw;
      if(ApKsxeUchg == hpqBfDCcJN){MDcKUgaSbZ = true;}
      else if(hpqBfDCcJN == ApKsxeUchg){aydWqjylJL = true;}
      if(HVUazqKsQe == aPEHEWFLYg){DelLQFEHxo = true;}
      else if(aPEHEWFLYg == HVUazqKsQe){aNePpOyEOD = true;}
      if(HizpQDKnMU == HtGiMdXHdM){LbSVoUuQgK = true;}
      else if(HtGiMdXHdM == HizpQDKnMU){ZhXOjSONwo = true;}
      if(phIxEnQhPc == sKsPlzFZYk){cWUZQhweWS = true;}
      else if(sKsPlzFZYk == phIxEnQhPc){ZJYOdKrnoR = true;}
      if(zorYZwNNMG == IoJDMhrxQl){EkRutkKYFA = true;}
      else if(IoJDMhrxQl == zorYZwNNMG){uUScEjLtgS = true;}
      if(feQofQpUbt == MNGAsniIoC){IiPskkkGUz = true;}
      else if(MNGAsniIoC == feQofQpUbt){uultmsaPlY = true;}
      if(TgQMqYNXWK == euywUPVnUF){kJkQxgJLth = true;}
      else if(euywUPVnUF == TgQMqYNXWK){VkoTkRnQle = true;}
      if(ZccmeRpaPU == RflmXSjDWd){IYjdjZKWbE = true;}
      if(owaFnKRTQm == lZpqzfEsVy){JWLgBtGbnf = true;}
      if(ywFVJSYegA == PwjVFNpMhw){xpKWgtzAxN = true;}
      while(RflmXSjDWd == ZccmeRpaPU){VKOxIFPuJA = true;}
      while(lZpqzfEsVy == lZpqzfEsVy){GTfdseXeak = true;}
      while(PwjVFNpMhw == PwjVFNpMhw){jbBQwKfyqm = true;}
      if(MDcKUgaSbZ == true){MDcKUgaSbZ = false;}
      if(DelLQFEHxo == true){DelLQFEHxo = false;}
      if(LbSVoUuQgK == true){LbSVoUuQgK = false;}
      if(cWUZQhweWS == true){cWUZQhweWS = false;}
      if(EkRutkKYFA == true){EkRutkKYFA = false;}
      if(IiPskkkGUz == true){IiPskkkGUz = false;}
      if(kJkQxgJLth == true){kJkQxgJLth = false;}
      if(IYjdjZKWbE == true){IYjdjZKWbE = false;}
      if(JWLgBtGbnf == true){JWLgBtGbnf = false;}
      if(xpKWgtzAxN == true){xpKWgtzAxN = false;}
      if(aydWqjylJL == true){aydWqjylJL = false;}
      if(aNePpOyEOD == true){aNePpOyEOD = false;}
      if(ZhXOjSONwo == true){ZhXOjSONwo = false;}
      if(ZJYOdKrnoR == true){ZJYOdKrnoR = false;}
      if(uUScEjLtgS == true){uUScEjLtgS = false;}
      if(uultmsaPlY == true){uultmsaPlY = false;}
      if(VkoTkRnQle == true){VkoTkRnQle = false;}
      if(VKOxIFPuJA == true){VKOxIFPuJA = false;}
      if(GTfdseXeak == true){GTfdseXeak = false;}
      if(jbBQwKfyqm == true){jbBQwKfyqm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MOOYYNIWEX
{ 
  void pGNHTaQbAH()
  { 
      bool UrDTnCdVqq = false;
      bool HludxuZLEh = false;
      bool NnzDoGXVLG = false;
      bool erKfcgMNiY = false;
      bool fMSWBjroDi = false;
      bool lMeYjfsAeh = false;
      bool eHJkLOaOBP = false;
      bool hLbfCBUcBE = false;
      bool NhScTSMkww = false;
      bool nLhBtYChLl = false;
      bool IdnmAZjzWi = false;
      bool LQzaTwngQr = false;
      bool BpSFbWcZkc = false;
      bool ymQOuitfct = false;
      bool hNNNjXMHgy = false;
      bool jUQSjSLiLd = false;
      bool xJOsjYzjfk = false;
      bool BIEWKncEUE = false;
      bool JVrmscVyec = false;
      bool XLUyxXyTcL = false;
      string MVTzWwSCoN;
      string tzeWdVEeLV;
      string SlkgUnYnic;
      string njiJLYNBAN;
      string ONCUQupLyJ;
      string uzmMHZjlei;
      string WdTMZJQULF;
      string HCexHEjQPY;
      string xNoZgnxVCk;
      string nbVRqjtpuH;
      string eVzsCrUoEC;
      string pTVGlIotek;
      string LtRZQefNFX;
      string KsUIsaNwWo;
      string SHRzuTFPeh;
      string uAKJujEZbW;
      string LHigrwzMML;
      string trWoREcRex;
      string ZQhXbJhKNJ;
      string WnTAdLYjHy;
      if(MVTzWwSCoN == eVzsCrUoEC){UrDTnCdVqq = true;}
      else if(eVzsCrUoEC == MVTzWwSCoN){IdnmAZjzWi = true;}
      if(tzeWdVEeLV == pTVGlIotek){HludxuZLEh = true;}
      else if(pTVGlIotek == tzeWdVEeLV){LQzaTwngQr = true;}
      if(SlkgUnYnic == LtRZQefNFX){NnzDoGXVLG = true;}
      else if(LtRZQefNFX == SlkgUnYnic){BpSFbWcZkc = true;}
      if(njiJLYNBAN == KsUIsaNwWo){erKfcgMNiY = true;}
      else if(KsUIsaNwWo == njiJLYNBAN){ymQOuitfct = true;}
      if(ONCUQupLyJ == SHRzuTFPeh){fMSWBjroDi = true;}
      else if(SHRzuTFPeh == ONCUQupLyJ){hNNNjXMHgy = true;}
      if(uzmMHZjlei == uAKJujEZbW){lMeYjfsAeh = true;}
      else if(uAKJujEZbW == uzmMHZjlei){jUQSjSLiLd = true;}
      if(WdTMZJQULF == LHigrwzMML){eHJkLOaOBP = true;}
      else if(LHigrwzMML == WdTMZJQULF){xJOsjYzjfk = true;}
      if(HCexHEjQPY == trWoREcRex){hLbfCBUcBE = true;}
      if(xNoZgnxVCk == ZQhXbJhKNJ){NhScTSMkww = true;}
      if(nbVRqjtpuH == WnTAdLYjHy){nLhBtYChLl = true;}
      while(trWoREcRex == HCexHEjQPY){BIEWKncEUE = true;}
      while(ZQhXbJhKNJ == ZQhXbJhKNJ){JVrmscVyec = true;}
      while(WnTAdLYjHy == WnTAdLYjHy){XLUyxXyTcL = true;}
      if(UrDTnCdVqq == true){UrDTnCdVqq = false;}
      if(HludxuZLEh == true){HludxuZLEh = false;}
      if(NnzDoGXVLG == true){NnzDoGXVLG = false;}
      if(erKfcgMNiY == true){erKfcgMNiY = false;}
      if(fMSWBjroDi == true){fMSWBjroDi = false;}
      if(lMeYjfsAeh == true){lMeYjfsAeh = false;}
      if(eHJkLOaOBP == true){eHJkLOaOBP = false;}
      if(hLbfCBUcBE == true){hLbfCBUcBE = false;}
      if(NhScTSMkww == true){NhScTSMkww = false;}
      if(nLhBtYChLl == true){nLhBtYChLl = false;}
      if(IdnmAZjzWi == true){IdnmAZjzWi = false;}
      if(LQzaTwngQr == true){LQzaTwngQr = false;}
      if(BpSFbWcZkc == true){BpSFbWcZkc = false;}
      if(ymQOuitfct == true){ymQOuitfct = false;}
      if(hNNNjXMHgy == true){hNNNjXMHgy = false;}
      if(jUQSjSLiLd == true){jUQSjSLiLd = false;}
      if(xJOsjYzjfk == true){xJOsjYzjfk = false;}
      if(BIEWKncEUE == true){BIEWKncEUE = false;}
      if(JVrmscVyec == true){JVrmscVyec = false;}
      if(XLUyxXyTcL == true){XLUyxXyTcL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CMBYLLGIRP
{ 
  void UbVsgRrwMc()
  { 
      bool GbdkxizROa = false;
      bool rVgfImARCL = false;
      bool dPutIVWGto = false;
      bool HMpXIAZkGJ = false;
      bool JCjooBZxid = false;
      bool HVZqeqgjbb = false;
      bool MrXYSeeIWH = false;
      bool MPLsCbfGEG = false;
      bool wYXPhllzsR = false;
      bool TpIdxQdKVJ = false;
      bool WQZaQqJfoC = false;
      bool LZfUfmUHqg = false;
      bool sfPFmWGgwl = false;
      bool xUjtjezwyp = false;
      bool bGKpGsKmxJ = false;
      bool uVSnQpAUmt = false;
      bool kbiCEppkjA = false;
      bool mYIBFlVYzt = false;
      bool MSilpaCwct = false;
      bool eeIEEzWHkP = false;
      string JJyLFjrqBc;
      string WKgellEtuX;
      string YUqsLSOMgV;
      string QKCPpJmwWs;
      string nmbwjCzqHA;
      string KeBHOnREzm;
      string UeKeLLIRob;
      string yHWCjkstJJ;
      string TWFfLlIuAB;
      string riwjXIcwkY;
      string tgCfBCIzWc;
      string tXupSxZdTZ;
      string DNRnjPKldb;
      string IwJjOmUYdI;
      string cIXXGUmGQZ;
      string izcKUgPOul;
      string qVVuUyNKbf;
      string bAzKwtosgP;
      string cEVDrITyPg;
      string XGcrQHmcDI;
      if(JJyLFjrqBc == tgCfBCIzWc){GbdkxizROa = true;}
      else if(tgCfBCIzWc == JJyLFjrqBc){WQZaQqJfoC = true;}
      if(WKgellEtuX == tXupSxZdTZ){rVgfImARCL = true;}
      else if(tXupSxZdTZ == WKgellEtuX){LZfUfmUHqg = true;}
      if(YUqsLSOMgV == DNRnjPKldb){dPutIVWGto = true;}
      else if(DNRnjPKldb == YUqsLSOMgV){sfPFmWGgwl = true;}
      if(QKCPpJmwWs == IwJjOmUYdI){HMpXIAZkGJ = true;}
      else if(IwJjOmUYdI == QKCPpJmwWs){xUjtjezwyp = true;}
      if(nmbwjCzqHA == cIXXGUmGQZ){JCjooBZxid = true;}
      else if(cIXXGUmGQZ == nmbwjCzqHA){bGKpGsKmxJ = true;}
      if(KeBHOnREzm == izcKUgPOul){HVZqeqgjbb = true;}
      else if(izcKUgPOul == KeBHOnREzm){uVSnQpAUmt = true;}
      if(UeKeLLIRob == qVVuUyNKbf){MrXYSeeIWH = true;}
      else if(qVVuUyNKbf == UeKeLLIRob){kbiCEppkjA = true;}
      if(yHWCjkstJJ == bAzKwtosgP){MPLsCbfGEG = true;}
      if(TWFfLlIuAB == cEVDrITyPg){wYXPhllzsR = true;}
      if(riwjXIcwkY == XGcrQHmcDI){TpIdxQdKVJ = true;}
      while(bAzKwtosgP == yHWCjkstJJ){mYIBFlVYzt = true;}
      while(cEVDrITyPg == cEVDrITyPg){MSilpaCwct = true;}
      while(XGcrQHmcDI == XGcrQHmcDI){eeIEEzWHkP = true;}
      if(GbdkxizROa == true){GbdkxizROa = false;}
      if(rVgfImARCL == true){rVgfImARCL = false;}
      if(dPutIVWGto == true){dPutIVWGto = false;}
      if(HMpXIAZkGJ == true){HMpXIAZkGJ = false;}
      if(JCjooBZxid == true){JCjooBZxid = false;}
      if(HVZqeqgjbb == true){HVZqeqgjbb = false;}
      if(MrXYSeeIWH == true){MrXYSeeIWH = false;}
      if(MPLsCbfGEG == true){MPLsCbfGEG = false;}
      if(wYXPhllzsR == true){wYXPhllzsR = false;}
      if(TpIdxQdKVJ == true){TpIdxQdKVJ = false;}
      if(WQZaQqJfoC == true){WQZaQqJfoC = false;}
      if(LZfUfmUHqg == true){LZfUfmUHqg = false;}
      if(sfPFmWGgwl == true){sfPFmWGgwl = false;}
      if(xUjtjezwyp == true){xUjtjezwyp = false;}
      if(bGKpGsKmxJ == true){bGKpGsKmxJ = false;}
      if(uVSnQpAUmt == true){uVSnQpAUmt = false;}
      if(kbiCEppkjA == true){kbiCEppkjA = false;}
      if(mYIBFlVYzt == true){mYIBFlVYzt = false;}
      if(MSilpaCwct == true){MSilpaCwct = false;}
      if(eeIEEzWHkP == true){eeIEEzWHkP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GVSBGEQVHV
{ 
  void RPlcHpYlqF()
  { 
      bool QwhuGAsJHb = false;
      bool ziyegXyWHf = false;
      bool QEGGynCAHO = false;
      bool BkZkNlKwbW = false;
      bool KTkmsFMWqE = false;
      bool CYnopLpaxS = false;
      bool xsBrZOINQt = false;
      bool MBujVxSTNr = false;
      bool aeIDMbxjuC = false;
      bool wjzJYQSrMe = false;
      bool wzGxrAdDTs = false;
      bool rVNEqSjyfy = false;
      bool QXSOxcCkLE = false;
      bool daHiubYLLL = false;
      bool ytQBJaVtRG = false;
      bool gUMONYgrzO = false;
      bool yrqOOROeJP = false;
      bool sXmTzWGQFW = false;
      bool HITFXJOsUW = false;
      bool tgMwNFCClq = false;
      string yNHZtkOaUF;
      string rVgzPeZDFs;
      string FiyjmqqlFG;
      string mtiTjrAspE;
      string wCmzVZrBYx;
      string WdwCIOOXlD;
      string scVNRxmMPW;
      string efMAwCwzNN;
      string ypMHjJxfLU;
      string dcGmKTgYgY;
      string mflBSpQwGF;
      string SPKYFDPOdI;
      string zGBtrBZZpb;
      string VXRnZSDKos;
      string rBZFBNSghG;
      string EStYZgdqJQ;
      string QqFdjtxVzl;
      string AEYkQxgurd;
      string irriWDcsOs;
      string nLBYcBwuVH;
      if(yNHZtkOaUF == mflBSpQwGF){QwhuGAsJHb = true;}
      else if(mflBSpQwGF == yNHZtkOaUF){wzGxrAdDTs = true;}
      if(rVgzPeZDFs == SPKYFDPOdI){ziyegXyWHf = true;}
      else if(SPKYFDPOdI == rVgzPeZDFs){rVNEqSjyfy = true;}
      if(FiyjmqqlFG == zGBtrBZZpb){QEGGynCAHO = true;}
      else if(zGBtrBZZpb == FiyjmqqlFG){QXSOxcCkLE = true;}
      if(mtiTjrAspE == VXRnZSDKos){BkZkNlKwbW = true;}
      else if(VXRnZSDKos == mtiTjrAspE){daHiubYLLL = true;}
      if(wCmzVZrBYx == rBZFBNSghG){KTkmsFMWqE = true;}
      else if(rBZFBNSghG == wCmzVZrBYx){ytQBJaVtRG = true;}
      if(WdwCIOOXlD == EStYZgdqJQ){CYnopLpaxS = true;}
      else if(EStYZgdqJQ == WdwCIOOXlD){gUMONYgrzO = true;}
      if(scVNRxmMPW == QqFdjtxVzl){xsBrZOINQt = true;}
      else if(QqFdjtxVzl == scVNRxmMPW){yrqOOROeJP = true;}
      if(efMAwCwzNN == AEYkQxgurd){MBujVxSTNr = true;}
      if(ypMHjJxfLU == irriWDcsOs){aeIDMbxjuC = true;}
      if(dcGmKTgYgY == nLBYcBwuVH){wjzJYQSrMe = true;}
      while(AEYkQxgurd == efMAwCwzNN){sXmTzWGQFW = true;}
      while(irriWDcsOs == irriWDcsOs){HITFXJOsUW = true;}
      while(nLBYcBwuVH == nLBYcBwuVH){tgMwNFCClq = true;}
      if(QwhuGAsJHb == true){QwhuGAsJHb = false;}
      if(ziyegXyWHf == true){ziyegXyWHf = false;}
      if(QEGGynCAHO == true){QEGGynCAHO = false;}
      if(BkZkNlKwbW == true){BkZkNlKwbW = false;}
      if(KTkmsFMWqE == true){KTkmsFMWqE = false;}
      if(CYnopLpaxS == true){CYnopLpaxS = false;}
      if(xsBrZOINQt == true){xsBrZOINQt = false;}
      if(MBujVxSTNr == true){MBujVxSTNr = false;}
      if(aeIDMbxjuC == true){aeIDMbxjuC = false;}
      if(wjzJYQSrMe == true){wjzJYQSrMe = false;}
      if(wzGxrAdDTs == true){wzGxrAdDTs = false;}
      if(rVNEqSjyfy == true){rVNEqSjyfy = false;}
      if(QXSOxcCkLE == true){QXSOxcCkLE = false;}
      if(daHiubYLLL == true){daHiubYLLL = false;}
      if(ytQBJaVtRG == true){ytQBJaVtRG = false;}
      if(gUMONYgrzO == true){gUMONYgrzO = false;}
      if(yrqOOROeJP == true){yrqOOROeJP = false;}
      if(sXmTzWGQFW == true){sXmTzWGQFW = false;}
      if(HITFXJOsUW == true){HITFXJOsUW = false;}
      if(tgMwNFCClq == true){tgMwNFCClq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KTKQIFKCPK
{ 
  void HkWLCxfHUz()
  { 
      bool lWecjYYrYU = false;
      bool HdErMZGktQ = false;
      bool kwQRZOKdpQ = false;
      bool oVSeSOXoEA = false;
      bool fbUBuXhZRS = false;
      bool oIFFfpFNDU = false;
      bool NuCHelLYYx = false;
      bool BQftkIaOVB = false;
      bool gkMWbgPMzc = false;
      bool JKljmjZNWI = false;
      bool MIBURXcLUW = false;
      bool PpqCmciefx = false;
      bool VyZumqewGA = false;
      bool iNrMnYirED = false;
      bool EEqbNSSjwJ = false;
      bool BthYlOiFga = false;
      bool rfETnByeNO = false;
      bool sqZbsRGxhU = false;
      bool XuAwynfSZZ = false;
      bool pdLdKXAXnL = false;
      string icOacKwipb;
      string xnyqukJsTG;
      string XIKRYJQHel;
      string gXhNzPTEVT;
      string jUiNiYlLLK;
      string kSBSLaSwCT;
      string soDEuZFjqf;
      string REXAWCZxiA;
      string WFwUAOVPBX;
      string xWBBZuQdRI;
      string SBTfrftBuI;
      string BUNBxnsiDg;
      string EohShuDWab;
      string lueagUlPbU;
      string TUTsWbJzEl;
      string CgakEcSAxx;
      string cXZTVFomUb;
      string BcHhMjVowF;
      string kDRoZFUsVD;
      string ClRdrutzWQ;
      if(icOacKwipb == SBTfrftBuI){lWecjYYrYU = true;}
      else if(SBTfrftBuI == icOacKwipb){MIBURXcLUW = true;}
      if(xnyqukJsTG == BUNBxnsiDg){HdErMZGktQ = true;}
      else if(BUNBxnsiDg == xnyqukJsTG){PpqCmciefx = true;}
      if(XIKRYJQHel == EohShuDWab){kwQRZOKdpQ = true;}
      else if(EohShuDWab == XIKRYJQHel){VyZumqewGA = true;}
      if(gXhNzPTEVT == lueagUlPbU){oVSeSOXoEA = true;}
      else if(lueagUlPbU == gXhNzPTEVT){iNrMnYirED = true;}
      if(jUiNiYlLLK == TUTsWbJzEl){fbUBuXhZRS = true;}
      else if(TUTsWbJzEl == jUiNiYlLLK){EEqbNSSjwJ = true;}
      if(kSBSLaSwCT == CgakEcSAxx){oIFFfpFNDU = true;}
      else if(CgakEcSAxx == kSBSLaSwCT){BthYlOiFga = true;}
      if(soDEuZFjqf == cXZTVFomUb){NuCHelLYYx = true;}
      else if(cXZTVFomUb == soDEuZFjqf){rfETnByeNO = true;}
      if(REXAWCZxiA == BcHhMjVowF){BQftkIaOVB = true;}
      if(WFwUAOVPBX == kDRoZFUsVD){gkMWbgPMzc = true;}
      if(xWBBZuQdRI == ClRdrutzWQ){JKljmjZNWI = true;}
      while(BcHhMjVowF == REXAWCZxiA){sqZbsRGxhU = true;}
      while(kDRoZFUsVD == kDRoZFUsVD){XuAwynfSZZ = true;}
      while(ClRdrutzWQ == ClRdrutzWQ){pdLdKXAXnL = true;}
      if(lWecjYYrYU == true){lWecjYYrYU = false;}
      if(HdErMZGktQ == true){HdErMZGktQ = false;}
      if(kwQRZOKdpQ == true){kwQRZOKdpQ = false;}
      if(oVSeSOXoEA == true){oVSeSOXoEA = false;}
      if(fbUBuXhZRS == true){fbUBuXhZRS = false;}
      if(oIFFfpFNDU == true){oIFFfpFNDU = false;}
      if(NuCHelLYYx == true){NuCHelLYYx = false;}
      if(BQftkIaOVB == true){BQftkIaOVB = false;}
      if(gkMWbgPMzc == true){gkMWbgPMzc = false;}
      if(JKljmjZNWI == true){JKljmjZNWI = false;}
      if(MIBURXcLUW == true){MIBURXcLUW = false;}
      if(PpqCmciefx == true){PpqCmciefx = false;}
      if(VyZumqewGA == true){VyZumqewGA = false;}
      if(iNrMnYirED == true){iNrMnYirED = false;}
      if(EEqbNSSjwJ == true){EEqbNSSjwJ = false;}
      if(BthYlOiFga == true){BthYlOiFga = false;}
      if(rfETnByeNO == true){rfETnByeNO = false;}
      if(sqZbsRGxhU == true){sqZbsRGxhU = false;}
      if(XuAwynfSZZ == true){XuAwynfSZZ = false;}
      if(pdLdKXAXnL == true){pdLdKXAXnL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HQPZLSMXPW
{ 
  void cNoaIlgWwA()
  { 
      bool PRHXsjtThA = false;
      bool lLBwOZGlkG = false;
      bool nYNVjYTpcV = false;
      bool JmpNmFZAae = false;
      bool mQbCXhHTrm = false;
      bool DuNHxqDlmT = false;
      bool PEqMqyVnIJ = false;
      bool AFKGSXwKxB = false;
      bool RzyoRNYhPw = false;
      bool BmMGOiYPLZ = false;
      bool LTLhlxkHoZ = false;
      bool tsokCQGBtd = false;
      bool YnXLHNsDqj = false;
      bool yfzXoxnPDT = false;
      bool bRspBIyaEy = false;
      bool bWUgAXuzaX = false;
      bool EgPEbOsQwF = false;
      bool bqyaBHugJz = false;
      bool PRzsIaKSWR = false;
      bool BooePuBnOm = false;
      string iIzfDwmPGc;
      string hhLKVleiDh;
      string RwxQXfdXmx;
      string QKOmSPgIQm;
      string hKesudwFGz;
      string yWShVlKiHX;
      string RPcJMXSYrG;
      string FVSQAGQEUs;
      string iYQmysqgRf;
      string EyBrUSgVgX;
      string KXNAtcEChO;
      string UGNNoszpLT;
      string flHLBHUZuh;
      string ZuXwhKKPsU;
      string nozCLyjmCa;
      string SqKNrYQBzl;
      string HdGznZiRee;
      string hiwpeqmEAD;
      string XgXUOhkKDJ;
      string tJPGNJhzOl;
      if(iIzfDwmPGc == KXNAtcEChO){PRHXsjtThA = true;}
      else if(KXNAtcEChO == iIzfDwmPGc){LTLhlxkHoZ = true;}
      if(hhLKVleiDh == UGNNoszpLT){lLBwOZGlkG = true;}
      else if(UGNNoszpLT == hhLKVleiDh){tsokCQGBtd = true;}
      if(RwxQXfdXmx == flHLBHUZuh){nYNVjYTpcV = true;}
      else if(flHLBHUZuh == RwxQXfdXmx){YnXLHNsDqj = true;}
      if(QKOmSPgIQm == ZuXwhKKPsU){JmpNmFZAae = true;}
      else if(ZuXwhKKPsU == QKOmSPgIQm){yfzXoxnPDT = true;}
      if(hKesudwFGz == nozCLyjmCa){mQbCXhHTrm = true;}
      else if(nozCLyjmCa == hKesudwFGz){bRspBIyaEy = true;}
      if(yWShVlKiHX == SqKNrYQBzl){DuNHxqDlmT = true;}
      else if(SqKNrYQBzl == yWShVlKiHX){bWUgAXuzaX = true;}
      if(RPcJMXSYrG == HdGznZiRee){PEqMqyVnIJ = true;}
      else if(HdGznZiRee == RPcJMXSYrG){EgPEbOsQwF = true;}
      if(FVSQAGQEUs == hiwpeqmEAD){AFKGSXwKxB = true;}
      if(iYQmysqgRf == XgXUOhkKDJ){RzyoRNYhPw = true;}
      if(EyBrUSgVgX == tJPGNJhzOl){BmMGOiYPLZ = true;}
      while(hiwpeqmEAD == FVSQAGQEUs){bqyaBHugJz = true;}
      while(XgXUOhkKDJ == XgXUOhkKDJ){PRzsIaKSWR = true;}
      while(tJPGNJhzOl == tJPGNJhzOl){BooePuBnOm = true;}
      if(PRHXsjtThA == true){PRHXsjtThA = false;}
      if(lLBwOZGlkG == true){lLBwOZGlkG = false;}
      if(nYNVjYTpcV == true){nYNVjYTpcV = false;}
      if(JmpNmFZAae == true){JmpNmFZAae = false;}
      if(mQbCXhHTrm == true){mQbCXhHTrm = false;}
      if(DuNHxqDlmT == true){DuNHxqDlmT = false;}
      if(PEqMqyVnIJ == true){PEqMqyVnIJ = false;}
      if(AFKGSXwKxB == true){AFKGSXwKxB = false;}
      if(RzyoRNYhPw == true){RzyoRNYhPw = false;}
      if(BmMGOiYPLZ == true){BmMGOiYPLZ = false;}
      if(LTLhlxkHoZ == true){LTLhlxkHoZ = false;}
      if(tsokCQGBtd == true){tsokCQGBtd = false;}
      if(YnXLHNsDqj == true){YnXLHNsDqj = false;}
      if(yfzXoxnPDT == true){yfzXoxnPDT = false;}
      if(bRspBIyaEy == true){bRspBIyaEy = false;}
      if(bWUgAXuzaX == true){bWUgAXuzaX = false;}
      if(EgPEbOsQwF == true){EgPEbOsQwF = false;}
      if(bqyaBHugJz == true){bqyaBHugJz = false;}
      if(PRzsIaKSWR == true){PRzsIaKSWR = false;}
      if(BooePuBnOm == true){BooePuBnOm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DFIKCNNYNK
{ 
  void cCnxhuCBdR()
  { 
      bool mspikDDPTo = false;
      bool gbDjTozoTV = false;
      bool eGSkJHpjzJ = false;
      bool jzSsXlDirA = false;
      bool cSlqqgRKIH = false;
      bool NnXSTfZFZc = false;
      bool MSPJEhJJMh = false;
      bool lPwSrMUbtb = false;
      bool oebquwbzYq = false;
      bool LyIOnKCclJ = false;
      bool WHyaVyqEaH = false;
      bool LxKxmgqYOI = false;
      bool FJkUVmGheh = false;
      bool JqpjEhgAUx = false;
      bool EWYQfNmKaG = false;
      bool SdNdDNWdFh = false;
      bool qJZsyTHasF = false;
      bool AqDRDhjToH = false;
      bool AwGXOfdLfq = false;
      bool LCulGuxfSL = false;
      string IxLalzxBhF;
      string gufmfGwkNz;
      string pPyeryoYQs;
      string mOIqOJIrBP;
      string ondKgQWYcT;
      string iVHbAsygim;
      string JXjnSRloYW;
      string yEixKBknya;
      string aQwwKQGgpU;
      string KaINGQEpqO;
      string dhzGVdlJRz;
      string IKiHLsbncB;
      string KFyELWuUqs;
      string xuwixsQpaf;
      string ewOqMArBCN;
      string oPMpDXjuty;
      string iRSXCMNzKS;
      string bkMwWOpHrK;
      string etSdEghGfY;
      string JUubxMdqnz;
      if(IxLalzxBhF == dhzGVdlJRz){mspikDDPTo = true;}
      else if(dhzGVdlJRz == IxLalzxBhF){WHyaVyqEaH = true;}
      if(gufmfGwkNz == IKiHLsbncB){gbDjTozoTV = true;}
      else if(IKiHLsbncB == gufmfGwkNz){LxKxmgqYOI = true;}
      if(pPyeryoYQs == KFyELWuUqs){eGSkJHpjzJ = true;}
      else if(KFyELWuUqs == pPyeryoYQs){FJkUVmGheh = true;}
      if(mOIqOJIrBP == xuwixsQpaf){jzSsXlDirA = true;}
      else if(xuwixsQpaf == mOIqOJIrBP){JqpjEhgAUx = true;}
      if(ondKgQWYcT == ewOqMArBCN){cSlqqgRKIH = true;}
      else if(ewOqMArBCN == ondKgQWYcT){EWYQfNmKaG = true;}
      if(iVHbAsygim == oPMpDXjuty){NnXSTfZFZc = true;}
      else if(oPMpDXjuty == iVHbAsygim){SdNdDNWdFh = true;}
      if(JXjnSRloYW == iRSXCMNzKS){MSPJEhJJMh = true;}
      else if(iRSXCMNzKS == JXjnSRloYW){qJZsyTHasF = true;}
      if(yEixKBknya == bkMwWOpHrK){lPwSrMUbtb = true;}
      if(aQwwKQGgpU == etSdEghGfY){oebquwbzYq = true;}
      if(KaINGQEpqO == JUubxMdqnz){LyIOnKCclJ = true;}
      while(bkMwWOpHrK == yEixKBknya){AqDRDhjToH = true;}
      while(etSdEghGfY == etSdEghGfY){AwGXOfdLfq = true;}
      while(JUubxMdqnz == JUubxMdqnz){LCulGuxfSL = true;}
      if(mspikDDPTo == true){mspikDDPTo = false;}
      if(gbDjTozoTV == true){gbDjTozoTV = false;}
      if(eGSkJHpjzJ == true){eGSkJHpjzJ = false;}
      if(jzSsXlDirA == true){jzSsXlDirA = false;}
      if(cSlqqgRKIH == true){cSlqqgRKIH = false;}
      if(NnXSTfZFZc == true){NnXSTfZFZc = false;}
      if(MSPJEhJJMh == true){MSPJEhJJMh = false;}
      if(lPwSrMUbtb == true){lPwSrMUbtb = false;}
      if(oebquwbzYq == true){oebquwbzYq = false;}
      if(LyIOnKCclJ == true){LyIOnKCclJ = false;}
      if(WHyaVyqEaH == true){WHyaVyqEaH = false;}
      if(LxKxmgqYOI == true){LxKxmgqYOI = false;}
      if(FJkUVmGheh == true){FJkUVmGheh = false;}
      if(JqpjEhgAUx == true){JqpjEhgAUx = false;}
      if(EWYQfNmKaG == true){EWYQfNmKaG = false;}
      if(SdNdDNWdFh == true){SdNdDNWdFh = false;}
      if(qJZsyTHasF == true){qJZsyTHasF = false;}
      if(AqDRDhjToH == true){AqDRDhjToH = false;}
      if(AwGXOfdLfq == true){AwGXOfdLfq = false;}
      if(LCulGuxfSL == true){LCulGuxfSL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZDBMPUKIUX
{ 
  void gnRPeQBtAk()
  { 
      bool rcoWtQFGXF = false;
      bool FiAfnRxEzR = false;
      bool HFmfVeUQMl = false;
      bool gqKHsxcBXn = false;
      bool HEhdMIxewg = false;
      bool qyCzamoJAO = false;
      bool woskYOXrmp = false;
      bool owfKKCWJSN = false;
      bool hpTeuhnLfF = false;
      bool HTAJrRKwFy = false;
      bool jEqFruSroY = false;
      bool PfqmgFVOpp = false;
      bool ZMLgiUuWRr = false;
      bool raDTdcyNmA = false;
      bool PwVHobcNcl = false;
      bool wwGPYXrJWW = false;
      bool zuZwfobWJD = false;
      bool HHaoadpErH = false;
      bool rhtewuTnuF = false;
      bool knqPBTVYaz = false;
      string QIpPDfzqAE;
      string NnuYnwwPzF;
      string XcBSgysFCc;
      string dUZXjcWlFY;
      string zulKsOJXTO;
      string BokMamHXiF;
      string ltnXasOWoL;
      string ZJJaGbqpZS;
      string oVknboiNwx;
      string VyDAlEHOjz;
      string AlhIoqeKGz;
      string uQxPPPIAwk;
      string JAZrpaGInB;
      string UcFZRCmcyj;
      string KYHxBEAmjV;
      string wHRzpzLpRN;
      string kVVEeSLXyN;
      string bfBLajscpl;
      string ZSYRFMKGjS;
      string qxKkznDCRi;
      if(QIpPDfzqAE == AlhIoqeKGz){rcoWtQFGXF = true;}
      else if(AlhIoqeKGz == QIpPDfzqAE){jEqFruSroY = true;}
      if(NnuYnwwPzF == uQxPPPIAwk){FiAfnRxEzR = true;}
      else if(uQxPPPIAwk == NnuYnwwPzF){PfqmgFVOpp = true;}
      if(XcBSgysFCc == JAZrpaGInB){HFmfVeUQMl = true;}
      else if(JAZrpaGInB == XcBSgysFCc){ZMLgiUuWRr = true;}
      if(dUZXjcWlFY == UcFZRCmcyj){gqKHsxcBXn = true;}
      else if(UcFZRCmcyj == dUZXjcWlFY){raDTdcyNmA = true;}
      if(zulKsOJXTO == KYHxBEAmjV){HEhdMIxewg = true;}
      else if(KYHxBEAmjV == zulKsOJXTO){PwVHobcNcl = true;}
      if(BokMamHXiF == wHRzpzLpRN){qyCzamoJAO = true;}
      else if(wHRzpzLpRN == BokMamHXiF){wwGPYXrJWW = true;}
      if(ltnXasOWoL == kVVEeSLXyN){woskYOXrmp = true;}
      else if(kVVEeSLXyN == ltnXasOWoL){zuZwfobWJD = true;}
      if(ZJJaGbqpZS == bfBLajscpl){owfKKCWJSN = true;}
      if(oVknboiNwx == ZSYRFMKGjS){hpTeuhnLfF = true;}
      if(VyDAlEHOjz == qxKkznDCRi){HTAJrRKwFy = true;}
      while(bfBLajscpl == ZJJaGbqpZS){HHaoadpErH = true;}
      while(ZSYRFMKGjS == ZSYRFMKGjS){rhtewuTnuF = true;}
      while(qxKkznDCRi == qxKkznDCRi){knqPBTVYaz = true;}
      if(rcoWtQFGXF == true){rcoWtQFGXF = false;}
      if(FiAfnRxEzR == true){FiAfnRxEzR = false;}
      if(HFmfVeUQMl == true){HFmfVeUQMl = false;}
      if(gqKHsxcBXn == true){gqKHsxcBXn = false;}
      if(HEhdMIxewg == true){HEhdMIxewg = false;}
      if(qyCzamoJAO == true){qyCzamoJAO = false;}
      if(woskYOXrmp == true){woskYOXrmp = false;}
      if(owfKKCWJSN == true){owfKKCWJSN = false;}
      if(hpTeuhnLfF == true){hpTeuhnLfF = false;}
      if(HTAJrRKwFy == true){HTAJrRKwFy = false;}
      if(jEqFruSroY == true){jEqFruSroY = false;}
      if(PfqmgFVOpp == true){PfqmgFVOpp = false;}
      if(ZMLgiUuWRr == true){ZMLgiUuWRr = false;}
      if(raDTdcyNmA == true){raDTdcyNmA = false;}
      if(PwVHobcNcl == true){PwVHobcNcl = false;}
      if(wwGPYXrJWW == true){wwGPYXrJWW = false;}
      if(zuZwfobWJD == true){zuZwfobWJD = false;}
      if(HHaoadpErH == true){HHaoadpErH = false;}
      if(rhtewuTnuF == true){rhtewuTnuF = false;}
      if(knqPBTVYaz == true){knqPBTVYaz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ICYODFHCCC
{ 
  void NlfaiqlSjV()
  { 
      bool mxYmmhWxZP = false;
      bool VVClkhfOsm = false;
      bool XsleiKgqma = false;
      bool HyxgZaMSjs = false;
      bool xCPYJwgjIK = false;
      bool AyuylsKsDb = false;
      bool AeKyIXXVLK = false;
      bool UMdAoUdNUm = false;
      bool cwibZIDytg = false;
      bool PTOIrRrQQl = false;
      bool XLXxpnUuIQ = false;
      bool OxXQyEuhYz = false;
      bool UZaetBxVIX = false;
      bool cNwiPRXkll = false;
      bool nHocRRuyJm = false;
      bool VKqQRyjsDo = false;
      bool pymknAAoBg = false;
      bool fKwlLMjyMc = false;
      bool FzMUBSVfQR = false;
      bool gehwwoUEoQ = false;
      string QsRbDpUimP;
      string AahuSVipkF;
      string ROKohfWskt;
      string oqpglJAdiZ;
      string jtOhMnfaqa;
      string tnBBGBhbtX;
      string TaxraIjBIJ;
      string UFGKVXzcbs;
      string AuCJAAonaj;
      string aXjLFJwVHD;
      string EeRgGngpfh;
      string BFzAWXXnrZ;
      string TuaYiwUuEL;
      string bdVYoVcUDL;
      string SyIrhVXjnh;
      string kHdgjIDsyJ;
      string SlkTGXiaKB;
      string uDkfHUmhpM;
      string XkGWaTAonf;
      string RjVsrfMnkJ;
      if(QsRbDpUimP == EeRgGngpfh){mxYmmhWxZP = true;}
      else if(EeRgGngpfh == QsRbDpUimP){XLXxpnUuIQ = true;}
      if(AahuSVipkF == BFzAWXXnrZ){VVClkhfOsm = true;}
      else if(BFzAWXXnrZ == AahuSVipkF){OxXQyEuhYz = true;}
      if(ROKohfWskt == TuaYiwUuEL){XsleiKgqma = true;}
      else if(TuaYiwUuEL == ROKohfWskt){UZaetBxVIX = true;}
      if(oqpglJAdiZ == bdVYoVcUDL){HyxgZaMSjs = true;}
      else if(bdVYoVcUDL == oqpglJAdiZ){cNwiPRXkll = true;}
      if(jtOhMnfaqa == SyIrhVXjnh){xCPYJwgjIK = true;}
      else if(SyIrhVXjnh == jtOhMnfaqa){nHocRRuyJm = true;}
      if(tnBBGBhbtX == kHdgjIDsyJ){AyuylsKsDb = true;}
      else if(kHdgjIDsyJ == tnBBGBhbtX){VKqQRyjsDo = true;}
      if(TaxraIjBIJ == SlkTGXiaKB){AeKyIXXVLK = true;}
      else if(SlkTGXiaKB == TaxraIjBIJ){pymknAAoBg = true;}
      if(UFGKVXzcbs == uDkfHUmhpM){UMdAoUdNUm = true;}
      if(AuCJAAonaj == XkGWaTAonf){cwibZIDytg = true;}
      if(aXjLFJwVHD == RjVsrfMnkJ){PTOIrRrQQl = true;}
      while(uDkfHUmhpM == UFGKVXzcbs){fKwlLMjyMc = true;}
      while(XkGWaTAonf == XkGWaTAonf){FzMUBSVfQR = true;}
      while(RjVsrfMnkJ == RjVsrfMnkJ){gehwwoUEoQ = true;}
      if(mxYmmhWxZP == true){mxYmmhWxZP = false;}
      if(VVClkhfOsm == true){VVClkhfOsm = false;}
      if(XsleiKgqma == true){XsleiKgqma = false;}
      if(HyxgZaMSjs == true){HyxgZaMSjs = false;}
      if(xCPYJwgjIK == true){xCPYJwgjIK = false;}
      if(AyuylsKsDb == true){AyuylsKsDb = false;}
      if(AeKyIXXVLK == true){AeKyIXXVLK = false;}
      if(UMdAoUdNUm == true){UMdAoUdNUm = false;}
      if(cwibZIDytg == true){cwibZIDytg = false;}
      if(PTOIrRrQQl == true){PTOIrRrQQl = false;}
      if(XLXxpnUuIQ == true){XLXxpnUuIQ = false;}
      if(OxXQyEuhYz == true){OxXQyEuhYz = false;}
      if(UZaetBxVIX == true){UZaetBxVIX = false;}
      if(cNwiPRXkll == true){cNwiPRXkll = false;}
      if(nHocRRuyJm == true){nHocRRuyJm = false;}
      if(VKqQRyjsDo == true){VKqQRyjsDo = false;}
      if(pymknAAoBg == true){pymknAAoBg = false;}
      if(fKwlLMjyMc == true){fKwlLMjyMc = false;}
      if(FzMUBSVfQR == true){FzMUBSVfQR = false;}
      if(gehwwoUEoQ == true){gehwwoUEoQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SIQYFQIPHG
{ 
  void yRuAUVixxZ()
  { 
      bool nEFHmtuRcd = false;
      bool OuSOLHdfsr = false;
      bool DqUuuQnzth = false;
      bool RWtUrPWwGi = false;
      bool xiYzNExxER = false;
      bool jHMhpwiKMV = false;
      bool GwWuLrAyVu = false;
      bool jgZKuTSUgq = false;
      bool SptbxKJLdw = false;
      bool gMjjbAMdnV = false;
      bool DzaJgCgCmW = false;
      bool cNcUDPBtxB = false;
      bool hnTZNXxoht = false;
      bool HogKmSPjuu = false;
      bool cHXGSSiOgL = false;
      bool yRfmnOeBuJ = false;
      bool ErTsPngRhj = false;
      bool ARzCXZPMGu = false;
      bool oiIVupItWX = false;
      bool nNolpkmeqe = false;
      string tqUPPLNBET;
      string UeqnZhRFjD;
      string EyhpBrnopY;
      string rAxKkKtlkW;
      string bfuqXgdwXq;
      string WHzXLuJeOX;
      string WYKfbTgfCO;
      string meTMbXDpRU;
      string NrQzBjAZzp;
      string BPXMzCVPPD;
      string zxsJDqnmwm;
      string rTMsKeHKdq;
      string THgMNXHGNY;
      string CmZNUPyHwJ;
      string uNdLygCdCT;
      string uKlYjrQguf;
      string jhZWkEsyXt;
      string gAOTuGcVOf;
      string UcDsdrBBdc;
      string zQMPZNnGas;
      if(tqUPPLNBET == zxsJDqnmwm){nEFHmtuRcd = true;}
      else if(zxsJDqnmwm == tqUPPLNBET){DzaJgCgCmW = true;}
      if(UeqnZhRFjD == rTMsKeHKdq){OuSOLHdfsr = true;}
      else if(rTMsKeHKdq == UeqnZhRFjD){cNcUDPBtxB = true;}
      if(EyhpBrnopY == THgMNXHGNY){DqUuuQnzth = true;}
      else if(THgMNXHGNY == EyhpBrnopY){hnTZNXxoht = true;}
      if(rAxKkKtlkW == CmZNUPyHwJ){RWtUrPWwGi = true;}
      else if(CmZNUPyHwJ == rAxKkKtlkW){HogKmSPjuu = true;}
      if(bfuqXgdwXq == uNdLygCdCT){xiYzNExxER = true;}
      else if(uNdLygCdCT == bfuqXgdwXq){cHXGSSiOgL = true;}
      if(WHzXLuJeOX == uKlYjrQguf){jHMhpwiKMV = true;}
      else if(uKlYjrQguf == WHzXLuJeOX){yRfmnOeBuJ = true;}
      if(WYKfbTgfCO == jhZWkEsyXt){GwWuLrAyVu = true;}
      else if(jhZWkEsyXt == WYKfbTgfCO){ErTsPngRhj = true;}
      if(meTMbXDpRU == gAOTuGcVOf){jgZKuTSUgq = true;}
      if(NrQzBjAZzp == UcDsdrBBdc){SptbxKJLdw = true;}
      if(BPXMzCVPPD == zQMPZNnGas){gMjjbAMdnV = true;}
      while(gAOTuGcVOf == meTMbXDpRU){ARzCXZPMGu = true;}
      while(UcDsdrBBdc == UcDsdrBBdc){oiIVupItWX = true;}
      while(zQMPZNnGas == zQMPZNnGas){nNolpkmeqe = true;}
      if(nEFHmtuRcd == true){nEFHmtuRcd = false;}
      if(OuSOLHdfsr == true){OuSOLHdfsr = false;}
      if(DqUuuQnzth == true){DqUuuQnzth = false;}
      if(RWtUrPWwGi == true){RWtUrPWwGi = false;}
      if(xiYzNExxER == true){xiYzNExxER = false;}
      if(jHMhpwiKMV == true){jHMhpwiKMV = false;}
      if(GwWuLrAyVu == true){GwWuLrAyVu = false;}
      if(jgZKuTSUgq == true){jgZKuTSUgq = false;}
      if(SptbxKJLdw == true){SptbxKJLdw = false;}
      if(gMjjbAMdnV == true){gMjjbAMdnV = false;}
      if(DzaJgCgCmW == true){DzaJgCgCmW = false;}
      if(cNcUDPBtxB == true){cNcUDPBtxB = false;}
      if(hnTZNXxoht == true){hnTZNXxoht = false;}
      if(HogKmSPjuu == true){HogKmSPjuu = false;}
      if(cHXGSSiOgL == true){cHXGSSiOgL = false;}
      if(yRfmnOeBuJ == true){yRfmnOeBuJ = false;}
      if(ErTsPngRhj == true){ErTsPngRhj = false;}
      if(ARzCXZPMGu == true){ARzCXZPMGu = false;}
      if(oiIVupItWX == true){oiIVupItWX = false;}
      if(nNolpkmeqe == true){nNolpkmeqe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JVQHDLMPDL
{ 
  void dlVoqeRpfF()
  { 
      bool XiPNbcgXGj = false;
      bool SrGilIstVk = false;
      bool NNzTWPuuni = false;
      bool MWYRhCKaJY = false;
      bool otbNVnYbhu = false;
      bool mCSfWJiwqM = false;
      bool gkjWXwSoBi = false;
      bool yRkwuSklxi = false;
      bool qSNrwJDstY = false;
      bool SNjxrDKFpZ = false;
      bool UjazwUQVjD = false;
      bool YxyIUNXqaN = false;
      bool rZtcPjhnEr = false;
      bool IfuXNHqOwa = false;
      bool shJqzVPabd = false;
      bool fTebgtoyiL = false;
      bool lcQrtrBZtM = false;
      bool VJtzUKQCct = false;
      bool UHLaMoBTpT = false;
      bool RsejoPQQwH = false;
      string jBdYmePORP;
      string cNCceiItMH;
      string SlrMXNkkVL;
      string fWQsdfpEHz;
      string ZwDkmAEXQk;
      string nbDswyJQBZ;
      string terFiGwJyy;
      string sDmNhWhGZW;
      string IMUjXDSXwf;
      string WYDgEuEyBg;
      string DtZRrjnoKY;
      string PCqdQgKHge;
      string mYMRbTAFIm;
      string SjFrTNlCrn;
      string PcMXQUhZQW;
      string jwQVtehHpX;
      string iifXGfwyJC;
      string uHPsPJLoBG;
      string lZolCVSiyo;
      string feLTEfxoQj;
      if(jBdYmePORP == DtZRrjnoKY){XiPNbcgXGj = true;}
      else if(DtZRrjnoKY == jBdYmePORP){UjazwUQVjD = true;}
      if(cNCceiItMH == PCqdQgKHge){SrGilIstVk = true;}
      else if(PCqdQgKHge == cNCceiItMH){YxyIUNXqaN = true;}
      if(SlrMXNkkVL == mYMRbTAFIm){NNzTWPuuni = true;}
      else if(mYMRbTAFIm == SlrMXNkkVL){rZtcPjhnEr = true;}
      if(fWQsdfpEHz == SjFrTNlCrn){MWYRhCKaJY = true;}
      else if(SjFrTNlCrn == fWQsdfpEHz){IfuXNHqOwa = true;}
      if(ZwDkmAEXQk == PcMXQUhZQW){otbNVnYbhu = true;}
      else if(PcMXQUhZQW == ZwDkmAEXQk){shJqzVPabd = true;}
      if(nbDswyJQBZ == jwQVtehHpX){mCSfWJiwqM = true;}
      else if(jwQVtehHpX == nbDswyJQBZ){fTebgtoyiL = true;}
      if(terFiGwJyy == iifXGfwyJC){gkjWXwSoBi = true;}
      else if(iifXGfwyJC == terFiGwJyy){lcQrtrBZtM = true;}
      if(sDmNhWhGZW == uHPsPJLoBG){yRkwuSklxi = true;}
      if(IMUjXDSXwf == lZolCVSiyo){qSNrwJDstY = true;}
      if(WYDgEuEyBg == feLTEfxoQj){SNjxrDKFpZ = true;}
      while(uHPsPJLoBG == sDmNhWhGZW){VJtzUKQCct = true;}
      while(lZolCVSiyo == lZolCVSiyo){UHLaMoBTpT = true;}
      while(feLTEfxoQj == feLTEfxoQj){RsejoPQQwH = true;}
      if(XiPNbcgXGj == true){XiPNbcgXGj = false;}
      if(SrGilIstVk == true){SrGilIstVk = false;}
      if(NNzTWPuuni == true){NNzTWPuuni = false;}
      if(MWYRhCKaJY == true){MWYRhCKaJY = false;}
      if(otbNVnYbhu == true){otbNVnYbhu = false;}
      if(mCSfWJiwqM == true){mCSfWJiwqM = false;}
      if(gkjWXwSoBi == true){gkjWXwSoBi = false;}
      if(yRkwuSklxi == true){yRkwuSklxi = false;}
      if(qSNrwJDstY == true){qSNrwJDstY = false;}
      if(SNjxrDKFpZ == true){SNjxrDKFpZ = false;}
      if(UjazwUQVjD == true){UjazwUQVjD = false;}
      if(YxyIUNXqaN == true){YxyIUNXqaN = false;}
      if(rZtcPjhnEr == true){rZtcPjhnEr = false;}
      if(IfuXNHqOwa == true){IfuXNHqOwa = false;}
      if(shJqzVPabd == true){shJqzVPabd = false;}
      if(fTebgtoyiL == true){fTebgtoyiL = false;}
      if(lcQrtrBZtM == true){lcQrtrBZtM = false;}
      if(VJtzUKQCct == true){VJtzUKQCct = false;}
      if(UHLaMoBTpT == true){UHLaMoBTpT = false;}
      if(RsejoPQQwH == true){RsejoPQQwH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOOTNHJJPU
{ 
  void hxDDxwMqVu()
  { 
      bool JWqJtWPjQn = false;
      bool aDzjUCjZCh = false;
      bool EiFHPrsnKF = false;
      bool hqaizqlyCZ = false;
      bool hTpQppNUuF = false;
      bool KymnnaRjtB = false;
      bool dbZLFibQjc = false;
      bool QkWbAjsnRZ = false;
      bool dYrEtjEmtY = false;
      bool YCEBwGteLG = false;
      bool NxjRoYaKnD = false;
      bool gdRTXEiUQU = false;
      bool eoVLENgVft = false;
      bool pNcLLfoZPU = false;
      bool Jsyqqtjjdq = false;
      bool WYNpaYbXNa = false;
      bool rFesMSBGHq = false;
      bool PIFuLlpeUJ = false;
      bool dwrnWRzljm = false;
      bool EQMncEOFdX = false;
      string mXTQHeYcZa;
      string xnwrhoZaew;
      string wEeLOEpcwB;
      string deJrzIAQak;
      string NEkBOJGwbS;
      string jVOBpisHCC;
      string nwgYwReDrZ;
      string nIAsiqSdxE;
      string ysAcQIPHCP;
      string PrOXwQXLFw;
      string eCDCdaUURz;
      string POKyEuuDIn;
      string BWBbJwtbxS;
      string SHYAYuoxyo;
      string muoDAAhQTA;
      string UUDQuIBash;
      string grWzfHpKYJ;
      string GyESAGgybJ;
      string NdIxxyDHFQ;
      string sNEPItXoWr;
      if(mXTQHeYcZa == eCDCdaUURz){JWqJtWPjQn = true;}
      else if(eCDCdaUURz == mXTQHeYcZa){NxjRoYaKnD = true;}
      if(xnwrhoZaew == POKyEuuDIn){aDzjUCjZCh = true;}
      else if(POKyEuuDIn == xnwrhoZaew){gdRTXEiUQU = true;}
      if(wEeLOEpcwB == BWBbJwtbxS){EiFHPrsnKF = true;}
      else if(BWBbJwtbxS == wEeLOEpcwB){eoVLENgVft = true;}
      if(deJrzIAQak == SHYAYuoxyo){hqaizqlyCZ = true;}
      else if(SHYAYuoxyo == deJrzIAQak){pNcLLfoZPU = true;}
      if(NEkBOJGwbS == muoDAAhQTA){hTpQppNUuF = true;}
      else if(muoDAAhQTA == NEkBOJGwbS){Jsyqqtjjdq = true;}
      if(jVOBpisHCC == UUDQuIBash){KymnnaRjtB = true;}
      else if(UUDQuIBash == jVOBpisHCC){WYNpaYbXNa = true;}
      if(nwgYwReDrZ == grWzfHpKYJ){dbZLFibQjc = true;}
      else if(grWzfHpKYJ == nwgYwReDrZ){rFesMSBGHq = true;}
      if(nIAsiqSdxE == GyESAGgybJ){QkWbAjsnRZ = true;}
      if(ysAcQIPHCP == NdIxxyDHFQ){dYrEtjEmtY = true;}
      if(PrOXwQXLFw == sNEPItXoWr){YCEBwGteLG = true;}
      while(GyESAGgybJ == nIAsiqSdxE){PIFuLlpeUJ = true;}
      while(NdIxxyDHFQ == NdIxxyDHFQ){dwrnWRzljm = true;}
      while(sNEPItXoWr == sNEPItXoWr){EQMncEOFdX = true;}
      if(JWqJtWPjQn == true){JWqJtWPjQn = false;}
      if(aDzjUCjZCh == true){aDzjUCjZCh = false;}
      if(EiFHPrsnKF == true){EiFHPrsnKF = false;}
      if(hqaizqlyCZ == true){hqaizqlyCZ = false;}
      if(hTpQppNUuF == true){hTpQppNUuF = false;}
      if(KymnnaRjtB == true){KymnnaRjtB = false;}
      if(dbZLFibQjc == true){dbZLFibQjc = false;}
      if(QkWbAjsnRZ == true){QkWbAjsnRZ = false;}
      if(dYrEtjEmtY == true){dYrEtjEmtY = false;}
      if(YCEBwGteLG == true){YCEBwGteLG = false;}
      if(NxjRoYaKnD == true){NxjRoYaKnD = false;}
      if(gdRTXEiUQU == true){gdRTXEiUQU = false;}
      if(eoVLENgVft == true){eoVLENgVft = false;}
      if(pNcLLfoZPU == true){pNcLLfoZPU = false;}
      if(Jsyqqtjjdq == true){Jsyqqtjjdq = false;}
      if(WYNpaYbXNa == true){WYNpaYbXNa = false;}
      if(rFesMSBGHq == true){rFesMSBGHq = false;}
      if(PIFuLlpeUJ == true){PIFuLlpeUJ = false;}
      if(dwrnWRzljm == true){dwrnWRzljm = false;}
      if(EQMncEOFdX == true){EQMncEOFdX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOLQQSJJDO
{ 
  void IXPqbGXzYs()
  { 
      bool arlNZDiSLf = false;
      bool lkqRaNsHsL = false;
      bool dsUTLFGKSb = false;
      bool fXhrUygwhj = false;
      bool fIomSnnThs = false;
      bool zNUlEgWRCT = false;
      bool xXAizNDywb = false;
      bool fxZIeUEhAW = false;
      bool QlbYcSDumm = false;
      bool bOsNmIIFnF = false;
      bool jpWGMBmLGi = false;
      bool bnQeDKwaGf = false;
      bool JIKJAQEMUW = false;
      bool LytumaOVLb = false;
      bool fqnEKmxOJc = false;
      bool LhyHubeNFu = false;
      bool waqjWdtVrc = false;
      bool wZOeARTGyN = false;
      bool TpCqLdzlzK = false;
      bool wAQPeFAEgg = false;
      string iGJNDzhzJh;
      string QltmObnGOV;
      string NlBxdKmQsz;
      string HLtWKBixUE;
      string CBNXawuJpg;
      string CyrjExEMjr;
      string LQFJaFqXcM;
      string sqkKTMXdVh;
      string PdaMxJSKRN;
      string wWMbrkuExU;
      string EXfcmKMhPr;
      string YapCchpsMQ;
      string mgFOQWdIRi;
      string RTgBcqxyZm;
      string wktYnUfKzK;
      string uCrfhXRRMQ;
      string fDPgaFtnYk;
      string kqTYUBlfau;
      string pSJHnObATE;
      string ZfkIGsUNyh;
      if(iGJNDzhzJh == EXfcmKMhPr){arlNZDiSLf = true;}
      else if(EXfcmKMhPr == iGJNDzhzJh){jpWGMBmLGi = true;}
      if(QltmObnGOV == YapCchpsMQ){lkqRaNsHsL = true;}
      else if(YapCchpsMQ == QltmObnGOV){bnQeDKwaGf = true;}
      if(NlBxdKmQsz == mgFOQWdIRi){dsUTLFGKSb = true;}
      else if(mgFOQWdIRi == NlBxdKmQsz){JIKJAQEMUW = true;}
      if(HLtWKBixUE == RTgBcqxyZm){fXhrUygwhj = true;}
      else if(RTgBcqxyZm == HLtWKBixUE){LytumaOVLb = true;}
      if(CBNXawuJpg == wktYnUfKzK){fIomSnnThs = true;}
      else if(wktYnUfKzK == CBNXawuJpg){fqnEKmxOJc = true;}
      if(CyrjExEMjr == uCrfhXRRMQ){zNUlEgWRCT = true;}
      else if(uCrfhXRRMQ == CyrjExEMjr){LhyHubeNFu = true;}
      if(LQFJaFqXcM == fDPgaFtnYk){xXAizNDywb = true;}
      else if(fDPgaFtnYk == LQFJaFqXcM){waqjWdtVrc = true;}
      if(sqkKTMXdVh == kqTYUBlfau){fxZIeUEhAW = true;}
      if(PdaMxJSKRN == pSJHnObATE){QlbYcSDumm = true;}
      if(wWMbrkuExU == ZfkIGsUNyh){bOsNmIIFnF = true;}
      while(kqTYUBlfau == sqkKTMXdVh){wZOeARTGyN = true;}
      while(pSJHnObATE == pSJHnObATE){TpCqLdzlzK = true;}
      while(ZfkIGsUNyh == ZfkIGsUNyh){wAQPeFAEgg = true;}
      if(arlNZDiSLf == true){arlNZDiSLf = false;}
      if(lkqRaNsHsL == true){lkqRaNsHsL = false;}
      if(dsUTLFGKSb == true){dsUTLFGKSb = false;}
      if(fXhrUygwhj == true){fXhrUygwhj = false;}
      if(fIomSnnThs == true){fIomSnnThs = false;}
      if(zNUlEgWRCT == true){zNUlEgWRCT = false;}
      if(xXAizNDywb == true){xXAizNDywb = false;}
      if(fxZIeUEhAW == true){fxZIeUEhAW = false;}
      if(QlbYcSDumm == true){QlbYcSDumm = false;}
      if(bOsNmIIFnF == true){bOsNmIIFnF = false;}
      if(jpWGMBmLGi == true){jpWGMBmLGi = false;}
      if(bnQeDKwaGf == true){bnQeDKwaGf = false;}
      if(JIKJAQEMUW == true){JIKJAQEMUW = false;}
      if(LytumaOVLb == true){LytumaOVLb = false;}
      if(fqnEKmxOJc == true){fqnEKmxOJc = false;}
      if(LhyHubeNFu == true){LhyHubeNFu = false;}
      if(waqjWdtVrc == true){waqjWdtVrc = false;}
      if(wZOeARTGyN == true){wZOeARTGyN = false;}
      if(TpCqLdzlzK == true){TpCqLdzlzK = false;}
      if(wAQPeFAEgg == true){wAQPeFAEgg = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JAHKMRYZJJ
{ 
  void KsjCKqpDHG()
  { 
      bool nZcupHVKLU = false;
      bool nmKwpcRQEP = false;
      bool wMRdbCJglo = false;
      bool iRFCwAjsiI = false;
      bool NSUlzVdEZA = false;
      bool zKDVzaHyTm = false;
      bool ynYVkWgIzp = false;
      bool rXguCdrfeF = false;
      bool XFHloXNjDV = false;
      bool rkZwKZsyAc = false;
      bool nVEjHbdHSU = false;
      bool ibTNOsgmgN = false;
      bool IxFdyHykkb = false;
      bool jsfCRCAWdN = false;
      bool tGQhlPRLgR = false;
      bool sGRmUqQakA = false;
      bool dsWqjpwRwL = false;
      bool rJITSBfWqi = false;
      bool olDSidarga = false;
      bool lYldjAjZxw = false;
      string PLyKjDAApk;
      string fWKQGzWxbd;
      string IXYrYRhZwn;
      string EpiusNPAWu;
      string jEKcxFWDsY;
      string VDczGrmFlS;
      string VZEBfonPWs;
      string iWZfsENutS;
      string tegXSBNjnB;
      string ZJNEYKAJwH;
      string RGfzozcwFC;
      string erLSXqrfie;
      string bEbVSYIVGi;
      string DDylBmzghT;
      string YQdkWhyufh;
      string STZRfNeJgQ;
      string pHoVzasYRj;
      string rKyOGKZqyk;
      string ZNoYQidfGm;
      string IoAQcRbIkw;
      if(PLyKjDAApk == RGfzozcwFC){nZcupHVKLU = true;}
      else if(RGfzozcwFC == PLyKjDAApk){nVEjHbdHSU = true;}
      if(fWKQGzWxbd == erLSXqrfie){nmKwpcRQEP = true;}
      else if(erLSXqrfie == fWKQGzWxbd){ibTNOsgmgN = true;}
      if(IXYrYRhZwn == bEbVSYIVGi){wMRdbCJglo = true;}
      else if(bEbVSYIVGi == IXYrYRhZwn){IxFdyHykkb = true;}
      if(EpiusNPAWu == DDylBmzghT){iRFCwAjsiI = true;}
      else if(DDylBmzghT == EpiusNPAWu){jsfCRCAWdN = true;}
      if(jEKcxFWDsY == YQdkWhyufh){NSUlzVdEZA = true;}
      else if(YQdkWhyufh == jEKcxFWDsY){tGQhlPRLgR = true;}
      if(VDczGrmFlS == STZRfNeJgQ){zKDVzaHyTm = true;}
      else if(STZRfNeJgQ == VDczGrmFlS){sGRmUqQakA = true;}
      if(VZEBfonPWs == pHoVzasYRj){ynYVkWgIzp = true;}
      else if(pHoVzasYRj == VZEBfonPWs){dsWqjpwRwL = true;}
      if(iWZfsENutS == rKyOGKZqyk){rXguCdrfeF = true;}
      if(tegXSBNjnB == ZNoYQidfGm){XFHloXNjDV = true;}
      if(ZJNEYKAJwH == IoAQcRbIkw){rkZwKZsyAc = true;}
      while(rKyOGKZqyk == iWZfsENutS){rJITSBfWqi = true;}
      while(ZNoYQidfGm == ZNoYQidfGm){olDSidarga = true;}
      while(IoAQcRbIkw == IoAQcRbIkw){lYldjAjZxw = true;}
      if(nZcupHVKLU == true){nZcupHVKLU = false;}
      if(nmKwpcRQEP == true){nmKwpcRQEP = false;}
      if(wMRdbCJglo == true){wMRdbCJglo = false;}
      if(iRFCwAjsiI == true){iRFCwAjsiI = false;}
      if(NSUlzVdEZA == true){NSUlzVdEZA = false;}
      if(zKDVzaHyTm == true){zKDVzaHyTm = false;}
      if(ynYVkWgIzp == true){ynYVkWgIzp = false;}
      if(rXguCdrfeF == true){rXguCdrfeF = false;}
      if(XFHloXNjDV == true){XFHloXNjDV = false;}
      if(rkZwKZsyAc == true){rkZwKZsyAc = false;}
      if(nVEjHbdHSU == true){nVEjHbdHSU = false;}
      if(ibTNOsgmgN == true){ibTNOsgmgN = false;}
      if(IxFdyHykkb == true){IxFdyHykkb = false;}
      if(jsfCRCAWdN == true){jsfCRCAWdN = false;}
      if(tGQhlPRLgR == true){tGQhlPRLgR = false;}
      if(sGRmUqQakA == true){sGRmUqQakA = false;}
      if(dsWqjpwRwL == true){dsWqjpwRwL = false;}
      if(rJITSBfWqi == true){rJITSBfWqi = false;}
      if(olDSidarga == true){olDSidarga = false;}
      if(lYldjAjZxw == true){lYldjAjZxw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FBKJROXQOM
{ 
  void YfDlOlmOZb()
  { 
      bool sPlliAKOVf = false;
      bool PTeeUmpEkj = false;
      bool DbkWeLHFXg = false;
      bool rOywnZNqxr = false;
      bool xpCTwCtypc = false;
      bool MoazRQdOGX = false;
      bool rFqnnAVipt = false;
      bool YzHrlNZRab = false;
      bool RJOtUCmyeW = false;
      bool wzjZAryNeu = false;
      bool gLIwEMhiMD = false;
      bool qLndGiqxTk = false;
      bool NWDSxZPgrt = false;
      bool OMleuFASpH = false;
      bool TEkafkKFTQ = false;
      bool BBsshKHpyd = false;
      bool xApSodYIiA = false;
      bool hXHMQIkblM = false;
      bool OKNasMTrxj = false;
      bool yKTMKTNSlC = false;
      string XbmXNcUfAm;
      string cHoIhNcCkI;
      string UDLccFCPVw;
      string YotcQkPBWE;
      string RXzYYTWlky;
      string jQUxYUPJtI;
      string ZFJENmNMrx;
      string xbfgpxDmEI;
      string hQdWLHUwDu;
      string VGGOdMNuKk;
      string GKsnBIknGb;
      string RbEkZIalrF;
      string SQIwaqURCd;
      string jIbSCXWpVD;
      string VfobdsBUnU;
      string kaDESpBxMK;
      string mTlcrIzliB;
      string kkQnilagRh;
      string ozcgrIFXAL;
      string WZcLQCtMiU;
      if(XbmXNcUfAm == GKsnBIknGb){sPlliAKOVf = true;}
      else if(GKsnBIknGb == XbmXNcUfAm){gLIwEMhiMD = true;}
      if(cHoIhNcCkI == RbEkZIalrF){PTeeUmpEkj = true;}
      else if(RbEkZIalrF == cHoIhNcCkI){qLndGiqxTk = true;}
      if(UDLccFCPVw == SQIwaqURCd){DbkWeLHFXg = true;}
      else if(SQIwaqURCd == UDLccFCPVw){NWDSxZPgrt = true;}
      if(YotcQkPBWE == jIbSCXWpVD){rOywnZNqxr = true;}
      else if(jIbSCXWpVD == YotcQkPBWE){OMleuFASpH = true;}
      if(RXzYYTWlky == VfobdsBUnU){xpCTwCtypc = true;}
      else if(VfobdsBUnU == RXzYYTWlky){TEkafkKFTQ = true;}
      if(jQUxYUPJtI == kaDESpBxMK){MoazRQdOGX = true;}
      else if(kaDESpBxMK == jQUxYUPJtI){BBsshKHpyd = true;}
      if(ZFJENmNMrx == mTlcrIzliB){rFqnnAVipt = true;}
      else if(mTlcrIzliB == ZFJENmNMrx){xApSodYIiA = true;}
      if(xbfgpxDmEI == kkQnilagRh){YzHrlNZRab = true;}
      if(hQdWLHUwDu == ozcgrIFXAL){RJOtUCmyeW = true;}
      if(VGGOdMNuKk == WZcLQCtMiU){wzjZAryNeu = true;}
      while(kkQnilagRh == xbfgpxDmEI){hXHMQIkblM = true;}
      while(ozcgrIFXAL == ozcgrIFXAL){OKNasMTrxj = true;}
      while(WZcLQCtMiU == WZcLQCtMiU){yKTMKTNSlC = true;}
      if(sPlliAKOVf == true){sPlliAKOVf = false;}
      if(PTeeUmpEkj == true){PTeeUmpEkj = false;}
      if(DbkWeLHFXg == true){DbkWeLHFXg = false;}
      if(rOywnZNqxr == true){rOywnZNqxr = false;}
      if(xpCTwCtypc == true){xpCTwCtypc = false;}
      if(MoazRQdOGX == true){MoazRQdOGX = false;}
      if(rFqnnAVipt == true){rFqnnAVipt = false;}
      if(YzHrlNZRab == true){YzHrlNZRab = false;}
      if(RJOtUCmyeW == true){RJOtUCmyeW = false;}
      if(wzjZAryNeu == true){wzjZAryNeu = false;}
      if(gLIwEMhiMD == true){gLIwEMhiMD = false;}
      if(qLndGiqxTk == true){qLndGiqxTk = false;}
      if(NWDSxZPgrt == true){NWDSxZPgrt = false;}
      if(OMleuFASpH == true){OMleuFASpH = false;}
      if(TEkafkKFTQ == true){TEkafkKFTQ = false;}
      if(BBsshKHpyd == true){BBsshKHpyd = false;}
      if(xApSodYIiA == true){xApSodYIiA = false;}
      if(hXHMQIkblM == true){hXHMQIkblM = false;}
      if(OKNasMTrxj == true){OKNasMTrxj = false;}
      if(yKTMKTNSlC == true){yKTMKTNSlC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TDHUGNQOEG
{ 
  void ZGoAPmlTqR()
  { 
      bool uMNDJhWmUD = false;
      bool pnfiVItnIa = false;
      bool sxAtWdVlOW = false;
      bool QyGzBMDoPJ = false;
      bool yXaTEuMBJE = false;
      bool fRPsuyMSff = false;
      bool LjYymrgfgJ = false;
      bool FEXzcUVfpb = false;
      bool QTcJpyiNDD = false;
      bool zjIjxNycgM = false;
      bool NgUzyatQPZ = false;
      bool bfcEOqHOHn = false;
      bool ISbcJUwnBO = false;
      bool VszcNjzcKP = false;
      bool ESydbTHEUF = false;
      bool GjUrDtFqpd = false;
      bool eFOAEigzpe = false;
      bool reDrjpUyBp = false;
      bool NqSuIlXGYS = false;
      bool eHlgkyDxYs = false;
      string KiInjPAXtX;
      string NZaGlRnplL;
      string pchRYZRrBy;
      string JgaNDzzRYO;
      string UBYqzHTEGW;
      string RGrWsgApPK;
      string xIVuNgzMDl;
      string SWQXqnDqSQ;
      string FNkegDoDUV;
      string zICOQLfdcS;
      string IHATEtoUfE;
      string dIXyEjXqOK;
      string quqgmJUCGe;
      string IZOzIyMQZm;
      string NTXQrpkDJg;
      string nnORaMPTft;
      string ZmNAwDtXpG;
      string WGRbxWWanx;
      string YQrZqUKwXu;
      string irjeMZGXUn;
      if(KiInjPAXtX == IHATEtoUfE){uMNDJhWmUD = true;}
      else if(IHATEtoUfE == KiInjPAXtX){NgUzyatQPZ = true;}
      if(NZaGlRnplL == dIXyEjXqOK){pnfiVItnIa = true;}
      else if(dIXyEjXqOK == NZaGlRnplL){bfcEOqHOHn = true;}
      if(pchRYZRrBy == quqgmJUCGe){sxAtWdVlOW = true;}
      else if(quqgmJUCGe == pchRYZRrBy){ISbcJUwnBO = true;}
      if(JgaNDzzRYO == IZOzIyMQZm){QyGzBMDoPJ = true;}
      else if(IZOzIyMQZm == JgaNDzzRYO){VszcNjzcKP = true;}
      if(UBYqzHTEGW == NTXQrpkDJg){yXaTEuMBJE = true;}
      else if(NTXQrpkDJg == UBYqzHTEGW){ESydbTHEUF = true;}
      if(RGrWsgApPK == nnORaMPTft){fRPsuyMSff = true;}
      else if(nnORaMPTft == RGrWsgApPK){GjUrDtFqpd = true;}
      if(xIVuNgzMDl == ZmNAwDtXpG){LjYymrgfgJ = true;}
      else if(ZmNAwDtXpG == xIVuNgzMDl){eFOAEigzpe = true;}
      if(SWQXqnDqSQ == WGRbxWWanx){FEXzcUVfpb = true;}
      if(FNkegDoDUV == YQrZqUKwXu){QTcJpyiNDD = true;}
      if(zICOQLfdcS == irjeMZGXUn){zjIjxNycgM = true;}
      while(WGRbxWWanx == SWQXqnDqSQ){reDrjpUyBp = true;}
      while(YQrZqUKwXu == YQrZqUKwXu){NqSuIlXGYS = true;}
      while(irjeMZGXUn == irjeMZGXUn){eHlgkyDxYs = true;}
      if(uMNDJhWmUD == true){uMNDJhWmUD = false;}
      if(pnfiVItnIa == true){pnfiVItnIa = false;}
      if(sxAtWdVlOW == true){sxAtWdVlOW = false;}
      if(QyGzBMDoPJ == true){QyGzBMDoPJ = false;}
      if(yXaTEuMBJE == true){yXaTEuMBJE = false;}
      if(fRPsuyMSff == true){fRPsuyMSff = false;}
      if(LjYymrgfgJ == true){LjYymrgfgJ = false;}
      if(FEXzcUVfpb == true){FEXzcUVfpb = false;}
      if(QTcJpyiNDD == true){QTcJpyiNDD = false;}
      if(zjIjxNycgM == true){zjIjxNycgM = false;}
      if(NgUzyatQPZ == true){NgUzyatQPZ = false;}
      if(bfcEOqHOHn == true){bfcEOqHOHn = false;}
      if(ISbcJUwnBO == true){ISbcJUwnBO = false;}
      if(VszcNjzcKP == true){VszcNjzcKP = false;}
      if(ESydbTHEUF == true){ESydbTHEUF = false;}
      if(GjUrDtFqpd == true){GjUrDtFqpd = false;}
      if(eFOAEigzpe == true){eFOAEigzpe = false;}
      if(reDrjpUyBp == true){reDrjpUyBp = false;}
      if(NqSuIlXGYS == true){NqSuIlXGYS = false;}
      if(eHlgkyDxYs == true){eHlgkyDxYs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QRCITQGKCN
{ 
  void xYazGxYxVM()
  { 
      bool WjgrNTMeqo = false;
      bool iHiMudxgFd = false;
      bool SpDDAJJVaz = false;
      bool qnDbpcUHlR = false;
      bool oWyBoUxzqV = false;
      bool kGuCuggcrE = false;
      bool KpDqxtDGVR = false;
      bool JounowqBOe = false;
      bool LJeEOjrAgJ = false;
      bool wqOgGzSGEt = false;
      bool QpcZMuLGNz = false;
      bool ofOMrALycM = false;
      bool CCZLVRZNgo = false;
      bool ckwTOUfErT = false;
      bool YlmhUQcCoG = false;
      bool mTdrrNQREu = false;
      bool UyYXVEityW = false;
      bool TGGImQPrqE = false;
      bool nXYaCJGEbB = false;
      bool EOAwyaMPLQ = false;
      string izwYZKXWsI;
      string FsqPQxzGPH;
      string ZbzJzTDlWJ;
      string yyAYWzVGVJ;
      string iZAwcyJtIa;
      string kCrpEGzlZG;
      string HlziJHrSrQ;
      string arjGXzaCgB;
      string KJduYwGKTP;
      string zigPCxaQmV;
      string FpCqArRkRF;
      string fwFXuxHEJM;
      string AhybQqLUEa;
      string YJPLwTHlug;
      string eUrNjDdJTE;
      string oVHZUXrZZI;
      string fxgqgVftbb;
      string pVIbhfojqN;
      string NCAzkCCpqX;
      string bgLmAhnBgO;
      if(izwYZKXWsI == FpCqArRkRF){WjgrNTMeqo = true;}
      else if(FpCqArRkRF == izwYZKXWsI){QpcZMuLGNz = true;}
      if(FsqPQxzGPH == fwFXuxHEJM){iHiMudxgFd = true;}
      else if(fwFXuxHEJM == FsqPQxzGPH){ofOMrALycM = true;}
      if(ZbzJzTDlWJ == AhybQqLUEa){SpDDAJJVaz = true;}
      else if(AhybQqLUEa == ZbzJzTDlWJ){CCZLVRZNgo = true;}
      if(yyAYWzVGVJ == YJPLwTHlug){qnDbpcUHlR = true;}
      else if(YJPLwTHlug == yyAYWzVGVJ){ckwTOUfErT = true;}
      if(iZAwcyJtIa == eUrNjDdJTE){oWyBoUxzqV = true;}
      else if(eUrNjDdJTE == iZAwcyJtIa){YlmhUQcCoG = true;}
      if(kCrpEGzlZG == oVHZUXrZZI){kGuCuggcrE = true;}
      else if(oVHZUXrZZI == kCrpEGzlZG){mTdrrNQREu = true;}
      if(HlziJHrSrQ == fxgqgVftbb){KpDqxtDGVR = true;}
      else if(fxgqgVftbb == HlziJHrSrQ){UyYXVEityW = true;}
      if(arjGXzaCgB == pVIbhfojqN){JounowqBOe = true;}
      if(KJduYwGKTP == NCAzkCCpqX){LJeEOjrAgJ = true;}
      if(zigPCxaQmV == bgLmAhnBgO){wqOgGzSGEt = true;}
      while(pVIbhfojqN == arjGXzaCgB){TGGImQPrqE = true;}
      while(NCAzkCCpqX == NCAzkCCpqX){nXYaCJGEbB = true;}
      while(bgLmAhnBgO == bgLmAhnBgO){EOAwyaMPLQ = true;}
      if(WjgrNTMeqo == true){WjgrNTMeqo = false;}
      if(iHiMudxgFd == true){iHiMudxgFd = false;}
      if(SpDDAJJVaz == true){SpDDAJJVaz = false;}
      if(qnDbpcUHlR == true){qnDbpcUHlR = false;}
      if(oWyBoUxzqV == true){oWyBoUxzqV = false;}
      if(kGuCuggcrE == true){kGuCuggcrE = false;}
      if(KpDqxtDGVR == true){KpDqxtDGVR = false;}
      if(JounowqBOe == true){JounowqBOe = false;}
      if(LJeEOjrAgJ == true){LJeEOjrAgJ = false;}
      if(wqOgGzSGEt == true){wqOgGzSGEt = false;}
      if(QpcZMuLGNz == true){QpcZMuLGNz = false;}
      if(ofOMrALycM == true){ofOMrALycM = false;}
      if(CCZLVRZNgo == true){CCZLVRZNgo = false;}
      if(ckwTOUfErT == true){ckwTOUfErT = false;}
      if(YlmhUQcCoG == true){YlmhUQcCoG = false;}
      if(mTdrrNQREu == true){mTdrrNQREu = false;}
      if(UyYXVEityW == true){UyYXVEityW = false;}
      if(TGGImQPrqE == true){TGGImQPrqE = false;}
      if(nXYaCJGEbB == true){nXYaCJGEbB = false;}
      if(EOAwyaMPLQ == true){EOAwyaMPLQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IPRAEXYEHY
{ 
  void szjIUKWAco()
  { 
      bool RVMrhtJgfA = false;
      bool WDUIstjwmD = false;
      bool EwHbjoLHMP = false;
      bool AZVbHsmLpj = false;
      bool yBDahjTDEW = false;
      bool dQfDfUOjHr = false;
      bool hCTQAxgrAz = false;
      bool GYGDECsfsz = false;
      bool VICokitRwl = false;
      bool dCDyBuglak = false;
      bool DaCEPFKbyd = false;
      bool aipHVYzeeR = false;
      bool COwOVDUHBI = false;
      bool UHFIwuGyYf = false;
      bool XRTSKuLubb = false;
      bool wCGIhndniM = false;
      bool jCZpITdhLa = false;
      bool SBGysjudDZ = false;
      bool qZfYMSAwaH = false;
      bool QSOMwVrdKp = false;
      string BfbGOkPOOg;
      string aSiMiNHVDa;
      string UdSymZtARb;
      string qqpiZlKQwE;
      string wWlguWbFCg;
      string dgDrgWMdTo;
      string qOwmpjbzmx;
      string CpZydxrkCP;
      string xoykyJmyCC;
      string YgxGNhZzqL;
      string tcSFemGFNi;
      string eaaDqBQLnf;
      string wdwjXVsSjw;
      string WrtqVTWsGK;
      string ChkSWyFsPM;
      string ROKKOQfhgI;
      string snURLlfigX;
      string dWEddNscMY;
      string PzDXdIhjbF;
      string moLxQONSHj;
      if(BfbGOkPOOg == tcSFemGFNi){RVMrhtJgfA = true;}
      else if(tcSFemGFNi == BfbGOkPOOg){DaCEPFKbyd = true;}
      if(aSiMiNHVDa == eaaDqBQLnf){WDUIstjwmD = true;}
      else if(eaaDqBQLnf == aSiMiNHVDa){aipHVYzeeR = true;}
      if(UdSymZtARb == wdwjXVsSjw){EwHbjoLHMP = true;}
      else if(wdwjXVsSjw == UdSymZtARb){COwOVDUHBI = true;}
      if(qqpiZlKQwE == WrtqVTWsGK){AZVbHsmLpj = true;}
      else if(WrtqVTWsGK == qqpiZlKQwE){UHFIwuGyYf = true;}
      if(wWlguWbFCg == ChkSWyFsPM){yBDahjTDEW = true;}
      else if(ChkSWyFsPM == wWlguWbFCg){XRTSKuLubb = true;}
      if(dgDrgWMdTo == ROKKOQfhgI){dQfDfUOjHr = true;}
      else if(ROKKOQfhgI == dgDrgWMdTo){wCGIhndniM = true;}
      if(qOwmpjbzmx == snURLlfigX){hCTQAxgrAz = true;}
      else if(snURLlfigX == qOwmpjbzmx){jCZpITdhLa = true;}
      if(CpZydxrkCP == dWEddNscMY){GYGDECsfsz = true;}
      if(xoykyJmyCC == PzDXdIhjbF){VICokitRwl = true;}
      if(YgxGNhZzqL == moLxQONSHj){dCDyBuglak = true;}
      while(dWEddNscMY == CpZydxrkCP){SBGysjudDZ = true;}
      while(PzDXdIhjbF == PzDXdIhjbF){qZfYMSAwaH = true;}
      while(moLxQONSHj == moLxQONSHj){QSOMwVrdKp = true;}
      if(RVMrhtJgfA == true){RVMrhtJgfA = false;}
      if(WDUIstjwmD == true){WDUIstjwmD = false;}
      if(EwHbjoLHMP == true){EwHbjoLHMP = false;}
      if(AZVbHsmLpj == true){AZVbHsmLpj = false;}
      if(yBDahjTDEW == true){yBDahjTDEW = false;}
      if(dQfDfUOjHr == true){dQfDfUOjHr = false;}
      if(hCTQAxgrAz == true){hCTQAxgrAz = false;}
      if(GYGDECsfsz == true){GYGDECsfsz = false;}
      if(VICokitRwl == true){VICokitRwl = false;}
      if(dCDyBuglak == true){dCDyBuglak = false;}
      if(DaCEPFKbyd == true){DaCEPFKbyd = false;}
      if(aipHVYzeeR == true){aipHVYzeeR = false;}
      if(COwOVDUHBI == true){COwOVDUHBI = false;}
      if(UHFIwuGyYf == true){UHFIwuGyYf = false;}
      if(XRTSKuLubb == true){XRTSKuLubb = false;}
      if(wCGIhndniM == true){wCGIhndniM = false;}
      if(jCZpITdhLa == true){jCZpITdhLa = false;}
      if(SBGysjudDZ == true){SBGysjudDZ = false;}
      if(qZfYMSAwaH == true){qZfYMSAwaH = false;}
      if(QSOMwVrdKp == true){QSOMwVrdKp = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BOTISQXEPT
{ 
  void QSYEfMqNyX()
  { 
      bool auiMEXllOV = false;
      bool MVPVzGOpHf = false;
      bool GWBoIFreCq = false;
      bool rLeTbiSCZn = false;
      bool alEIuRtWgV = false;
      bool QwBSdDssEy = false;
      bool eHjXdBpwtQ = false;
      bool mMQSFTSrtp = false;
      bool LVxgZuJLpY = false;
      bool oYybISCfes = false;
      bool bxbQhEfiNJ = false;
      bool jAyNlIJDxh = false;
      bool qeFVaNNoJl = false;
      bool RlNAONtgzV = false;
      bool PaygnoZjoz = false;
      bool JzixCwqGpT = false;
      bool pqYaqxmxyX = false;
      bool OcDaRPfYgk = false;
      bool nczKbSDPyR = false;
      bool VmqefYCXoq = false;
      string ZwJbCPwtJh;
      string UJJVEZgLnL;
      string SmgYRGDhcU;
      string mHslUZbdpa;
      string blTLCwtmWh;
      string sYgZlmOkJw;
      string LOQotNQWAe;
      string yCUNjfSUVV;
      string LQKuaOaSzh;
      string KqTtONUdQo;
      string fXlrjpESpF;
      string TtOmjtDVNf;
      string mxXSTGhZyW;
      string LoDgpndRKj;
      string JHcsOQcgJU;
      string JyQkBhzTYd;
      string IeUTtepgxw;
      string QkLdVeUdDC;
      string dBPdMNXSeH;
      string nKrgbemIzj;
      if(ZwJbCPwtJh == fXlrjpESpF){auiMEXllOV = true;}
      else if(fXlrjpESpF == ZwJbCPwtJh){bxbQhEfiNJ = true;}
      if(UJJVEZgLnL == TtOmjtDVNf){MVPVzGOpHf = true;}
      else if(TtOmjtDVNf == UJJVEZgLnL){jAyNlIJDxh = true;}
      if(SmgYRGDhcU == mxXSTGhZyW){GWBoIFreCq = true;}
      else if(mxXSTGhZyW == SmgYRGDhcU){qeFVaNNoJl = true;}
      if(mHslUZbdpa == LoDgpndRKj){rLeTbiSCZn = true;}
      else if(LoDgpndRKj == mHslUZbdpa){RlNAONtgzV = true;}
      if(blTLCwtmWh == JHcsOQcgJU){alEIuRtWgV = true;}
      else if(JHcsOQcgJU == blTLCwtmWh){PaygnoZjoz = true;}
      if(sYgZlmOkJw == JyQkBhzTYd){QwBSdDssEy = true;}
      else if(JyQkBhzTYd == sYgZlmOkJw){JzixCwqGpT = true;}
      if(LOQotNQWAe == IeUTtepgxw){eHjXdBpwtQ = true;}
      else if(IeUTtepgxw == LOQotNQWAe){pqYaqxmxyX = true;}
      if(yCUNjfSUVV == QkLdVeUdDC){mMQSFTSrtp = true;}
      if(LQKuaOaSzh == dBPdMNXSeH){LVxgZuJLpY = true;}
      if(KqTtONUdQo == nKrgbemIzj){oYybISCfes = true;}
      while(QkLdVeUdDC == yCUNjfSUVV){OcDaRPfYgk = true;}
      while(dBPdMNXSeH == dBPdMNXSeH){nczKbSDPyR = true;}
      while(nKrgbemIzj == nKrgbemIzj){VmqefYCXoq = true;}
      if(auiMEXllOV == true){auiMEXllOV = false;}
      if(MVPVzGOpHf == true){MVPVzGOpHf = false;}
      if(GWBoIFreCq == true){GWBoIFreCq = false;}
      if(rLeTbiSCZn == true){rLeTbiSCZn = false;}
      if(alEIuRtWgV == true){alEIuRtWgV = false;}
      if(QwBSdDssEy == true){QwBSdDssEy = false;}
      if(eHjXdBpwtQ == true){eHjXdBpwtQ = false;}
      if(mMQSFTSrtp == true){mMQSFTSrtp = false;}
      if(LVxgZuJLpY == true){LVxgZuJLpY = false;}
      if(oYybISCfes == true){oYybISCfes = false;}
      if(bxbQhEfiNJ == true){bxbQhEfiNJ = false;}
      if(jAyNlIJDxh == true){jAyNlIJDxh = false;}
      if(qeFVaNNoJl == true){qeFVaNNoJl = false;}
      if(RlNAONtgzV == true){RlNAONtgzV = false;}
      if(PaygnoZjoz == true){PaygnoZjoz = false;}
      if(JzixCwqGpT == true){JzixCwqGpT = false;}
      if(pqYaqxmxyX == true){pqYaqxmxyX = false;}
      if(OcDaRPfYgk == true){OcDaRPfYgk = false;}
      if(nczKbSDPyR == true){nczKbSDPyR = false;}
      if(VmqefYCXoq == true){VmqefYCXoq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SANENJJGYZ
{ 
  void ICJqfNUzPM()
  { 
      bool RokowHbZYx = false;
      bool LhukDBqIaQ = false;
      bool BYlMVbUgXE = false;
      bool Hpermowjlx = false;
      bool qnnlheadZX = false;
      bool KCGfnsCXST = false;
      bool mtBTpUDDyN = false;
      bool FGWdYMjzdp = false;
      bool BpHAtEjyEt = false;
      bool lXQRJxlUuN = false;
      bool bWeLRDfbHa = false;
      bool IDsbSwmBYl = false;
      bool kEyDbhoEUa = false;
      bool ZPEQOdLuhq = false;
      bool YHqckidoyM = false;
      bool KkuJcWXajj = false;
      bool NaHfArycNo = false;
      bool kqShNcNURs = false;
      bool DAFIQnUunr = false;
      bool FjKycxAKcs = false;
      string Wdapfbtdmh;
      string GfIGyOMFZI;
      string bJFQEkQyaT;
      string bNAdRGqNaE;
      string dMuJdDwDqs;
      string FtqHGNYJlu;
      string LwDTGrfeoq;
      string WzdVdPNulB;
      string nNGgNhWrWD;
      string hhbfrhIbDl;
      string XRNAWGPUcp;
      string AdWjGxpVFY;
      string UmHkJoXycw;
      string GkrAdWzGKH;
      string RCOfDDKqYt;
      string qmWeQjJcqg;
      string hQqfjOlVGj;
      string yjKxGSYagF;
      string dgZhFTNMjs;
      string NWOdYBLhwo;
      if(Wdapfbtdmh == XRNAWGPUcp){RokowHbZYx = true;}
      else if(XRNAWGPUcp == Wdapfbtdmh){bWeLRDfbHa = true;}
      if(GfIGyOMFZI == AdWjGxpVFY){LhukDBqIaQ = true;}
      else if(AdWjGxpVFY == GfIGyOMFZI){IDsbSwmBYl = true;}
      if(bJFQEkQyaT == UmHkJoXycw){BYlMVbUgXE = true;}
      else if(UmHkJoXycw == bJFQEkQyaT){kEyDbhoEUa = true;}
      if(bNAdRGqNaE == GkrAdWzGKH){Hpermowjlx = true;}
      else if(GkrAdWzGKH == bNAdRGqNaE){ZPEQOdLuhq = true;}
      if(dMuJdDwDqs == RCOfDDKqYt){qnnlheadZX = true;}
      else if(RCOfDDKqYt == dMuJdDwDqs){YHqckidoyM = true;}
      if(FtqHGNYJlu == qmWeQjJcqg){KCGfnsCXST = true;}
      else if(qmWeQjJcqg == FtqHGNYJlu){KkuJcWXajj = true;}
      if(LwDTGrfeoq == hQqfjOlVGj){mtBTpUDDyN = true;}
      else if(hQqfjOlVGj == LwDTGrfeoq){NaHfArycNo = true;}
      if(WzdVdPNulB == yjKxGSYagF){FGWdYMjzdp = true;}
      if(nNGgNhWrWD == dgZhFTNMjs){BpHAtEjyEt = true;}
      if(hhbfrhIbDl == NWOdYBLhwo){lXQRJxlUuN = true;}
      while(yjKxGSYagF == WzdVdPNulB){kqShNcNURs = true;}
      while(dgZhFTNMjs == dgZhFTNMjs){DAFIQnUunr = true;}
      while(NWOdYBLhwo == NWOdYBLhwo){FjKycxAKcs = true;}
      if(RokowHbZYx == true){RokowHbZYx = false;}
      if(LhukDBqIaQ == true){LhukDBqIaQ = false;}
      if(BYlMVbUgXE == true){BYlMVbUgXE = false;}
      if(Hpermowjlx == true){Hpermowjlx = false;}
      if(qnnlheadZX == true){qnnlheadZX = false;}
      if(KCGfnsCXST == true){KCGfnsCXST = false;}
      if(mtBTpUDDyN == true){mtBTpUDDyN = false;}
      if(FGWdYMjzdp == true){FGWdYMjzdp = false;}
      if(BpHAtEjyEt == true){BpHAtEjyEt = false;}
      if(lXQRJxlUuN == true){lXQRJxlUuN = false;}
      if(bWeLRDfbHa == true){bWeLRDfbHa = false;}
      if(IDsbSwmBYl == true){IDsbSwmBYl = false;}
      if(kEyDbhoEUa == true){kEyDbhoEUa = false;}
      if(ZPEQOdLuhq == true){ZPEQOdLuhq = false;}
      if(YHqckidoyM == true){YHqckidoyM = false;}
      if(KkuJcWXajj == true){KkuJcWXajj = false;}
      if(NaHfArycNo == true){NaHfArycNo = false;}
      if(kqShNcNURs == true){kqShNcNURs = false;}
      if(DAFIQnUunr == true){DAFIQnUunr = false;}
      if(FjKycxAKcs == true){FjKycxAKcs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BXOPVJGNJD
{ 
  void TcbelPqJsj()
  { 
      bool bNpeMsQfjP = false;
      bool VCzSjZhfYQ = false;
      bool PyXgUBqEHS = false;
      bool NbIUbmCnYs = false;
      bool LwBweJkpqb = false;
      bool FRaKMdpIJL = false;
      bool CLYQCsseqw = false;
      bool WsGeFSwaBL = false;
      bool PBMfMuByIx = false;
      bool xteNqzXjtk = false;
      bool QGztFMKuFs = false;
      bool gOHyuBzrEG = false;
      bool ruWszcOrmF = false;
      bool tMgPtIBRaA = false;
      bool jFsOPilgRp = false;
      bool AfensVjFub = false;
      bool xSHzyUgqDm = false;
      bool zgREVlBHdP = false;
      bool phaxFOgsWY = false;
      bool YwWyCRxsPu = false;
      string dxgNuXpfmE;
      string MqIWjlhuoe;
      string thXaDWGqqG;
      string iWCTZxXNSk;
      string XeyIoWfKZw;
      string DKGzhuZGKX;
      string QynfVpoowm;
      string OTbEGPjOkf;
      string pwRuDYQDke;
      string WRDuTZpRmz;
      string hpNlhihBrR;
      string KLUehUfscw;
      string YlDRPxQgkg;
      string YRblxzhfMT;
      string pOKTULwXMz;
      string sxopZwmwWk;
      string SBQLrriihh;
      string lAfIGKajwD;
      string AsTkmbckLA;
      string MkIbBnxzab;
      if(dxgNuXpfmE == hpNlhihBrR){bNpeMsQfjP = true;}
      else if(hpNlhihBrR == dxgNuXpfmE){QGztFMKuFs = true;}
      if(MqIWjlhuoe == KLUehUfscw){VCzSjZhfYQ = true;}
      else if(KLUehUfscw == MqIWjlhuoe){gOHyuBzrEG = true;}
      if(thXaDWGqqG == YlDRPxQgkg){PyXgUBqEHS = true;}
      else if(YlDRPxQgkg == thXaDWGqqG){ruWszcOrmF = true;}
      if(iWCTZxXNSk == YRblxzhfMT){NbIUbmCnYs = true;}
      else if(YRblxzhfMT == iWCTZxXNSk){tMgPtIBRaA = true;}
      if(XeyIoWfKZw == pOKTULwXMz){LwBweJkpqb = true;}
      else if(pOKTULwXMz == XeyIoWfKZw){jFsOPilgRp = true;}
      if(DKGzhuZGKX == sxopZwmwWk){FRaKMdpIJL = true;}
      else if(sxopZwmwWk == DKGzhuZGKX){AfensVjFub = true;}
      if(QynfVpoowm == SBQLrriihh){CLYQCsseqw = true;}
      else if(SBQLrriihh == QynfVpoowm){xSHzyUgqDm = true;}
      if(OTbEGPjOkf == lAfIGKajwD){WsGeFSwaBL = true;}
      if(pwRuDYQDke == AsTkmbckLA){PBMfMuByIx = true;}
      if(WRDuTZpRmz == MkIbBnxzab){xteNqzXjtk = true;}
      while(lAfIGKajwD == OTbEGPjOkf){zgREVlBHdP = true;}
      while(AsTkmbckLA == AsTkmbckLA){phaxFOgsWY = true;}
      while(MkIbBnxzab == MkIbBnxzab){YwWyCRxsPu = true;}
      if(bNpeMsQfjP == true){bNpeMsQfjP = false;}
      if(VCzSjZhfYQ == true){VCzSjZhfYQ = false;}
      if(PyXgUBqEHS == true){PyXgUBqEHS = false;}
      if(NbIUbmCnYs == true){NbIUbmCnYs = false;}
      if(LwBweJkpqb == true){LwBweJkpqb = false;}
      if(FRaKMdpIJL == true){FRaKMdpIJL = false;}
      if(CLYQCsseqw == true){CLYQCsseqw = false;}
      if(WsGeFSwaBL == true){WsGeFSwaBL = false;}
      if(PBMfMuByIx == true){PBMfMuByIx = false;}
      if(xteNqzXjtk == true){xteNqzXjtk = false;}
      if(QGztFMKuFs == true){QGztFMKuFs = false;}
      if(gOHyuBzrEG == true){gOHyuBzrEG = false;}
      if(ruWszcOrmF == true){ruWszcOrmF = false;}
      if(tMgPtIBRaA == true){tMgPtIBRaA = false;}
      if(jFsOPilgRp == true){jFsOPilgRp = false;}
      if(AfensVjFub == true){AfensVjFub = false;}
      if(xSHzyUgqDm == true){xSHzyUgqDm = false;}
      if(zgREVlBHdP == true){zgREVlBHdP = false;}
      if(phaxFOgsWY == true){phaxFOgsWY = false;}
      if(YwWyCRxsPu == true){YwWyCRxsPu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XQUXNGRQEF
{ 
  void wecJunNbCn()
  { 
      bool cpdkMxUJyf = false;
      bool jWzUCWmPmr = false;
      bool gsqPLFdTfl = false;
      bool JBijWUUaHQ = false;
      bool waCGgjTDFN = false;
      bool OFzrajzNYG = false;
      bool TBdLWhmPyg = false;
      bool CMIWwGqkib = false;
      bool lZOSWEBpIm = false;
      bool VGCAwuxpjy = false;
      bool wDFXLCwbwU = false;
      bool rOewTtlKoR = false;
      bool gxGLKuJHaB = false;
      bool IAAjfhTpGU = false;
      bool IYtntHaFgB = false;
      bool IlBSRzxtMP = false;
      bool rnBuLrNtQk = false;
      bool ZuETJHCRQD = false;
      bool TEaYrWzhSf = false;
      bool kZdqbTXzcB = false;
      string QhtrOEZCRN;
      string mRhTQDiXzl;
      string MYpVdtAupG;
      string KIhywsXsnY;
      string aFLcZlpfMZ;
      string UwOberrYNU;
      string EeMKCdjiWW;
      string eoiZLHNMur;
      string ETiPlNsFTE;
      string ByfTUwsAaw;
      string CGJoteNoQX;
      string eedVQjuwYn;
      string jdPRUgIOEO;
      string NMlzoxKsUG;
      string AEouZVUKDk;
      string AwblLYqqTN;
      string VyjBJRsefS;
      string YCaRnenTrz;
      string LyLTNRkAmQ;
      string GSYHkBUmkH;
      if(QhtrOEZCRN == CGJoteNoQX){cpdkMxUJyf = true;}
      else if(CGJoteNoQX == QhtrOEZCRN){wDFXLCwbwU = true;}
      if(mRhTQDiXzl == eedVQjuwYn){jWzUCWmPmr = true;}
      else if(eedVQjuwYn == mRhTQDiXzl){rOewTtlKoR = true;}
      if(MYpVdtAupG == jdPRUgIOEO){gsqPLFdTfl = true;}
      else if(jdPRUgIOEO == MYpVdtAupG){gxGLKuJHaB = true;}
      if(KIhywsXsnY == NMlzoxKsUG){JBijWUUaHQ = true;}
      else if(NMlzoxKsUG == KIhywsXsnY){IAAjfhTpGU = true;}
      if(aFLcZlpfMZ == AEouZVUKDk){waCGgjTDFN = true;}
      else if(AEouZVUKDk == aFLcZlpfMZ){IYtntHaFgB = true;}
      if(UwOberrYNU == AwblLYqqTN){OFzrajzNYG = true;}
      else if(AwblLYqqTN == UwOberrYNU){IlBSRzxtMP = true;}
      if(EeMKCdjiWW == VyjBJRsefS){TBdLWhmPyg = true;}
      else if(VyjBJRsefS == EeMKCdjiWW){rnBuLrNtQk = true;}
      if(eoiZLHNMur == YCaRnenTrz){CMIWwGqkib = true;}
      if(ETiPlNsFTE == LyLTNRkAmQ){lZOSWEBpIm = true;}
      if(ByfTUwsAaw == GSYHkBUmkH){VGCAwuxpjy = true;}
      while(YCaRnenTrz == eoiZLHNMur){ZuETJHCRQD = true;}
      while(LyLTNRkAmQ == LyLTNRkAmQ){TEaYrWzhSf = true;}
      while(GSYHkBUmkH == GSYHkBUmkH){kZdqbTXzcB = true;}
      if(cpdkMxUJyf == true){cpdkMxUJyf = false;}
      if(jWzUCWmPmr == true){jWzUCWmPmr = false;}
      if(gsqPLFdTfl == true){gsqPLFdTfl = false;}
      if(JBijWUUaHQ == true){JBijWUUaHQ = false;}
      if(waCGgjTDFN == true){waCGgjTDFN = false;}
      if(OFzrajzNYG == true){OFzrajzNYG = false;}
      if(TBdLWhmPyg == true){TBdLWhmPyg = false;}
      if(CMIWwGqkib == true){CMIWwGqkib = false;}
      if(lZOSWEBpIm == true){lZOSWEBpIm = false;}
      if(VGCAwuxpjy == true){VGCAwuxpjy = false;}
      if(wDFXLCwbwU == true){wDFXLCwbwU = false;}
      if(rOewTtlKoR == true){rOewTtlKoR = false;}
      if(gxGLKuJHaB == true){gxGLKuJHaB = false;}
      if(IAAjfhTpGU == true){IAAjfhTpGU = false;}
      if(IYtntHaFgB == true){IYtntHaFgB = false;}
      if(IlBSRzxtMP == true){IlBSRzxtMP = false;}
      if(rnBuLrNtQk == true){rnBuLrNtQk = false;}
      if(ZuETJHCRQD == true){ZuETJHCRQD = false;}
      if(TEaYrWzhSf == true){TEaYrWzhSf = false;}
      if(kZdqbTXzcB == true){kZdqbTXzcB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NAFNFSQCMA
{ 
  void DCMBafKFjT()
  { 
      bool JmOKgntLpu = false;
      bool ELNAXwfPuo = false;
      bool MFgGYBHAlz = false;
      bool KbecaAobxw = false;
      bool DKDjDQQXoG = false;
      bool tNAyLEFHkf = false;
      bool fsYpkbfjZW = false;
      bool HDSylCCwDI = false;
      bool jjKfEwbfQW = false;
      bool ugqQQJkMza = false;
      bool jgySypuBMJ = false;
      bool BIDcwuiskn = false;
      bool AASDaaMChb = false;
      bool QoxbXClrgO = false;
      bool BZTHUlcOUU = false;
      bool arbKajNuqX = false;
      bool dsSkrMLaEs = false;
      bool IpLWlZgLho = false;
      bool xxhNAYNlBn = false;
      bool nqBJjIGiOH = false;
      string HSbTLDKDze;
      string BfWCfrueJF;
      string zlOiHVLesX;
      string xcQIhVXmKe;
      string oCVSUsnspU;
      string nlRBnVErMp;
      string reJbgXPOUK;
      string UHTePwPuNN;
      string gRoYiLLOTM;
      string QeaUmKrslx;
      string dZXeAfRmlr;
      string FIeyIZpkrU;
      string MBuIsofyrF;
      string EWJQjHtJXD;
      string tYcbGIAcLN;
      string UGAQjcePkL;
      string AStfmhyNOB;
      string ffMoanYSBk;
      string nDlekxVsfH;
      string bdXMCadwnY;
      if(HSbTLDKDze == dZXeAfRmlr){JmOKgntLpu = true;}
      else if(dZXeAfRmlr == HSbTLDKDze){jgySypuBMJ = true;}
      if(BfWCfrueJF == FIeyIZpkrU){ELNAXwfPuo = true;}
      else if(FIeyIZpkrU == BfWCfrueJF){BIDcwuiskn = true;}
      if(zlOiHVLesX == MBuIsofyrF){MFgGYBHAlz = true;}
      else if(MBuIsofyrF == zlOiHVLesX){AASDaaMChb = true;}
      if(xcQIhVXmKe == EWJQjHtJXD){KbecaAobxw = true;}
      else if(EWJQjHtJXD == xcQIhVXmKe){QoxbXClrgO = true;}
      if(oCVSUsnspU == tYcbGIAcLN){DKDjDQQXoG = true;}
      else if(tYcbGIAcLN == oCVSUsnspU){BZTHUlcOUU = true;}
      if(nlRBnVErMp == UGAQjcePkL){tNAyLEFHkf = true;}
      else if(UGAQjcePkL == nlRBnVErMp){arbKajNuqX = true;}
      if(reJbgXPOUK == AStfmhyNOB){fsYpkbfjZW = true;}
      else if(AStfmhyNOB == reJbgXPOUK){dsSkrMLaEs = true;}
      if(UHTePwPuNN == ffMoanYSBk){HDSylCCwDI = true;}
      if(gRoYiLLOTM == nDlekxVsfH){jjKfEwbfQW = true;}
      if(QeaUmKrslx == bdXMCadwnY){ugqQQJkMza = true;}
      while(ffMoanYSBk == UHTePwPuNN){IpLWlZgLho = true;}
      while(nDlekxVsfH == nDlekxVsfH){xxhNAYNlBn = true;}
      while(bdXMCadwnY == bdXMCadwnY){nqBJjIGiOH = true;}
      if(JmOKgntLpu == true){JmOKgntLpu = false;}
      if(ELNAXwfPuo == true){ELNAXwfPuo = false;}
      if(MFgGYBHAlz == true){MFgGYBHAlz = false;}
      if(KbecaAobxw == true){KbecaAobxw = false;}
      if(DKDjDQQXoG == true){DKDjDQQXoG = false;}
      if(tNAyLEFHkf == true){tNAyLEFHkf = false;}
      if(fsYpkbfjZW == true){fsYpkbfjZW = false;}
      if(HDSylCCwDI == true){HDSylCCwDI = false;}
      if(jjKfEwbfQW == true){jjKfEwbfQW = false;}
      if(ugqQQJkMza == true){ugqQQJkMza = false;}
      if(jgySypuBMJ == true){jgySypuBMJ = false;}
      if(BIDcwuiskn == true){BIDcwuiskn = false;}
      if(AASDaaMChb == true){AASDaaMChb = false;}
      if(QoxbXClrgO == true){QoxbXClrgO = false;}
      if(BZTHUlcOUU == true){BZTHUlcOUU = false;}
      if(arbKajNuqX == true){arbKajNuqX = false;}
      if(dsSkrMLaEs == true){dsSkrMLaEs = false;}
      if(IpLWlZgLho == true){IpLWlZgLho = false;}
      if(xxhNAYNlBn == true){xxhNAYNlBn = false;}
      if(nqBJjIGiOH == true){nqBJjIGiOH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FKFAHZMQEJ
{ 
  void CqlwBkRDTY()
  { 
      bool lDjPdJfrOk = false;
      bool RGkhfczCFg = false;
      bool GYekFCwpuF = false;
      bool iyRJKxgGCL = false;
      bool YmACXSOFQb = false;
      bool zXJBkNXzKa = false;
      bool MowJBToaXh = false;
      bool uszsHTXcNm = false;
      bool mqZTFLoQYw = false;
      bool ZhubyjppAD = false;
      bool cnghfGXPsR = false;
      bool pukXmVOcEo = false;
      bool eYBaINxXtW = false;
      bool cgbJJzJqmy = false;
      bool mYWHGdwXnc = false;
      bool VNlLYElGZR = false;
      bool JAVVnITbNm = false;
      bool MWoFBGBtTd = false;
      bool QYiRBPuPkE = false;
      bool NCRVolDBSy = false;
      string AxTgCZXazz;
      string AVuXRHuBrk;
      string qczZchiwcd;
      string TejwIwOLbL;
      string XiNXgWZfZu;
      string HMbCSXysDU;
      string qayVfUlyPV;
      string rjZfMIOasG;
      string HCWgVHDgdk;
      string xVHHTxjrrL;
      string kaQDDWmupJ;
      string IWAwMnDQzY;
      string RETdXWcRMO;
      string XWigVglTdz;
      string TIpTcaVKuf;
      string sSrTtfGlfg;
      string tUialknSae;
      string CjmAEJNzBx;
      string ARVfVEePmo;
      string ZznwEyZPhq;
      if(AxTgCZXazz == kaQDDWmupJ){lDjPdJfrOk = true;}
      else if(kaQDDWmupJ == AxTgCZXazz){cnghfGXPsR = true;}
      if(AVuXRHuBrk == IWAwMnDQzY){RGkhfczCFg = true;}
      else if(IWAwMnDQzY == AVuXRHuBrk){pukXmVOcEo = true;}
      if(qczZchiwcd == RETdXWcRMO){GYekFCwpuF = true;}
      else if(RETdXWcRMO == qczZchiwcd){eYBaINxXtW = true;}
      if(TejwIwOLbL == XWigVglTdz){iyRJKxgGCL = true;}
      else if(XWigVglTdz == TejwIwOLbL){cgbJJzJqmy = true;}
      if(XiNXgWZfZu == TIpTcaVKuf){YmACXSOFQb = true;}
      else if(TIpTcaVKuf == XiNXgWZfZu){mYWHGdwXnc = true;}
      if(HMbCSXysDU == sSrTtfGlfg){zXJBkNXzKa = true;}
      else if(sSrTtfGlfg == HMbCSXysDU){VNlLYElGZR = true;}
      if(qayVfUlyPV == tUialknSae){MowJBToaXh = true;}
      else if(tUialknSae == qayVfUlyPV){JAVVnITbNm = true;}
      if(rjZfMIOasG == CjmAEJNzBx){uszsHTXcNm = true;}
      if(HCWgVHDgdk == ARVfVEePmo){mqZTFLoQYw = true;}
      if(xVHHTxjrrL == ZznwEyZPhq){ZhubyjppAD = true;}
      while(CjmAEJNzBx == rjZfMIOasG){MWoFBGBtTd = true;}
      while(ARVfVEePmo == ARVfVEePmo){QYiRBPuPkE = true;}
      while(ZznwEyZPhq == ZznwEyZPhq){NCRVolDBSy = true;}
      if(lDjPdJfrOk == true){lDjPdJfrOk = false;}
      if(RGkhfczCFg == true){RGkhfczCFg = false;}
      if(GYekFCwpuF == true){GYekFCwpuF = false;}
      if(iyRJKxgGCL == true){iyRJKxgGCL = false;}
      if(YmACXSOFQb == true){YmACXSOFQb = false;}
      if(zXJBkNXzKa == true){zXJBkNXzKa = false;}
      if(MowJBToaXh == true){MowJBToaXh = false;}
      if(uszsHTXcNm == true){uszsHTXcNm = false;}
      if(mqZTFLoQYw == true){mqZTFLoQYw = false;}
      if(ZhubyjppAD == true){ZhubyjppAD = false;}
      if(cnghfGXPsR == true){cnghfGXPsR = false;}
      if(pukXmVOcEo == true){pukXmVOcEo = false;}
      if(eYBaINxXtW == true){eYBaINxXtW = false;}
      if(cgbJJzJqmy == true){cgbJJzJqmy = false;}
      if(mYWHGdwXnc == true){mYWHGdwXnc = false;}
      if(VNlLYElGZR == true){VNlLYElGZR = false;}
      if(JAVVnITbNm == true){JAVVnITbNm = false;}
      if(MWoFBGBtTd == true){MWoFBGBtTd = false;}
      if(QYiRBPuPkE == true){QYiRBPuPkE = false;}
      if(NCRVolDBSy == true){NCRVolDBSy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YUOVXTLUBA
{ 
  void lwQphToQWs()
  { 
      bool MgUITmpXLz = false;
      bool yBKnknWmfT = false;
      bool PdIizfmEsK = false;
      bool GDUhxbMGlw = false;
      bool dKNhCUZOAo = false;
      bool EeCxliXlJm = false;
      bool zldAsufcpI = false;
      bool bQBipEPafd = false;
      bool FYDDGaBWuO = false;
      bool EaxywrgMnH = false;
      bool nxXrNndSHg = false;
      bool SxFegahZtR = false;
      bool tIgbEASClx = false;
      bool XDlYpIALoR = false;
      bool xhPNwHURQV = false;
      bool JGRWMePwmN = false;
      bool UagJJPKgoY = false;
      bool ZWjbTLsTfx = false;
      bool eAlPmQkPLK = false;
      bool rEXbxWAFDf = false;
      string SYuJbzjIIa;
      string exhHLRPaMV;
      string otXqhJtRdc;
      string cDmsysUCZm;
      string miyygcUlqW;
      string UgJLwXBDaS;
      string FReexbKoJS;
      string wNBZhGwYJF;
      string oIaHHutawa;
      string rQaHhUESAF;
      string SfcYZHfjcp;
      string abtVdxqXXC;
      string hirhZflsqN;
      string BdXElIeBgz;
      string OKxGzPWEdk;
      string cguyWjmtyt;
      string VmrPnxDpjU;
      string RppXmXkliK;
      string JgPFjUhZgB;
      string VkCldbRhRB;
      if(SYuJbzjIIa == SfcYZHfjcp){MgUITmpXLz = true;}
      else if(SfcYZHfjcp == SYuJbzjIIa){nxXrNndSHg = true;}
      if(exhHLRPaMV == abtVdxqXXC){yBKnknWmfT = true;}
      else if(abtVdxqXXC == exhHLRPaMV){SxFegahZtR = true;}
      if(otXqhJtRdc == hirhZflsqN){PdIizfmEsK = true;}
      else if(hirhZflsqN == otXqhJtRdc){tIgbEASClx = true;}
      if(cDmsysUCZm == BdXElIeBgz){GDUhxbMGlw = true;}
      else if(BdXElIeBgz == cDmsysUCZm){XDlYpIALoR = true;}
      if(miyygcUlqW == OKxGzPWEdk){dKNhCUZOAo = true;}
      else if(OKxGzPWEdk == miyygcUlqW){xhPNwHURQV = true;}
      if(UgJLwXBDaS == cguyWjmtyt){EeCxliXlJm = true;}
      else if(cguyWjmtyt == UgJLwXBDaS){JGRWMePwmN = true;}
      if(FReexbKoJS == VmrPnxDpjU){zldAsufcpI = true;}
      else if(VmrPnxDpjU == FReexbKoJS){UagJJPKgoY = true;}
      if(wNBZhGwYJF == RppXmXkliK){bQBipEPafd = true;}
      if(oIaHHutawa == JgPFjUhZgB){FYDDGaBWuO = true;}
      if(rQaHhUESAF == VkCldbRhRB){EaxywrgMnH = true;}
      while(RppXmXkliK == wNBZhGwYJF){ZWjbTLsTfx = true;}
      while(JgPFjUhZgB == JgPFjUhZgB){eAlPmQkPLK = true;}
      while(VkCldbRhRB == VkCldbRhRB){rEXbxWAFDf = true;}
      if(MgUITmpXLz == true){MgUITmpXLz = false;}
      if(yBKnknWmfT == true){yBKnknWmfT = false;}
      if(PdIizfmEsK == true){PdIizfmEsK = false;}
      if(GDUhxbMGlw == true){GDUhxbMGlw = false;}
      if(dKNhCUZOAo == true){dKNhCUZOAo = false;}
      if(EeCxliXlJm == true){EeCxliXlJm = false;}
      if(zldAsufcpI == true){zldAsufcpI = false;}
      if(bQBipEPafd == true){bQBipEPafd = false;}
      if(FYDDGaBWuO == true){FYDDGaBWuO = false;}
      if(EaxywrgMnH == true){EaxywrgMnH = false;}
      if(nxXrNndSHg == true){nxXrNndSHg = false;}
      if(SxFegahZtR == true){SxFegahZtR = false;}
      if(tIgbEASClx == true){tIgbEASClx = false;}
      if(XDlYpIALoR == true){XDlYpIALoR = false;}
      if(xhPNwHURQV == true){xhPNwHURQV = false;}
      if(JGRWMePwmN == true){JGRWMePwmN = false;}
      if(UagJJPKgoY == true){UagJJPKgoY = false;}
      if(ZWjbTLsTfx == true){ZWjbTLsTfx = false;}
      if(eAlPmQkPLK == true){eAlPmQkPLK = false;}
      if(rEXbxWAFDf == true){rEXbxWAFDf = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YVEUJPFQDH
{ 
  void GexYGNfGUr()
  { 
      bool ssTjWoRZLK = false;
      bool WrTcDGMPbo = false;
      bool tCRjKfwUuz = false;
      bool gBMunQTles = false;
      bool hrXrYqHfPq = false;
      bool HtiWlpbjYq = false;
      bool rwdMdVfYnl = false;
      bool mZkiGCeYKS = false;
      bool XPFDqlSpJy = false;
      bool oiMmAlNLOD = false;
      bool LiqdEgIAPi = false;
      bool LCZFTcqaUr = false;
      bool DoOpwDNyhU = false;
      bool oSKGHoXazK = false;
      bool cSZZlmJGTX = false;
      bool tlqWAVTBgA = false;
      bool TKRutrqSyd = false;
      bool EPbKOkPoEI = false;
      bool tGeKZCbnGk = false;
      bool UWtypRAees = false;
      string CqxFsefOZD;
      string QduouFSrVm;
      string DMYuyELMWL;
      string wTpZeAqSQd;
      string oTUITDLYTB;
      string iKqjdoSCGo;
      string STqOGwmUJu;
      string OAbrxirQbu;
      string rmsUbkbGuo;
      string goLaKUcGBW;
      string PqPYmiizlu;
      string xBZLwfaVlV;
      string UgYTNYnAgb;
      string GkPFeBXUhs;
      string CcUArfOAee;
      string tGTeCiRuKN;
      string thRJoLlUfu;
      string HFnsTqLYbh;
      string KnVBhzRjDw;
      string jbJTyTKKLf;
      if(CqxFsefOZD == PqPYmiizlu){ssTjWoRZLK = true;}
      else if(PqPYmiizlu == CqxFsefOZD){LiqdEgIAPi = true;}
      if(QduouFSrVm == xBZLwfaVlV){WrTcDGMPbo = true;}
      else if(xBZLwfaVlV == QduouFSrVm){LCZFTcqaUr = true;}
      if(DMYuyELMWL == UgYTNYnAgb){tCRjKfwUuz = true;}
      else if(UgYTNYnAgb == DMYuyELMWL){DoOpwDNyhU = true;}
      if(wTpZeAqSQd == GkPFeBXUhs){gBMunQTles = true;}
      else if(GkPFeBXUhs == wTpZeAqSQd){oSKGHoXazK = true;}
      if(oTUITDLYTB == CcUArfOAee){hrXrYqHfPq = true;}
      else if(CcUArfOAee == oTUITDLYTB){cSZZlmJGTX = true;}
      if(iKqjdoSCGo == tGTeCiRuKN){HtiWlpbjYq = true;}
      else if(tGTeCiRuKN == iKqjdoSCGo){tlqWAVTBgA = true;}
      if(STqOGwmUJu == thRJoLlUfu){rwdMdVfYnl = true;}
      else if(thRJoLlUfu == STqOGwmUJu){TKRutrqSyd = true;}
      if(OAbrxirQbu == HFnsTqLYbh){mZkiGCeYKS = true;}
      if(rmsUbkbGuo == KnVBhzRjDw){XPFDqlSpJy = true;}
      if(goLaKUcGBW == jbJTyTKKLf){oiMmAlNLOD = true;}
      while(HFnsTqLYbh == OAbrxirQbu){EPbKOkPoEI = true;}
      while(KnVBhzRjDw == KnVBhzRjDw){tGeKZCbnGk = true;}
      while(jbJTyTKKLf == jbJTyTKKLf){UWtypRAees = true;}
      if(ssTjWoRZLK == true){ssTjWoRZLK = false;}
      if(WrTcDGMPbo == true){WrTcDGMPbo = false;}
      if(tCRjKfwUuz == true){tCRjKfwUuz = false;}
      if(gBMunQTles == true){gBMunQTles = false;}
      if(hrXrYqHfPq == true){hrXrYqHfPq = false;}
      if(HtiWlpbjYq == true){HtiWlpbjYq = false;}
      if(rwdMdVfYnl == true){rwdMdVfYnl = false;}
      if(mZkiGCeYKS == true){mZkiGCeYKS = false;}
      if(XPFDqlSpJy == true){XPFDqlSpJy = false;}
      if(oiMmAlNLOD == true){oiMmAlNLOD = false;}
      if(LiqdEgIAPi == true){LiqdEgIAPi = false;}
      if(LCZFTcqaUr == true){LCZFTcqaUr = false;}
      if(DoOpwDNyhU == true){DoOpwDNyhU = false;}
      if(oSKGHoXazK == true){oSKGHoXazK = false;}
      if(cSZZlmJGTX == true){cSZZlmJGTX = false;}
      if(tlqWAVTBgA == true){tlqWAVTBgA = false;}
      if(TKRutrqSyd == true){TKRutrqSyd = false;}
      if(EPbKOkPoEI == true){EPbKOkPoEI = false;}
      if(tGeKZCbnGk == true){tGeKZCbnGk = false;}
      if(UWtypRAees == true){UWtypRAees = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RAYHXHOLTO
{ 
  void ZnzuXyOeLV()
  { 
      bool EFnIkephis = false;
      bool tLnqdcehVV = false;
      bool XwwHrpRmjB = false;
      bool ZcsPyDuLWu = false;
      bool QfYBxPDnuz = false;
      bool GXssHHIfQu = false;
      bool NCxhTRRYrZ = false;
      bool hSKZFqXuJb = false;
      bool lcqktBiAHq = false;
      bool uOubgOiqgf = false;
      bool xewVRmNSnQ = false;
      bool jMWnJjKMmR = false;
      bool iRQBdSXhsp = false;
      bool IgNdeGgVSH = false;
      bool nPLqGhrGRx = false;
      bool DVDPwfOtqy = false;
      bool OeZyHDzrJD = false;
      bool CnsfcTGZtu = false;
      bool ZQPullcygz = false;
      bool rmeUrJoNiC = false;
      string fgwWUiBpWb;
      string fbooIIEaWL;
      string neYNXfyuNu;
      string VoOEOCrPiP;
      string HDXzrDJXNI;
      string WnZDXBIhyn;
      string kFcaFqNjZG;
      string PnqHEjGYVU;
      string WZhEXhOKgM;
      string sJRteBaxtB;
      string oYxHriwSzm;
      string ujMdRKXXCX;
      string bDDNMGWxbD;
      string JIeUexcZLo;
      string kwLsRJxmcT;
      string cCuVYlVjtl;
      string MzpudlErzT;
      string hxMMmfrJuD;
      string IiUIZHNIur;
      string brfYckFPPf;
      if(fgwWUiBpWb == oYxHriwSzm){EFnIkephis = true;}
      else if(oYxHriwSzm == fgwWUiBpWb){xewVRmNSnQ = true;}
      if(fbooIIEaWL == ujMdRKXXCX){tLnqdcehVV = true;}
      else if(ujMdRKXXCX == fbooIIEaWL){jMWnJjKMmR = true;}
      if(neYNXfyuNu == bDDNMGWxbD){XwwHrpRmjB = true;}
      else if(bDDNMGWxbD == neYNXfyuNu){iRQBdSXhsp = true;}
      if(VoOEOCrPiP == JIeUexcZLo){ZcsPyDuLWu = true;}
      else if(JIeUexcZLo == VoOEOCrPiP){IgNdeGgVSH = true;}
      if(HDXzrDJXNI == kwLsRJxmcT){QfYBxPDnuz = true;}
      else if(kwLsRJxmcT == HDXzrDJXNI){nPLqGhrGRx = true;}
      if(WnZDXBIhyn == cCuVYlVjtl){GXssHHIfQu = true;}
      else if(cCuVYlVjtl == WnZDXBIhyn){DVDPwfOtqy = true;}
      if(kFcaFqNjZG == MzpudlErzT){NCxhTRRYrZ = true;}
      else if(MzpudlErzT == kFcaFqNjZG){OeZyHDzrJD = true;}
      if(PnqHEjGYVU == hxMMmfrJuD){hSKZFqXuJb = true;}
      if(WZhEXhOKgM == IiUIZHNIur){lcqktBiAHq = true;}
      if(sJRteBaxtB == brfYckFPPf){uOubgOiqgf = true;}
      while(hxMMmfrJuD == PnqHEjGYVU){CnsfcTGZtu = true;}
      while(IiUIZHNIur == IiUIZHNIur){ZQPullcygz = true;}
      while(brfYckFPPf == brfYckFPPf){rmeUrJoNiC = true;}
      if(EFnIkephis == true){EFnIkephis = false;}
      if(tLnqdcehVV == true){tLnqdcehVV = false;}
      if(XwwHrpRmjB == true){XwwHrpRmjB = false;}
      if(ZcsPyDuLWu == true){ZcsPyDuLWu = false;}
      if(QfYBxPDnuz == true){QfYBxPDnuz = false;}
      if(GXssHHIfQu == true){GXssHHIfQu = false;}
      if(NCxhTRRYrZ == true){NCxhTRRYrZ = false;}
      if(hSKZFqXuJb == true){hSKZFqXuJb = false;}
      if(lcqktBiAHq == true){lcqktBiAHq = false;}
      if(uOubgOiqgf == true){uOubgOiqgf = false;}
      if(xewVRmNSnQ == true){xewVRmNSnQ = false;}
      if(jMWnJjKMmR == true){jMWnJjKMmR = false;}
      if(iRQBdSXhsp == true){iRQBdSXhsp = false;}
      if(IgNdeGgVSH == true){IgNdeGgVSH = false;}
      if(nPLqGhrGRx == true){nPLqGhrGRx = false;}
      if(DVDPwfOtqy == true){DVDPwfOtqy = false;}
      if(OeZyHDzrJD == true){OeZyHDzrJD = false;}
      if(CnsfcTGZtu == true){CnsfcTGZtu = false;}
      if(ZQPullcygz == true){ZQPullcygz = false;}
      if(rmeUrJoNiC == true){rmeUrJoNiC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QQBZNCFSUK
{ 
  void CZsPNWZWVT()
  { 
      bool iDIKLyoCRI = false;
      bool XKpLsxLYbQ = false;
      bool sifMwZYzsq = false;
      bool WLEBViDHDU = false;
      bool AENPVoCiVX = false;
      bool HAoznAXuNk = false;
      bool ZpJalLGnpK = false;
      bool sDVGPnVPqG = false;
      bool XpdUjVWFNU = false;
      bool KIsRuKumdJ = false;
      bool iNrLdDehiz = false;
      bool eHEHNiLnpP = false;
      bool oAaUzjJELF = false;
      bool shlgXJxSqf = false;
      bool poVNpDElDL = false;
      bool yHwNUhlXLs = false;
      bool taWcGwVWiE = false;
      bool kswKYAeMdw = false;
      bool fWAzHllXMo = false;
      bool WZGxlPSrHt = false;
      string xsuoVGbJCX;
      string ppoVBGsdQn;
      string ywgRFnNSft;
      string PjIcDfUASI;
      string FpQqVVjGoL;
      string dOoPGALdjh;
      string rIcIHFgMRV;
      string lrKIhHepui;
      string EHdTPAusVC;
      string TqQkPwNIiQ;
      string oWoSZkDggz;
      string VViBdkdIrG;
      string KnqPCtbZsl;
      string sWHdtgSKte;
      string xNLVfxIPHd;
      string REPbFAfYQe;
      string JdnuFzOEGw;
      string STONJxbUiE;
      string lMSOToGeYs;
      string AstOUSZQPg;
      if(xsuoVGbJCX == oWoSZkDggz){iDIKLyoCRI = true;}
      else if(oWoSZkDggz == xsuoVGbJCX){iNrLdDehiz = true;}
      if(ppoVBGsdQn == VViBdkdIrG){XKpLsxLYbQ = true;}
      else if(VViBdkdIrG == ppoVBGsdQn){eHEHNiLnpP = true;}
      if(ywgRFnNSft == KnqPCtbZsl){sifMwZYzsq = true;}
      else if(KnqPCtbZsl == ywgRFnNSft){oAaUzjJELF = true;}
      if(PjIcDfUASI == sWHdtgSKte){WLEBViDHDU = true;}
      else if(sWHdtgSKte == PjIcDfUASI){shlgXJxSqf = true;}
      if(FpQqVVjGoL == xNLVfxIPHd){AENPVoCiVX = true;}
      else if(xNLVfxIPHd == FpQqVVjGoL){poVNpDElDL = true;}
      if(dOoPGALdjh == REPbFAfYQe){HAoznAXuNk = true;}
      else if(REPbFAfYQe == dOoPGALdjh){yHwNUhlXLs = true;}
      if(rIcIHFgMRV == JdnuFzOEGw){ZpJalLGnpK = true;}
      else if(JdnuFzOEGw == rIcIHFgMRV){taWcGwVWiE = true;}
      if(lrKIhHepui == STONJxbUiE){sDVGPnVPqG = true;}
      if(EHdTPAusVC == lMSOToGeYs){XpdUjVWFNU = true;}
      if(TqQkPwNIiQ == AstOUSZQPg){KIsRuKumdJ = true;}
      while(STONJxbUiE == lrKIhHepui){kswKYAeMdw = true;}
      while(lMSOToGeYs == lMSOToGeYs){fWAzHllXMo = true;}
      while(AstOUSZQPg == AstOUSZQPg){WZGxlPSrHt = true;}
      if(iDIKLyoCRI == true){iDIKLyoCRI = false;}
      if(XKpLsxLYbQ == true){XKpLsxLYbQ = false;}
      if(sifMwZYzsq == true){sifMwZYzsq = false;}
      if(WLEBViDHDU == true){WLEBViDHDU = false;}
      if(AENPVoCiVX == true){AENPVoCiVX = false;}
      if(HAoznAXuNk == true){HAoznAXuNk = false;}
      if(ZpJalLGnpK == true){ZpJalLGnpK = false;}
      if(sDVGPnVPqG == true){sDVGPnVPqG = false;}
      if(XpdUjVWFNU == true){XpdUjVWFNU = false;}
      if(KIsRuKumdJ == true){KIsRuKumdJ = false;}
      if(iNrLdDehiz == true){iNrLdDehiz = false;}
      if(eHEHNiLnpP == true){eHEHNiLnpP = false;}
      if(oAaUzjJELF == true){oAaUzjJELF = false;}
      if(shlgXJxSqf == true){shlgXJxSqf = false;}
      if(poVNpDElDL == true){poVNpDElDL = false;}
      if(yHwNUhlXLs == true){yHwNUhlXLs = false;}
      if(taWcGwVWiE == true){taWcGwVWiE = false;}
      if(kswKYAeMdw == true){kswKYAeMdw = false;}
      if(fWAzHllXMo == true){fWAzHllXMo = false;}
      if(WZGxlPSrHt == true){WZGxlPSrHt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XXKWKWBFUY
{ 
  void MdkkEubgKy()
  { 
      bool mYfxQZgTzX = false;
      bool AzZzofLujN = false;
      bool ExTMVpfIic = false;
      bool jEQsYexTxK = false;
      bool jSSesDuTAX = false;
      bool FgkdkoETbZ = false;
      bool sYkVnwifZJ = false;
      bool irxYcleoSR = false;
      bool uMHKXCSlIi = false;
      bool acrXQLhwqx = false;
      bool UPsqoNiMAy = false;
      bool JPBJegQZyk = false;
      bool cwXqiFAZHR = false;
      bool DixVweSQVJ = false;
      bool TGkJZLbhrj = false;
      bool tHyGTXrCIi = false;
      bool NIEiuaRfNz = false;
      bool NAEnRWLNQW = false;
      bool dRgXXwjqBX = false;
      bool mQIluKBCOy = false;
      string hsaUDIPreM;
      string oDSmsyemcJ;
      string nuYsjSMceq;
      string QCSnEzpzsQ;
      string rbtdVZSIVB;
      string EIOwolUhkk;
      string umMmwmkeit;
      string HetMQMBtWJ;
      string zEHdmHnVzS;
      string dHOHUFOHhi;
      string onYbsZOBfb;
      string kaLakqeiAn;
      string yOeQPAQlqK;
      string xsLmULefjN;
      string slpWgCwmVc;
      string ZfGtjZIILs;
      string JbJUeqPXGY;
      string MzymtMYIiM;
      string DcQOkRmRDt;
      string DVTpERbUro;
      if(hsaUDIPreM == onYbsZOBfb){mYfxQZgTzX = true;}
      else if(onYbsZOBfb == hsaUDIPreM){UPsqoNiMAy = true;}
      if(oDSmsyemcJ == kaLakqeiAn){AzZzofLujN = true;}
      else if(kaLakqeiAn == oDSmsyemcJ){JPBJegQZyk = true;}
      if(nuYsjSMceq == yOeQPAQlqK){ExTMVpfIic = true;}
      else if(yOeQPAQlqK == nuYsjSMceq){cwXqiFAZHR = true;}
      if(QCSnEzpzsQ == xsLmULefjN){jEQsYexTxK = true;}
      else if(xsLmULefjN == QCSnEzpzsQ){DixVweSQVJ = true;}
      if(rbtdVZSIVB == slpWgCwmVc){jSSesDuTAX = true;}
      else if(slpWgCwmVc == rbtdVZSIVB){TGkJZLbhrj = true;}
      if(EIOwolUhkk == ZfGtjZIILs){FgkdkoETbZ = true;}
      else if(ZfGtjZIILs == EIOwolUhkk){tHyGTXrCIi = true;}
      if(umMmwmkeit == JbJUeqPXGY){sYkVnwifZJ = true;}
      else if(JbJUeqPXGY == umMmwmkeit){NIEiuaRfNz = true;}
      if(HetMQMBtWJ == MzymtMYIiM){irxYcleoSR = true;}
      if(zEHdmHnVzS == DcQOkRmRDt){uMHKXCSlIi = true;}
      if(dHOHUFOHhi == DVTpERbUro){acrXQLhwqx = true;}
      while(MzymtMYIiM == HetMQMBtWJ){NAEnRWLNQW = true;}
      while(DcQOkRmRDt == DcQOkRmRDt){dRgXXwjqBX = true;}
      while(DVTpERbUro == DVTpERbUro){mQIluKBCOy = true;}
      if(mYfxQZgTzX == true){mYfxQZgTzX = false;}
      if(AzZzofLujN == true){AzZzofLujN = false;}
      if(ExTMVpfIic == true){ExTMVpfIic = false;}
      if(jEQsYexTxK == true){jEQsYexTxK = false;}
      if(jSSesDuTAX == true){jSSesDuTAX = false;}
      if(FgkdkoETbZ == true){FgkdkoETbZ = false;}
      if(sYkVnwifZJ == true){sYkVnwifZJ = false;}
      if(irxYcleoSR == true){irxYcleoSR = false;}
      if(uMHKXCSlIi == true){uMHKXCSlIi = false;}
      if(acrXQLhwqx == true){acrXQLhwqx = false;}
      if(UPsqoNiMAy == true){UPsqoNiMAy = false;}
      if(JPBJegQZyk == true){JPBJegQZyk = false;}
      if(cwXqiFAZHR == true){cwXqiFAZHR = false;}
      if(DixVweSQVJ == true){DixVweSQVJ = false;}
      if(TGkJZLbhrj == true){TGkJZLbhrj = false;}
      if(tHyGTXrCIi == true){tHyGTXrCIi = false;}
      if(NIEiuaRfNz == true){NIEiuaRfNz = false;}
      if(NAEnRWLNQW == true){NAEnRWLNQW = false;}
      if(dRgXXwjqBX == true){dRgXXwjqBX = false;}
      if(mQIluKBCOy == true){mQIluKBCOy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GQFVTKEBQT
{ 
  void wqzHdayUOT()
  { 
      bool rmrVOaXyFG = false;
      bool SjfllCuRcx = false;
      bool RlAqtkoNqE = false;
      bool zRVCBXRygk = false;
      bool VtlOPOLYsG = false;
      bool wzSEoGlAac = false;
      bool kKGBHAOdjO = false;
      bool jxkqbWzlbe = false;
      bool ZQUkHAzRMu = false;
      bool kRWDzKnkHO = false;
      bool AgndVzNabS = false;
      bool GAYuDUpVtG = false;
      bool AwRocSZhrQ = false;
      bool hOdMesbpOH = false;
      bool lYosnjsaAe = false;
      bool URpAODPzAD = false;
      bool ZfhKjrWLQJ = false;
      bool rzHmUbHYAs = false;
      bool ZzgMwQXcya = false;
      bool ccoKhFpeIw = false;
      string QqfMrNYaKU;
      string wFCXdxUxyE;
      string yGSEfNGtKf;
      string IVuqdSBlwI;
      string xCwRsniJGO;
      string WBpixKYbfe;
      string GeQdLaKKsK;
      string whBoGuJALr;
      string dxmuJHeMtS;
      string tZGQBSkSGS;
      string dXPwSnAAbB;
      string BXPnAJqUUE;
      string uxQaPVbjmm;
      string JqtsOGgqbU;
      string HdjsRVKRPr;
      string aQxCWfHofx;
      string MPNoqKdWZt;
      string cArzwQNWJt;
      string LJTuLssrVh;
      string zqKeIrQuBH;
      if(QqfMrNYaKU == dXPwSnAAbB){rmrVOaXyFG = true;}
      else if(dXPwSnAAbB == QqfMrNYaKU){AgndVzNabS = true;}
      if(wFCXdxUxyE == BXPnAJqUUE){SjfllCuRcx = true;}
      else if(BXPnAJqUUE == wFCXdxUxyE){GAYuDUpVtG = true;}
      if(yGSEfNGtKf == uxQaPVbjmm){RlAqtkoNqE = true;}
      else if(uxQaPVbjmm == yGSEfNGtKf){AwRocSZhrQ = true;}
      if(IVuqdSBlwI == JqtsOGgqbU){zRVCBXRygk = true;}
      else if(JqtsOGgqbU == IVuqdSBlwI){hOdMesbpOH = true;}
      if(xCwRsniJGO == HdjsRVKRPr){VtlOPOLYsG = true;}
      else if(HdjsRVKRPr == xCwRsniJGO){lYosnjsaAe = true;}
      if(WBpixKYbfe == aQxCWfHofx){wzSEoGlAac = true;}
      else if(aQxCWfHofx == WBpixKYbfe){URpAODPzAD = true;}
      if(GeQdLaKKsK == MPNoqKdWZt){kKGBHAOdjO = true;}
      else if(MPNoqKdWZt == GeQdLaKKsK){ZfhKjrWLQJ = true;}
      if(whBoGuJALr == cArzwQNWJt){jxkqbWzlbe = true;}
      if(dxmuJHeMtS == LJTuLssrVh){ZQUkHAzRMu = true;}
      if(tZGQBSkSGS == zqKeIrQuBH){kRWDzKnkHO = true;}
      while(cArzwQNWJt == whBoGuJALr){rzHmUbHYAs = true;}
      while(LJTuLssrVh == LJTuLssrVh){ZzgMwQXcya = true;}
      while(zqKeIrQuBH == zqKeIrQuBH){ccoKhFpeIw = true;}
      if(rmrVOaXyFG == true){rmrVOaXyFG = false;}
      if(SjfllCuRcx == true){SjfllCuRcx = false;}
      if(RlAqtkoNqE == true){RlAqtkoNqE = false;}
      if(zRVCBXRygk == true){zRVCBXRygk = false;}
      if(VtlOPOLYsG == true){VtlOPOLYsG = false;}
      if(wzSEoGlAac == true){wzSEoGlAac = false;}
      if(kKGBHAOdjO == true){kKGBHAOdjO = false;}
      if(jxkqbWzlbe == true){jxkqbWzlbe = false;}
      if(ZQUkHAzRMu == true){ZQUkHAzRMu = false;}
      if(kRWDzKnkHO == true){kRWDzKnkHO = false;}
      if(AgndVzNabS == true){AgndVzNabS = false;}
      if(GAYuDUpVtG == true){GAYuDUpVtG = false;}
      if(AwRocSZhrQ == true){AwRocSZhrQ = false;}
      if(hOdMesbpOH == true){hOdMesbpOH = false;}
      if(lYosnjsaAe == true){lYosnjsaAe = false;}
      if(URpAODPzAD == true){URpAODPzAD = false;}
      if(ZfhKjrWLQJ == true){ZfhKjrWLQJ = false;}
      if(rzHmUbHYAs == true){rzHmUbHYAs = false;}
      if(ZzgMwQXcya == true){ZzgMwQXcya = false;}
      if(ccoKhFpeIw == true){ccoKhFpeIw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZVMDUOJVWK
{ 
  void xLUywMSxsq()
  { 
      bool UwQzsfFeYs = false;
      bool tjZmYsyuLr = false;
      bool EEzDQWPrbg = false;
      bool MkBEKotrbb = false;
      bool VTwsuMKcRF = false;
      bool hQyXiQxNhW = false;
      bool QMCrfIhVbi = false;
      bool zGSjWkIFHD = false;
      bool LlaVzwmcUs = false;
      bool qfJFLIQENl = false;
      bool enAhGbxNBx = false;
      bool HrFeKdlfri = false;
      bool VEAhqSwMYO = false;
      bool UErUYWlsPX = false;
      bool yZbXlzaeug = false;
      bool VjLdQZnRpn = false;
      bool ekolbDzjGm = false;
      bool rsQUkwcbwr = false;
      bool rRRqsjrNEy = false;
      bool iTCarNjpuh = false;
      string qNOYACRUUk;
      string upLhxfJUHH;
      string XpzCDhmbFJ;
      string lgynArlEqZ;
      string OlyUfHcizZ;
      string eWTfUANMUR;
      string OoOKyOqWhF;
      string tEdgrlreVw;
      string FEbfeONRTz;
      string dYiZLYWmVR;
      string PFAseZslmi;
      string IipYMKNifi;
      string AZDiaUUDVT;
      string OFIEfzDhAE;
      string VbdkquiJMA;
      string LnGmRpCRSK;
      string EoPTTDZlsN;
      string HehbNYuSrf;
      string lLqUHayVlp;
      string wkPpzfLJQm;
      if(qNOYACRUUk == PFAseZslmi){UwQzsfFeYs = true;}
      else if(PFAseZslmi == qNOYACRUUk){enAhGbxNBx = true;}
      if(upLhxfJUHH == IipYMKNifi){tjZmYsyuLr = true;}
      else if(IipYMKNifi == upLhxfJUHH){HrFeKdlfri = true;}
      if(XpzCDhmbFJ == AZDiaUUDVT){EEzDQWPrbg = true;}
      else if(AZDiaUUDVT == XpzCDhmbFJ){VEAhqSwMYO = true;}
      if(lgynArlEqZ == OFIEfzDhAE){MkBEKotrbb = true;}
      else if(OFIEfzDhAE == lgynArlEqZ){UErUYWlsPX = true;}
      if(OlyUfHcizZ == VbdkquiJMA){VTwsuMKcRF = true;}
      else if(VbdkquiJMA == OlyUfHcizZ){yZbXlzaeug = true;}
      if(eWTfUANMUR == LnGmRpCRSK){hQyXiQxNhW = true;}
      else if(LnGmRpCRSK == eWTfUANMUR){VjLdQZnRpn = true;}
      if(OoOKyOqWhF == EoPTTDZlsN){QMCrfIhVbi = true;}
      else if(EoPTTDZlsN == OoOKyOqWhF){ekolbDzjGm = true;}
      if(tEdgrlreVw == HehbNYuSrf){zGSjWkIFHD = true;}
      if(FEbfeONRTz == lLqUHayVlp){LlaVzwmcUs = true;}
      if(dYiZLYWmVR == wkPpzfLJQm){qfJFLIQENl = true;}
      while(HehbNYuSrf == tEdgrlreVw){rsQUkwcbwr = true;}
      while(lLqUHayVlp == lLqUHayVlp){rRRqsjrNEy = true;}
      while(wkPpzfLJQm == wkPpzfLJQm){iTCarNjpuh = true;}
      if(UwQzsfFeYs == true){UwQzsfFeYs = false;}
      if(tjZmYsyuLr == true){tjZmYsyuLr = false;}
      if(EEzDQWPrbg == true){EEzDQWPrbg = false;}
      if(MkBEKotrbb == true){MkBEKotrbb = false;}
      if(VTwsuMKcRF == true){VTwsuMKcRF = false;}
      if(hQyXiQxNhW == true){hQyXiQxNhW = false;}
      if(QMCrfIhVbi == true){QMCrfIhVbi = false;}
      if(zGSjWkIFHD == true){zGSjWkIFHD = false;}
      if(LlaVzwmcUs == true){LlaVzwmcUs = false;}
      if(qfJFLIQENl == true){qfJFLIQENl = false;}
      if(enAhGbxNBx == true){enAhGbxNBx = false;}
      if(HrFeKdlfri == true){HrFeKdlfri = false;}
      if(VEAhqSwMYO == true){VEAhqSwMYO = false;}
      if(UErUYWlsPX == true){UErUYWlsPX = false;}
      if(yZbXlzaeug == true){yZbXlzaeug = false;}
      if(VjLdQZnRpn == true){VjLdQZnRpn = false;}
      if(ekolbDzjGm == true){ekolbDzjGm = false;}
      if(rsQUkwcbwr == true){rsQUkwcbwr = false;}
      if(rRRqsjrNEy == true){rRRqsjrNEy = false;}
      if(iTCarNjpuh == true){iTCarNjpuh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HXGAFSCJKU
{ 
  void VYhHjgFFZD()
  { 
      bool yhZnOAUWOC = false;
      bool AGfTeZCwsH = false;
      bool QgZJxQXeag = false;
      bool LPTfChfAJw = false;
      bool MlWoNXdjyq = false;
      bool arbxgRIPqx = false;
      bool cougpkmARx = false;
      bool tBmwteKjwY = false;
      bool mnfjFTmVyk = false;
      bool ZPJMTXpozc = false;
      bool kdRBxYEfDB = false;
      bool GXTRKoILaW = false;
      bool xQBgXdracW = false;
      bool nUkTnIVelf = false;
      bool OHSxdBuqBe = false;
      bool OeNbeTckdS = false;
      bool gdCgMkmAdB = false;
      bool ScYtHpyECV = false;
      bool iAYxHJHInT = false;
      bool fjBTzjEkjr = false;
      string qHumXnZtOT;
      string hWfVuLqLLy;
      string ENQalYNLZo;
      string WcyRzjhUDL;
      string sfhBRGNhyo;
      string GFiVXdqUBz;
      string aOTZxEYNWL;
      string cWIzEXfOIf;
      string LeMkhRsbtU;
      string sMSDkeMHEN;
      string gzLmLjwjQA;
      string xbMVmuBiqf;
      string KwTBRQDFOZ;
      string LXJuyLTZYy;
      string UJLtXQAxPp;
      string cdPYCkfJUu;
      string fqUpoYRRWb;
      string abJsKAkYdB;
      string PGoYxgIkrN;
      string eJEdRdoUFy;
      if(qHumXnZtOT == gzLmLjwjQA){yhZnOAUWOC = true;}
      else if(gzLmLjwjQA == qHumXnZtOT){kdRBxYEfDB = true;}
      if(hWfVuLqLLy == xbMVmuBiqf){AGfTeZCwsH = true;}
      else if(xbMVmuBiqf == hWfVuLqLLy){GXTRKoILaW = true;}
      if(ENQalYNLZo == KwTBRQDFOZ){QgZJxQXeag = true;}
      else if(KwTBRQDFOZ == ENQalYNLZo){xQBgXdracW = true;}
      if(WcyRzjhUDL == LXJuyLTZYy){LPTfChfAJw = true;}
      else if(LXJuyLTZYy == WcyRzjhUDL){nUkTnIVelf = true;}
      if(sfhBRGNhyo == UJLtXQAxPp){MlWoNXdjyq = true;}
      else if(UJLtXQAxPp == sfhBRGNhyo){OHSxdBuqBe = true;}
      if(GFiVXdqUBz == cdPYCkfJUu){arbxgRIPqx = true;}
      else if(cdPYCkfJUu == GFiVXdqUBz){OeNbeTckdS = true;}
      if(aOTZxEYNWL == fqUpoYRRWb){cougpkmARx = true;}
      else if(fqUpoYRRWb == aOTZxEYNWL){gdCgMkmAdB = true;}
      if(cWIzEXfOIf == abJsKAkYdB){tBmwteKjwY = true;}
      if(LeMkhRsbtU == PGoYxgIkrN){mnfjFTmVyk = true;}
      if(sMSDkeMHEN == eJEdRdoUFy){ZPJMTXpozc = true;}
      while(abJsKAkYdB == cWIzEXfOIf){ScYtHpyECV = true;}
      while(PGoYxgIkrN == PGoYxgIkrN){iAYxHJHInT = true;}
      while(eJEdRdoUFy == eJEdRdoUFy){fjBTzjEkjr = true;}
      if(yhZnOAUWOC == true){yhZnOAUWOC = false;}
      if(AGfTeZCwsH == true){AGfTeZCwsH = false;}
      if(QgZJxQXeag == true){QgZJxQXeag = false;}
      if(LPTfChfAJw == true){LPTfChfAJw = false;}
      if(MlWoNXdjyq == true){MlWoNXdjyq = false;}
      if(arbxgRIPqx == true){arbxgRIPqx = false;}
      if(cougpkmARx == true){cougpkmARx = false;}
      if(tBmwteKjwY == true){tBmwteKjwY = false;}
      if(mnfjFTmVyk == true){mnfjFTmVyk = false;}
      if(ZPJMTXpozc == true){ZPJMTXpozc = false;}
      if(kdRBxYEfDB == true){kdRBxYEfDB = false;}
      if(GXTRKoILaW == true){GXTRKoILaW = false;}
      if(xQBgXdracW == true){xQBgXdracW = false;}
      if(nUkTnIVelf == true){nUkTnIVelf = false;}
      if(OHSxdBuqBe == true){OHSxdBuqBe = false;}
      if(OeNbeTckdS == true){OeNbeTckdS = false;}
      if(gdCgMkmAdB == true){gdCgMkmAdB = false;}
      if(ScYtHpyECV == true){ScYtHpyECV = false;}
      if(iAYxHJHInT == true){iAYxHJHInT = false;}
      if(fjBTzjEkjr == true){fjBTzjEkjr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VLLMNGZLUU
{ 
  void hFOePBkMsT()
  { 
      bool GXHMwKZtbK = false;
      bool hGYEPTYzPC = false;
      bool RPkwDAkfxO = false;
      bool MlOSNDAnCc = false;
      bool ciKBTkLHWn = false;
      bool GNKysplLIw = false;
      bool YsxNxuqGqM = false;
      bool SjwmYhFibS = false;
      bool ndIIHzbQYW = false;
      bool QnfGDilwEX = false;
      bool sFPJUVRWYL = false;
      bool jCjpHujnFy = false;
      bool TagTweEoio = false;
      bool uJeDYhNznl = false;
      bool gEqNmNEoyI = false;
      bool zypFERYwNp = false;
      bool QbnjQaXfQZ = false;
      bool hkzTRPjSgh = false;
      bool ZPqapwSZzG = false;
      bool JQkyUEQHxL = false;
      string ziiWnjNhrt;
      string ojRdItxJDF;
      string pNsTqYfoTe;
      string DMPqPKgggM;
      string QEzNzGSCAB;
      string GMVmkkpPfP;
      string EYOMqcUaOy;
      string NPbpnkOeTj;
      string FcnRYqqcFd;
      string UQBRIrplWW;
      string lqnCmpjeyA;
      string WLuuJajxmA;
      string lqUjcwOJgu;
      string ubIxxNoldi;
      string GLdFGJVFIh;
      string IryVkuWDYq;
      string bayDEuCyyu;
      string kySEtZMPdT;
      string aIBzmCdlMC;
      string qkMmHeMaAK;
      if(ziiWnjNhrt == lqnCmpjeyA){GXHMwKZtbK = true;}
      else if(lqnCmpjeyA == ziiWnjNhrt){sFPJUVRWYL = true;}
      if(ojRdItxJDF == WLuuJajxmA){hGYEPTYzPC = true;}
      else if(WLuuJajxmA == ojRdItxJDF){jCjpHujnFy = true;}
      if(pNsTqYfoTe == lqUjcwOJgu){RPkwDAkfxO = true;}
      else if(lqUjcwOJgu == pNsTqYfoTe){TagTweEoio = true;}
      if(DMPqPKgggM == ubIxxNoldi){MlOSNDAnCc = true;}
      else if(ubIxxNoldi == DMPqPKgggM){uJeDYhNznl = true;}
      if(QEzNzGSCAB == GLdFGJVFIh){ciKBTkLHWn = true;}
      else if(GLdFGJVFIh == QEzNzGSCAB){gEqNmNEoyI = true;}
      if(GMVmkkpPfP == IryVkuWDYq){GNKysplLIw = true;}
      else if(IryVkuWDYq == GMVmkkpPfP){zypFERYwNp = true;}
      if(EYOMqcUaOy == bayDEuCyyu){YsxNxuqGqM = true;}
      else if(bayDEuCyyu == EYOMqcUaOy){QbnjQaXfQZ = true;}
      if(NPbpnkOeTj == kySEtZMPdT){SjwmYhFibS = true;}
      if(FcnRYqqcFd == aIBzmCdlMC){ndIIHzbQYW = true;}
      if(UQBRIrplWW == qkMmHeMaAK){QnfGDilwEX = true;}
      while(kySEtZMPdT == NPbpnkOeTj){hkzTRPjSgh = true;}
      while(aIBzmCdlMC == aIBzmCdlMC){ZPqapwSZzG = true;}
      while(qkMmHeMaAK == qkMmHeMaAK){JQkyUEQHxL = true;}
      if(GXHMwKZtbK == true){GXHMwKZtbK = false;}
      if(hGYEPTYzPC == true){hGYEPTYzPC = false;}
      if(RPkwDAkfxO == true){RPkwDAkfxO = false;}
      if(MlOSNDAnCc == true){MlOSNDAnCc = false;}
      if(ciKBTkLHWn == true){ciKBTkLHWn = false;}
      if(GNKysplLIw == true){GNKysplLIw = false;}
      if(YsxNxuqGqM == true){YsxNxuqGqM = false;}
      if(SjwmYhFibS == true){SjwmYhFibS = false;}
      if(ndIIHzbQYW == true){ndIIHzbQYW = false;}
      if(QnfGDilwEX == true){QnfGDilwEX = false;}
      if(sFPJUVRWYL == true){sFPJUVRWYL = false;}
      if(jCjpHujnFy == true){jCjpHujnFy = false;}
      if(TagTweEoio == true){TagTweEoio = false;}
      if(uJeDYhNznl == true){uJeDYhNznl = false;}
      if(gEqNmNEoyI == true){gEqNmNEoyI = false;}
      if(zypFERYwNp == true){zypFERYwNp = false;}
      if(QbnjQaXfQZ == true){QbnjQaXfQZ = false;}
      if(hkzTRPjSgh == true){hkzTRPjSgh = false;}
      if(ZPqapwSZzG == true){ZPqapwSZzG = false;}
      if(JQkyUEQHxL == true){JQkyUEQHxL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LNGJWAYIFG
{ 
  void dNHqgUuRJn()
  { 
      bool CfnsiiidKK = false;
      bool NaBYLKUxJe = false;
      bool VWLQaxODGX = false;
      bool wbABSuExJn = false;
      bool CkXQLWGwiY = false;
      bool JkXkufTXRl = false;
      bool TfWlNlaAGQ = false;
      bool GXmVbjyuEE = false;
      bool qSTgOKEOGm = false;
      bool bgeXTWNkDY = false;
      bool wFpSSlyCIK = false;
      bool gYSNtBLoXC = false;
      bool gLtToZrPBF = false;
      bool WMrfIkcBOb = false;
      bool MCFuFCNRcR = false;
      bool LXNArjLVcr = false;
      bool thADnUgmEJ = false;
      bool YJawoeFyOl = false;
      bool LZmitzzqmX = false;
      bool YGXVheBCHj = false;
      string UORiQtnSMO;
      string ToqJFEjKZX;
      string wTHDCZtzTW;
      string OUbfURlxWV;
      string JOmFrJycLB;
      string LHZmQAGOfp;
      string znzozyMLnn;
      string SlnDNhgCKs;
      string bRmRLHunOU;
      string LXQfWVOxFs;
      string wmnXiYSWZi;
      string SDDmNhAgcq;
      string DYwTcuVHgr;
      string SixQxocPim;
      string oXOfBITdRn;
      string UgUNoXqUtn;
      string CKogPjFmoY;
      string FkypCeJPSU;
      string brprXkOiaY;
      string yIVujdigPQ;
      if(UORiQtnSMO == wmnXiYSWZi){CfnsiiidKK = true;}
      else if(wmnXiYSWZi == UORiQtnSMO){wFpSSlyCIK = true;}
      if(ToqJFEjKZX == SDDmNhAgcq){NaBYLKUxJe = true;}
      else if(SDDmNhAgcq == ToqJFEjKZX){gYSNtBLoXC = true;}
      if(wTHDCZtzTW == DYwTcuVHgr){VWLQaxODGX = true;}
      else if(DYwTcuVHgr == wTHDCZtzTW){gLtToZrPBF = true;}
      if(OUbfURlxWV == SixQxocPim){wbABSuExJn = true;}
      else if(SixQxocPim == OUbfURlxWV){WMrfIkcBOb = true;}
      if(JOmFrJycLB == oXOfBITdRn){CkXQLWGwiY = true;}
      else if(oXOfBITdRn == JOmFrJycLB){MCFuFCNRcR = true;}
      if(LHZmQAGOfp == UgUNoXqUtn){JkXkufTXRl = true;}
      else if(UgUNoXqUtn == LHZmQAGOfp){LXNArjLVcr = true;}
      if(znzozyMLnn == CKogPjFmoY){TfWlNlaAGQ = true;}
      else if(CKogPjFmoY == znzozyMLnn){thADnUgmEJ = true;}
      if(SlnDNhgCKs == FkypCeJPSU){GXmVbjyuEE = true;}
      if(bRmRLHunOU == brprXkOiaY){qSTgOKEOGm = true;}
      if(LXQfWVOxFs == yIVujdigPQ){bgeXTWNkDY = true;}
      while(FkypCeJPSU == SlnDNhgCKs){YJawoeFyOl = true;}
      while(brprXkOiaY == brprXkOiaY){LZmitzzqmX = true;}
      while(yIVujdigPQ == yIVujdigPQ){YGXVheBCHj = true;}
      if(CfnsiiidKK == true){CfnsiiidKK = false;}
      if(NaBYLKUxJe == true){NaBYLKUxJe = false;}
      if(VWLQaxODGX == true){VWLQaxODGX = false;}
      if(wbABSuExJn == true){wbABSuExJn = false;}
      if(CkXQLWGwiY == true){CkXQLWGwiY = false;}
      if(JkXkufTXRl == true){JkXkufTXRl = false;}
      if(TfWlNlaAGQ == true){TfWlNlaAGQ = false;}
      if(GXmVbjyuEE == true){GXmVbjyuEE = false;}
      if(qSTgOKEOGm == true){qSTgOKEOGm = false;}
      if(bgeXTWNkDY == true){bgeXTWNkDY = false;}
      if(wFpSSlyCIK == true){wFpSSlyCIK = false;}
      if(gYSNtBLoXC == true){gYSNtBLoXC = false;}
      if(gLtToZrPBF == true){gLtToZrPBF = false;}
      if(WMrfIkcBOb == true){WMrfIkcBOb = false;}
      if(MCFuFCNRcR == true){MCFuFCNRcR = false;}
      if(LXNArjLVcr == true){LXNArjLVcr = false;}
      if(thADnUgmEJ == true){thADnUgmEJ = false;}
      if(YJawoeFyOl == true){YJawoeFyOl = false;}
      if(LZmitzzqmX == true){LZmitzzqmX = false;}
      if(YGXVheBCHj == true){YGXVheBCHj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YYDTOFYGKF
{ 
  void RxKKaSZCnA()
  { 
      bool uwClzaaUCy = false;
      bool uuoYKKKJco = false;
      bool uhrBylgSuS = false;
      bool HRtCMqilEM = false;
      bool ZcQAqdTnQs = false;
      bool IJDoIMEAmN = false;
      bool tIYipcAmLS = false;
      bool aYFQPRRrJU = false;
      bool RrCCKyOEsA = false;
      bool mizqRREhJN = false;
      bool cgcxSjElZq = false;
      bool uMqRuCFVXq = false;
      bool OusLfrYNzy = false;
      bool MihwqkPFja = false;
      bool tbeCEargwP = false;
      bool LiJSyhmRQa = false;
      bool KpladLdtZJ = false;
      bool lAlxxmPpGF = false;
      bool ArQjmdZPxJ = false;
      bool yynIPDoEYs = false;
      string YVOYjaoYCD;
      string ChpSIBkLun;
      string YbarhHTXEe;
      string tzNKFAXADC;
      string YLpnlKwNsD;
      string IZFpaxuVqb;
      string GQUWJDTfMI;
      string dkIsPNklGW;
      string KGdAWTtXiI;
      string ECJauHpTei;
      string pQAkZXQMZN;
      string LIOTRAlUzo;
      string JwwEEbahyI;
      string WRHXfsFsZz;
      string ChxVxwmoJt;
      string UVToZtUwgy;
      string wiNQpsBpmX;
      string ustzmbyZGH;
      string nlXDmAxSyl;
      string YJMaDTTgeE;
      if(YVOYjaoYCD == pQAkZXQMZN){uwClzaaUCy = true;}
      else if(pQAkZXQMZN == YVOYjaoYCD){cgcxSjElZq = true;}
      if(ChpSIBkLun == LIOTRAlUzo){uuoYKKKJco = true;}
      else if(LIOTRAlUzo == ChpSIBkLun){uMqRuCFVXq = true;}
      if(YbarhHTXEe == JwwEEbahyI){uhrBylgSuS = true;}
      else if(JwwEEbahyI == YbarhHTXEe){OusLfrYNzy = true;}
      if(tzNKFAXADC == WRHXfsFsZz){HRtCMqilEM = true;}
      else if(WRHXfsFsZz == tzNKFAXADC){MihwqkPFja = true;}
      if(YLpnlKwNsD == ChxVxwmoJt){ZcQAqdTnQs = true;}
      else if(ChxVxwmoJt == YLpnlKwNsD){tbeCEargwP = true;}
      if(IZFpaxuVqb == UVToZtUwgy){IJDoIMEAmN = true;}
      else if(UVToZtUwgy == IZFpaxuVqb){LiJSyhmRQa = true;}
      if(GQUWJDTfMI == wiNQpsBpmX){tIYipcAmLS = true;}
      else if(wiNQpsBpmX == GQUWJDTfMI){KpladLdtZJ = true;}
      if(dkIsPNklGW == ustzmbyZGH){aYFQPRRrJU = true;}
      if(KGdAWTtXiI == nlXDmAxSyl){RrCCKyOEsA = true;}
      if(ECJauHpTei == YJMaDTTgeE){mizqRREhJN = true;}
      while(ustzmbyZGH == dkIsPNklGW){lAlxxmPpGF = true;}
      while(nlXDmAxSyl == nlXDmAxSyl){ArQjmdZPxJ = true;}
      while(YJMaDTTgeE == YJMaDTTgeE){yynIPDoEYs = true;}
      if(uwClzaaUCy == true){uwClzaaUCy = false;}
      if(uuoYKKKJco == true){uuoYKKKJco = false;}
      if(uhrBylgSuS == true){uhrBylgSuS = false;}
      if(HRtCMqilEM == true){HRtCMqilEM = false;}
      if(ZcQAqdTnQs == true){ZcQAqdTnQs = false;}
      if(IJDoIMEAmN == true){IJDoIMEAmN = false;}
      if(tIYipcAmLS == true){tIYipcAmLS = false;}
      if(aYFQPRRrJU == true){aYFQPRRrJU = false;}
      if(RrCCKyOEsA == true){RrCCKyOEsA = false;}
      if(mizqRREhJN == true){mizqRREhJN = false;}
      if(cgcxSjElZq == true){cgcxSjElZq = false;}
      if(uMqRuCFVXq == true){uMqRuCFVXq = false;}
      if(OusLfrYNzy == true){OusLfrYNzy = false;}
      if(MihwqkPFja == true){MihwqkPFja = false;}
      if(tbeCEargwP == true){tbeCEargwP = false;}
      if(LiJSyhmRQa == true){LiJSyhmRQa = false;}
      if(KpladLdtZJ == true){KpladLdtZJ = false;}
      if(lAlxxmPpGF == true){lAlxxmPpGF = false;}
      if(ArQjmdZPxJ == true){ArQjmdZPxJ = false;}
      if(yynIPDoEYs == true){yynIPDoEYs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OWMYWUESKB
{ 
  void IsgrBgnXVE()
  { 
      bool iWHginEyJH = false;
      bool JYUwVaSHIg = false;
      bool XVWrorgeGU = false;
      bool nfibIJzZak = false;
      bool tzwRFwgBuC = false;
      bool LKgOoNzOqp = false;
      bool GYqoSdpsnM = false;
      bool argXmYZFms = false;
      bool KeiDIdyUlt = false;
      bool JjhjdUHGya = false;
      bool GmMiaScJaQ = false;
      bool mtbxebpmfo = false;
      bool fUHsDqKxtR = false;
      bool QSGLmWSAcp = false;
      bool AnsUGPIjEC = false;
      bool eTaIukfTfi = false;
      bool HfPIAUmPKI = false;
      bool IoKooqbllm = false;
      bool harqmwrxQC = false;
      bool qyUsiTYjUl = false;
      string ASpBoyLGSF;
      string zaWENYefWt;
      string UrHSePrUVJ;
      string cWnGsLYpCt;
      string WCkkeLctsB;
      string sBhtyILWWY;
      string GpkJbcmPKC;
      string AocbWRGVnT;
      string mxbTPantIk;
      string ABSdngRbIB;
      string XMjpAEXFWf;
      string BoiDkJOQtB;
      string nrHYCyWznp;
      string AeNluZNkTK;
      string CiWbjKSPre;
      string RBgHMfEKnB;
      string hYQgfVbwHL;
      string udxiyFVeoq;
      string fGGJYyxFYP;
      string VCxRcdRpwN;
      if(ASpBoyLGSF == XMjpAEXFWf){iWHginEyJH = true;}
      else if(XMjpAEXFWf == ASpBoyLGSF){GmMiaScJaQ = true;}
      if(zaWENYefWt == BoiDkJOQtB){JYUwVaSHIg = true;}
      else if(BoiDkJOQtB == zaWENYefWt){mtbxebpmfo = true;}
      if(UrHSePrUVJ == nrHYCyWznp){XVWrorgeGU = true;}
      else if(nrHYCyWznp == UrHSePrUVJ){fUHsDqKxtR = true;}
      if(cWnGsLYpCt == AeNluZNkTK){nfibIJzZak = true;}
      else if(AeNluZNkTK == cWnGsLYpCt){QSGLmWSAcp = true;}
      if(WCkkeLctsB == CiWbjKSPre){tzwRFwgBuC = true;}
      else if(CiWbjKSPre == WCkkeLctsB){AnsUGPIjEC = true;}
      if(sBhtyILWWY == RBgHMfEKnB){LKgOoNzOqp = true;}
      else if(RBgHMfEKnB == sBhtyILWWY){eTaIukfTfi = true;}
      if(GpkJbcmPKC == hYQgfVbwHL){GYqoSdpsnM = true;}
      else if(hYQgfVbwHL == GpkJbcmPKC){HfPIAUmPKI = true;}
      if(AocbWRGVnT == udxiyFVeoq){argXmYZFms = true;}
      if(mxbTPantIk == fGGJYyxFYP){KeiDIdyUlt = true;}
      if(ABSdngRbIB == VCxRcdRpwN){JjhjdUHGya = true;}
      while(udxiyFVeoq == AocbWRGVnT){IoKooqbllm = true;}
      while(fGGJYyxFYP == fGGJYyxFYP){harqmwrxQC = true;}
      while(VCxRcdRpwN == VCxRcdRpwN){qyUsiTYjUl = true;}
      if(iWHginEyJH == true){iWHginEyJH = false;}
      if(JYUwVaSHIg == true){JYUwVaSHIg = false;}
      if(XVWrorgeGU == true){XVWrorgeGU = false;}
      if(nfibIJzZak == true){nfibIJzZak = false;}
      if(tzwRFwgBuC == true){tzwRFwgBuC = false;}
      if(LKgOoNzOqp == true){LKgOoNzOqp = false;}
      if(GYqoSdpsnM == true){GYqoSdpsnM = false;}
      if(argXmYZFms == true){argXmYZFms = false;}
      if(KeiDIdyUlt == true){KeiDIdyUlt = false;}
      if(JjhjdUHGya == true){JjhjdUHGya = false;}
      if(GmMiaScJaQ == true){GmMiaScJaQ = false;}
      if(mtbxebpmfo == true){mtbxebpmfo = false;}
      if(fUHsDqKxtR == true){fUHsDqKxtR = false;}
      if(QSGLmWSAcp == true){QSGLmWSAcp = false;}
      if(AnsUGPIjEC == true){AnsUGPIjEC = false;}
      if(eTaIukfTfi == true){eTaIukfTfi = false;}
      if(HfPIAUmPKI == true){HfPIAUmPKI = false;}
      if(IoKooqbllm == true){IoKooqbllm = false;}
      if(harqmwrxQC == true){harqmwrxQC = false;}
      if(qyUsiTYjUl == true){qyUsiTYjUl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZVUEVALGOO
{ 
  void GgiuNRANmx()
  { 
      bool oLKZtmDmFm = false;
      bool uVDzuNHynu = false;
      bool gcrbfXrOQV = false;
      bool tlunLkXUJZ = false;
      bool awHmFqRqoY = false;
      bool KaGDLMThtW = false;
      bool toWisIaOnK = false;
      bool omoOUkVFVo = false;
      bool GBYqKTeqcz = false;
      bool RpZlekYXsx = false;
      bool PCDeiWkria = false;
      bool pOLEPrPeYZ = false;
      bool cilnszMDii = false;
      bool LyLYYDytIm = false;
      bool hmICICdUVl = false;
      bool VtyTTgQOki = false;
      bool JhJAOqCmfM = false;
      bool tUWkqpajep = false;
      bool VrgwMRAqEr = false;
      bool miIYdAcHmR = false;
      string TGslgUQGhB;
      string JzGijaHdng;
      string aYEHtxsOcq;
      string GqsFtcSnZM;
      string zKUwVKjOod;
      string nUVPYuClqj;
      string bBcptRGAoH;
      string ITDbgPEHki;
      string MdZzDkVdgz;
      string afwpjyhdCD;
      string EiZhUTQsiI;
      string puAIWxbdmI;
      string wbSogbWNqi;
      string hZFhdcErdi;
      string UbuYIeenTm;
      string WDqsHcQjBs;
      string wCCAZQoSds;
      string rizDnTgtgB;
      string xitDMeiMaB;
      string MUqkmSZtyr;
      if(TGslgUQGhB == EiZhUTQsiI){oLKZtmDmFm = true;}
      else if(EiZhUTQsiI == TGslgUQGhB){PCDeiWkria = true;}
      if(JzGijaHdng == puAIWxbdmI){uVDzuNHynu = true;}
      else if(puAIWxbdmI == JzGijaHdng){pOLEPrPeYZ = true;}
      if(aYEHtxsOcq == wbSogbWNqi){gcrbfXrOQV = true;}
      else if(wbSogbWNqi == aYEHtxsOcq){cilnszMDii = true;}
      if(GqsFtcSnZM == hZFhdcErdi){tlunLkXUJZ = true;}
      else if(hZFhdcErdi == GqsFtcSnZM){LyLYYDytIm = true;}
      if(zKUwVKjOod == UbuYIeenTm){awHmFqRqoY = true;}
      else if(UbuYIeenTm == zKUwVKjOod){hmICICdUVl = true;}
      if(nUVPYuClqj == WDqsHcQjBs){KaGDLMThtW = true;}
      else if(WDqsHcQjBs == nUVPYuClqj){VtyTTgQOki = true;}
      if(bBcptRGAoH == wCCAZQoSds){toWisIaOnK = true;}
      else if(wCCAZQoSds == bBcptRGAoH){JhJAOqCmfM = true;}
      if(ITDbgPEHki == rizDnTgtgB){omoOUkVFVo = true;}
      if(MdZzDkVdgz == xitDMeiMaB){GBYqKTeqcz = true;}
      if(afwpjyhdCD == MUqkmSZtyr){RpZlekYXsx = true;}
      while(rizDnTgtgB == ITDbgPEHki){tUWkqpajep = true;}
      while(xitDMeiMaB == xitDMeiMaB){VrgwMRAqEr = true;}
      while(MUqkmSZtyr == MUqkmSZtyr){miIYdAcHmR = true;}
      if(oLKZtmDmFm == true){oLKZtmDmFm = false;}
      if(uVDzuNHynu == true){uVDzuNHynu = false;}
      if(gcrbfXrOQV == true){gcrbfXrOQV = false;}
      if(tlunLkXUJZ == true){tlunLkXUJZ = false;}
      if(awHmFqRqoY == true){awHmFqRqoY = false;}
      if(KaGDLMThtW == true){KaGDLMThtW = false;}
      if(toWisIaOnK == true){toWisIaOnK = false;}
      if(omoOUkVFVo == true){omoOUkVFVo = false;}
      if(GBYqKTeqcz == true){GBYqKTeqcz = false;}
      if(RpZlekYXsx == true){RpZlekYXsx = false;}
      if(PCDeiWkria == true){PCDeiWkria = false;}
      if(pOLEPrPeYZ == true){pOLEPrPeYZ = false;}
      if(cilnszMDii == true){cilnszMDii = false;}
      if(LyLYYDytIm == true){LyLYYDytIm = false;}
      if(hmICICdUVl == true){hmICICdUVl = false;}
      if(VtyTTgQOki == true){VtyTTgQOki = false;}
      if(JhJAOqCmfM == true){JhJAOqCmfM = false;}
      if(tUWkqpajep == true){tUWkqpajep = false;}
      if(VrgwMRAqEr == true){VrgwMRAqEr = false;}
      if(miIYdAcHmR == true){miIYdAcHmR = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VSLCTJZMWB
{ 
  void TJecRBXoVO()
  { 
      bool qwYYbRIilH = false;
      bool GdqzKqjEaQ = false;
      bool VVBVodeOTu = false;
      bool rnAGZTyJDQ = false;
      bool TasCJhgiZu = false;
      bool bLUsHUuCjx = false;
      bool ltOpOSdkZp = false;
      bool rZBJwucyMG = false;
      bool xwnXSgTGyE = false;
      bool YwZFeYLCmq = false;
      bool EuJjDCKKxL = false;
      bool qwpcAoFpWh = false;
      bool yWlURwzueW = false;
      bool iZLUCBmEUQ = false;
      bool WJJmMbFxEo = false;
      bool TUfErgInRH = false;
      bool DOeZcQkUZz = false;
      bool CAVGLoMzIo = false;
      bool pBppDNtHcU = false;
      bool rzlMNHdjKu = false;
      string oKRORsNBtY;
      string eSNEYAZgpl;
      string qIfegHWaek;
      string cbDPVwglpB;
      string enIbdtBCCT;
      string lThcgfdmgK;
      string VlVIuDOxhb;
      string SnpnGCAEBm;
      string GTYLzIsaho;
      string StFfesRzfZ;
      string eDTZAxdsVe;
      string fZltEBNJJl;
      string JnOsJYiKIN;
      string kAXaHCzzLB;
      string eAEDJrHKad;
      string eHpsQqoaQa;
      string OSjIsfBKlC;
      string CNyHHIjKjQ;
      string QXiehcJeJQ;
      string MXhjmLQuKc;
      if(oKRORsNBtY == eDTZAxdsVe){qwYYbRIilH = true;}
      else if(eDTZAxdsVe == oKRORsNBtY){EuJjDCKKxL = true;}
      if(eSNEYAZgpl == fZltEBNJJl){GdqzKqjEaQ = true;}
      else if(fZltEBNJJl == eSNEYAZgpl){qwpcAoFpWh = true;}
      if(qIfegHWaek == JnOsJYiKIN){VVBVodeOTu = true;}
      else if(JnOsJYiKIN == qIfegHWaek){yWlURwzueW = true;}
      if(cbDPVwglpB == kAXaHCzzLB){rnAGZTyJDQ = true;}
      else if(kAXaHCzzLB == cbDPVwglpB){iZLUCBmEUQ = true;}
      if(enIbdtBCCT == eAEDJrHKad){TasCJhgiZu = true;}
      else if(eAEDJrHKad == enIbdtBCCT){WJJmMbFxEo = true;}
      if(lThcgfdmgK == eHpsQqoaQa){bLUsHUuCjx = true;}
      else if(eHpsQqoaQa == lThcgfdmgK){TUfErgInRH = true;}
      if(VlVIuDOxhb == OSjIsfBKlC){ltOpOSdkZp = true;}
      else if(OSjIsfBKlC == VlVIuDOxhb){DOeZcQkUZz = true;}
      if(SnpnGCAEBm == CNyHHIjKjQ){rZBJwucyMG = true;}
      if(GTYLzIsaho == QXiehcJeJQ){xwnXSgTGyE = true;}
      if(StFfesRzfZ == MXhjmLQuKc){YwZFeYLCmq = true;}
      while(CNyHHIjKjQ == SnpnGCAEBm){CAVGLoMzIo = true;}
      while(QXiehcJeJQ == QXiehcJeJQ){pBppDNtHcU = true;}
      while(MXhjmLQuKc == MXhjmLQuKc){rzlMNHdjKu = true;}
      if(qwYYbRIilH == true){qwYYbRIilH = false;}
      if(GdqzKqjEaQ == true){GdqzKqjEaQ = false;}
      if(VVBVodeOTu == true){VVBVodeOTu = false;}
      if(rnAGZTyJDQ == true){rnAGZTyJDQ = false;}
      if(TasCJhgiZu == true){TasCJhgiZu = false;}
      if(bLUsHUuCjx == true){bLUsHUuCjx = false;}
      if(ltOpOSdkZp == true){ltOpOSdkZp = false;}
      if(rZBJwucyMG == true){rZBJwucyMG = false;}
      if(xwnXSgTGyE == true){xwnXSgTGyE = false;}
      if(YwZFeYLCmq == true){YwZFeYLCmq = false;}
      if(EuJjDCKKxL == true){EuJjDCKKxL = false;}
      if(qwpcAoFpWh == true){qwpcAoFpWh = false;}
      if(yWlURwzueW == true){yWlURwzueW = false;}
      if(iZLUCBmEUQ == true){iZLUCBmEUQ = false;}
      if(WJJmMbFxEo == true){WJJmMbFxEo = false;}
      if(TUfErgInRH == true){TUfErgInRH = false;}
      if(DOeZcQkUZz == true){DOeZcQkUZz = false;}
      if(CAVGLoMzIo == true){CAVGLoMzIo = false;}
      if(pBppDNtHcU == true){pBppDNtHcU = false;}
      if(rzlMNHdjKu == true){rzlMNHdjKu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NEMAROWORD
{ 
  void XoHcOeubWV()
  { 
      bool VFDNwdZSpF = false;
      bool zUTtVCCSXg = false;
      bool uITZlZojZd = false;
      bool bQdeISgLPl = false;
      bool pxweYOpjlq = false;
      bool CLamRNkdaW = false;
      bool opsMchpolD = false;
      bool UpDkpyLoBW = false;
      bool jExhaQpqMt = false;
      bool hDjsxzDjCp = false;
      bool YByiWLIowT = false;
      bool BwcEmjrLkM = false;
      bool rybqVRGJmp = false;
      bool ubmKxiGOLP = false;
      bool HAusskafGN = false;
      bool HryoyaYGNF = false;
      bool FyCGYYJRGE = false;
      bool ddzUFqDiyj = false;
      bool pZDZiCcxMW = false;
      bool JzBFKJyoUT = false;
      string sZdTnBOlWy;
      string weqDxgOejJ;
      string pmTDXfpPHN;
      string QticuclkOA;
      string WrwUdPwnoZ;
      string FzJqeGFZuI;
      string KzdIgBwNIT;
      string udPimypzYI;
      string jCZhIRjryo;
      string XLpGoXOmnc;
      string TheVdajZEg;
      string PGBmXHYwAP;
      string QDPSwinItP;
      string iDKwnCWEDg;
      string gKnRZRKCig;
      string ySmYLPSMYu;
      string mbfoxLuEyV;
      string jUrThGPyre;
      string YkYMQQQljY;
      string iKeIsEeyTN;
      if(sZdTnBOlWy == TheVdajZEg){VFDNwdZSpF = true;}
      else if(TheVdajZEg == sZdTnBOlWy){YByiWLIowT = true;}
      if(weqDxgOejJ == PGBmXHYwAP){zUTtVCCSXg = true;}
      else if(PGBmXHYwAP == weqDxgOejJ){BwcEmjrLkM = true;}
      if(pmTDXfpPHN == QDPSwinItP){uITZlZojZd = true;}
      else if(QDPSwinItP == pmTDXfpPHN){rybqVRGJmp = true;}
      if(QticuclkOA == iDKwnCWEDg){bQdeISgLPl = true;}
      else if(iDKwnCWEDg == QticuclkOA){ubmKxiGOLP = true;}
      if(WrwUdPwnoZ == gKnRZRKCig){pxweYOpjlq = true;}
      else if(gKnRZRKCig == WrwUdPwnoZ){HAusskafGN = true;}
      if(FzJqeGFZuI == ySmYLPSMYu){CLamRNkdaW = true;}
      else if(ySmYLPSMYu == FzJqeGFZuI){HryoyaYGNF = true;}
      if(KzdIgBwNIT == mbfoxLuEyV){opsMchpolD = true;}
      else if(mbfoxLuEyV == KzdIgBwNIT){FyCGYYJRGE = true;}
      if(udPimypzYI == jUrThGPyre){UpDkpyLoBW = true;}
      if(jCZhIRjryo == YkYMQQQljY){jExhaQpqMt = true;}
      if(XLpGoXOmnc == iKeIsEeyTN){hDjsxzDjCp = true;}
      while(jUrThGPyre == udPimypzYI){ddzUFqDiyj = true;}
      while(YkYMQQQljY == YkYMQQQljY){pZDZiCcxMW = true;}
      while(iKeIsEeyTN == iKeIsEeyTN){JzBFKJyoUT = true;}
      if(VFDNwdZSpF == true){VFDNwdZSpF = false;}
      if(zUTtVCCSXg == true){zUTtVCCSXg = false;}
      if(uITZlZojZd == true){uITZlZojZd = false;}
      if(bQdeISgLPl == true){bQdeISgLPl = false;}
      if(pxweYOpjlq == true){pxweYOpjlq = false;}
      if(CLamRNkdaW == true){CLamRNkdaW = false;}
      if(opsMchpolD == true){opsMchpolD = false;}
      if(UpDkpyLoBW == true){UpDkpyLoBW = false;}
      if(jExhaQpqMt == true){jExhaQpqMt = false;}
      if(hDjsxzDjCp == true){hDjsxzDjCp = false;}
      if(YByiWLIowT == true){YByiWLIowT = false;}
      if(BwcEmjrLkM == true){BwcEmjrLkM = false;}
      if(rybqVRGJmp == true){rybqVRGJmp = false;}
      if(ubmKxiGOLP == true){ubmKxiGOLP = false;}
      if(HAusskafGN == true){HAusskafGN = false;}
      if(HryoyaYGNF == true){HryoyaYGNF = false;}
      if(FyCGYYJRGE == true){FyCGYYJRGE = false;}
      if(ddzUFqDiyj == true){ddzUFqDiyj = false;}
      if(pZDZiCcxMW == true){pZDZiCcxMW = false;}
      if(JzBFKJyoUT == true){JzBFKJyoUT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EUXQXUDPMW
{ 
  void hgMESoKQbN()
  { 
      bool NqXqrngOPK = false;
      bool hMJmrNljVm = false;
      bool EgQEzJZHxV = false;
      bool ozfVZCiGVT = false;
      bool sRLJwPAeKU = false;
      bool yOZgNgZSri = false;
      bool NZQtUifnMo = false;
      bool deNCsmUHHi = false;
      bool IuxbXZYzHs = false;
      bool cPNmFSARoM = false;
      bool iMRUpSAXma = false;
      bool LSLIaScoVt = false;
      bool KgfTMORawZ = false;
      bool lCRuzucbkE = false;
      bool YALghJEELl = false;
      bool pLutCCfyHb = false;
      bool BZaMoJAPJe = false;
      bool pADRUJUzkC = false;
      bool JMPKjaGXrw = false;
      bool OdUJDzYzrX = false;
      string NuGOWXYAwY;
      string ekwjVpCljx;
      string qzqzSXwGGc;
      string aSWBchmrjR;
      string miwuwMnwZK;
      string rEsdnODHwz;
      string bStTxLOuGn;
      string CJCRJHCwDF;
      string izfwkWZsNO;
      string dfwjpUocxj;
      string ysplHhCaTf;
      string LHDpQcmyia;
      string lUgrKVNRbo;
      string SZySHoItnD;
      string UyZgDWbGTM;
      string gqtWycMyal;
      string UjzdNkIkib;
      string TsJeGnuznn;
      string sGyOHafbTd;
      string bKSChjywTS;
      if(NuGOWXYAwY == ysplHhCaTf){NqXqrngOPK = true;}
      else if(ysplHhCaTf == NuGOWXYAwY){iMRUpSAXma = true;}
      if(ekwjVpCljx == LHDpQcmyia){hMJmrNljVm = true;}
      else if(LHDpQcmyia == ekwjVpCljx){LSLIaScoVt = true;}
      if(qzqzSXwGGc == lUgrKVNRbo){EgQEzJZHxV = true;}
      else if(lUgrKVNRbo == qzqzSXwGGc){KgfTMORawZ = true;}
      if(aSWBchmrjR == SZySHoItnD){ozfVZCiGVT = true;}
      else if(SZySHoItnD == aSWBchmrjR){lCRuzucbkE = true;}
      if(miwuwMnwZK == UyZgDWbGTM){sRLJwPAeKU = true;}
      else if(UyZgDWbGTM == miwuwMnwZK){YALghJEELl = true;}
      if(rEsdnODHwz == gqtWycMyal){yOZgNgZSri = true;}
      else if(gqtWycMyal == rEsdnODHwz){pLutCCfyHb = true;}
      if(bStTxLOuGn == UjzdNkIkib){NZQtUifnMo = true;}
      else if(UjzdNkIkib == bStTxLOuGn){BZaMoJAPJe = true;}
      if(CJCRJHCwDF == TsJeGnuznn){deNCsmUHHi = true;}
      if(izfwkWZsNO == sGyOHafbTd){IuxbXZYzHs = true;}
      if(dfwjpUocxj == bKSChjywTS){cPNmFSARoM = true;}
      while(TsJeGnuznn == CJCRJHCwDF){pADRUJUzkC = true;}
      while(sGyOHafbTd == sGyOHafbTd){JMPKjaGXrw = true;}
      while(bKSChjywTS == bKSChjywTS){OdUJDzYzrX = true;}
      if(NqXqrngOPK == true){NqXqrngOPK = false;}
      if(hMJmrNljVm == true){hMJmrNljVm = false;}
      if(EgQEzJZHxV == true){EgQEzJZHxV = false;}
      if(ozfVZCiGVT == true){ozfVZCiGVT = false;}
      if(sRLJwPAeKU == true){sRLJwPAeKU = false;}
      if(yOZgNgZSri == true){yOZgNgZSri = false;}
      if(NZQtUifnMo == true){NZQtUifnMo = false;}
      if(deNCsmUHHi == true){deNCsmUHHi = false;}
      if(IuxbXZYzHs == true){IuxbXZYzHs = false;}
      if(cPNmFSARoM == true){cPNmFSARoM = false;}
      if(iMRUpSAXma == true){iMRUpSAXma = false;}
      if(LSLIaScoVt == true){LSLIaScoVt = false;}
      if(KgfTMORawZ == true){KgfTMORawZ = false;}
      if(lCRuzucbkE == true){lCRuzucbkE = false;}
      if(YALghJEELl == true){YALghJEELl = false;}
      if(pLutCCfyHb == true){pLutCCfyHb = false;}
      if(BZaMoJAPJe == true){BZaMoJAPJe = false;}
      if(pADRUJUzkC == true){pADRUJUzkC = false;}
      if(JMPKjaGXrw == true){JMPKjaGXrw = false;}
      if(OdUJDzYzrX == true){OdUJDzYzrX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VKPOPGEQGD
{ 
  void YQjemxReBA()
  { 
      bool HwxIgDinwd = false;
      bool IGuoMFoBtU = false;
      bool hRpTKrBOGN = false;
      bool lZKgRFcVNe = false;
      bool KFmPoTpfjY = false;
      bool ptSyOoFdxn = false;
      bool BThiqhwOWH = false;
      bool cSmOKbdadq = false;
      bool CVdhWrGLSb = false;
      bool zyXZdpXEXG = false;
      bool WbIbuWGDnl = false;
      bool LeAGlfozLz = false;
      bool akutIduNkT = false;
      bool EYCWTaMClA = false;
      bool lDXgQTyQmy = false;
      bool AXsYelfNTR = false;
      bool brRTglcdmL = false;
      bool SmNLMXMOdn = false;
      bool edamSgfcqh = false;
      bool tzhTXskDze = false;
      string LIujAIpEue;
      string jMpOmXYDnL;
      string LgZHDyfATT;
      string tLEtwSNHqe;
      string VDuSMhqsRI;
      string mDNaKehmgC;
      string cojbjYHuST;
      string JQLJoHkHuU;
      string GlhLgqxcjQ;
      string jZanCgTThn;
      string auayyVqElC;
      string sarLupGYus;
      string aeLBijpWHI;
      string nRYGtKLjMK;
      string zsqhXJhOsL;
      string ZtYoMwjYOO;
      string fTxcOaBkRk;
      string eWLjshmddZ;
      string Baduissjty;
      string ZlUxdfuNTw;
      if(LIujAIpEue == auayyVqElC){HwxIgDinwd = true;}
      else if(auayyVqElC == LIujAIpEue){WbIbuWGDnl = true;}
      if(jMpOmXYDnL == sarLupGYus){IGuoMFoBtU = true;}
      else if(sarLupGYus == jMpOmXYDnL){LeAGlfozLz = true;}
      if(LgZHDyfATT == aeLBijpWHI){hRpTKrBOGN = true;}
      else if(aeLBijpWHI == LgZHDyfATT){akutIduNkT = true;}
      if(tLEtwSNHqe == nRYGtKLjMK){lZKgRFcVNe = true;}
      else if(nRYGtKLjMK == tLEtwSNHqe){EYCWTaMClA = true;}
      if(VDuSMhqsRI == zsqhXJhOsL){KFmPoTpfjY = true;}
      else if(zsqhXJhOsL == VDuSMhqsRI){lDXgQTyQmy = true;}
      if(mDNaKehmgC == ZtYoMwjYOO){ptSyOoFdxn = true;}
      else if(ZtYoMwjYOO == mDNaKehmgC){AXsYelfNTR = true;}
      if(cojbjYHuST == fTxcOaBkRk){BThiqhwOWH = true;}
      else if(fTxcOaBkRk == cojbjYHuST){brRTglcdmL = true;}
      if(JQLJoHkHuU == eWLjshmddZ){cSmOKbdadq = true;}
      if(GlhLgqxcjQ == Baduissjty){CVdhWrGLSb = true;}
      if(jZanCgTThn == ZlUxdfuNTw){zyXZdpXEXG = true;}
      while(eWLjshmddZ == JQLJoHkHuU){SmNLMXMOdn = true;}
      while(Baduissjty == Baduissjty){edamSgfcqh = true;}
      while(ZlUxdfuNTw == ZlUxdfuNTw){tzhTXskDze = true;}
      if(HwxIgDinwd == true){HwxIgDinwd = false;}
      if(IGuoMFoBtU == true){IGuoMFoBtU = false;}
      if(hRpTKrBOGN == true){hRpTKrBOGN = false;}
      if(lZKgRFcVNe == true){lZKgRFcVNe = false;}
      if(KFmPoTpfjY == true){KFmPoTpfjY = false;}
      if(ptSyOoFdxn == true){ptSyOoFdxn = false;}
      if(BThiqhwOWH == true){BThiqhwOWH = false;}
      if(cSmOKbdadq == true){cSmOKbdadq = false;}
      if(CVdhWrGLSb == true){CVdhWrGLSb = false;}
      if(zyXZdpXEXG == true){zyXZdpXEXG = false;}
      if(WbIbuWGDnl == true){WbIbuWGDnl = false;}
      if(LeAGlfozLz == true){LeAGlfozLz = false;}
      if(akutIduNkT == true){akutIduNkT = false;}
      if(EYCWTaMClA == true){EYCWTaMClA = false;}
      if(lDXgQTyQmy == true){lDXgQTyQmy = false;}
      if(AXsYelfNTR == true){AXsYelfNTR = false;}
      if(brRTglcdmL == true){brRTglcdmL = false;}
      if(SmNLMXMOdn == true){SmNLMXMOdn = false;}
      if(edamSgfcqh == true){edamSgfcqh = false;}
      if(tzhTXskDze == true){tzhTXskDze = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WQAEXAHBDY
{ 
  void VWPBXyPSjY()
  { 
      bool DuBaBRSEgi = false;
      bool YNyUZVFCzr = false;
      bool mxeiUQykMH = false;
      bool XyKQhtVeJq = false;
      bool fLINdfHRGc = false;
      bool UwOYDGPRGj = false;
      bool TgqYKaChmf = false;
      bool AZsHLbnQFo = false;
      bool JfejlailMa = false;
      bool dDYZWzTjDC = false;
      bool cZdZbtDGMa = false;
      bool RzMQZnBGmj = false;
      bool qQULBxBTkM = false;
      bool HENCLjBEIr = false;
      bool WcYlJeqHzh = false;
      bool xNxSLMFSYe = false;
      bool rjVzSMlJFp = false;
      bool mxxuMOqhCW = false;
      bool GtTCPtHtqy = false;
      bool eifdzmGYSw = false;
      string zDNsPzfJRd;
      string tkOnXwWYul;
      string XQZkHGAXTT;
      string WyJsKMfzsF;
      string qoRnoxwitt;
      string ZimQKhcbkI;
      string zIoxGwbUcL;
      string QXeouizkwK;
      string KQcZtnKrib;
      string iDNkGhzTgr;
      string VaFicCDxVa;
      string iqfaxheODS;
      string kikAcMJsiM;
      string ifEDoiKtwq;
      string jNWIsqWmnX;
      string DkPOGlpFGP;
      string rKNIqCmblN;
      string WkRLhXMcGk;
      string oWFKJyPpxC;
      string sFmgyZzNRp;
      if(zDNsPzfJRd == VaFicCDxVa){DuBaBRSEgi = true;}
      else if(VaFicCDxVa == zDNsPzfJRd){cZdZbtDGMa = true;}
      if(tkOnXwWYul == iqfaxheODS){YNyUZVFCzr = true;}
      else if(iqfaxheODS == tkOnXwWYul){RzMQZnBGmj = true;}
      if(XQZkHGAXTT == kikAcMJsiM){mxeiUQykMH = true;}
      else if(kikAcMJsiM == XQZkHGAXTT){qQULBxBTkM = true;}
      if(WyJsKMfzsF == ifEDoiKtwq){XyKQhtVeJq = true;}
      else if(ifEDoiKtwq == WyJsKMfzsF){HENCLjBEIr = true;}
      if(qoRnoxwitt == jNWIsqWmnX){fLINdfHRGc = true;}
      else if(jNWIsqWmnX == qoRnoxwitt){WcYlJeqHzh = true;}
      if(ZimQKhcbkI == DkPOGlpFGP){UwOYDGPRGj = true;}
      else if(DkPOGlpFGP == ZimQKhcbkI){xNxSLMFSYe = true;}
      if(zIoxGwbUcL == rKNIqCmblN){TgqYKaChmf = true;}
      else if(rKNIqCmblN == zIoxGwbUcL){rjVzSMlJFp = true;}
      if(QXeouizkwK == WkRLhXMcGk){AZsHLbnQFo = true;}
      if(KQcZtnKrib == oWFKJyPpxC){JfejlailMa = true;}
      if(iDNkGhzTgr == sFmgyZzNRp){dDYZWzTjDC = true;}
      while(WkRLhXMcGk == QXeouizkwK){mxxuMOqhCW = true;}
      while(oWFKJyPpxC == oWFKJyPpxC){GtTCPtHtqy = true;}
      while(sFmgyZzNRp == sFmgyZzNRp){eifdzmGYSw = true;}
      if(DuBaBRSEgi == true){DuBaBRSEgi = false;}
      if(YNyUZVFCzr == true){YNyUZVFCzr = false;}
      if(mxeiUQykMH == true){mxeiUQykMH = false;}
      if(XyKQhtVeJq == true){XyKQhtVeJq = false;}
      if(fLINdfHRGc == true){fLINdfHRGc = false;}
      if(UwOYDGPRGj == true){UwOYDGPRGj = false;}
      if(TgqYKaChmf == true){TgqYKaChmf = false;}
      if(AZsHLbnQFo == true){AZsHLbnQFo = false;}
      if(JfejlailMa == true){JfejlailMa = false;}
      if(dDYZWzTjDC == true){dDYZWzTjDC = false;}
      if(cZdZbtDGMa == true){cZdZbtDGMa = false;}
      if(RzMQZnBGmj == true){RzMQZnBGmj = false;}
      if(qQULBxBTkM == true){qQULBxBTkM = false;}
      if(HENCLjBEIr == true){HENCLjBEIr = false;}
      if(WcYlJeqHzh == true){WcYlJeqHzh = false;}
      if(xNxSLMFSYe == true){xNxSLMFSYe = false;}
      if(rjVzSMlJFp == true){rjVzSMlJFp = false;}
      if(mxxuMOqhCW == true){mxxuMOqhCW = false;}
      if(GtTCPtHtqy == true){GtTCPtHtqy = false;}
      if(eifdzmGYSw == true){eifdzmGYSw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PKWSAZJPJU
{ 
  void aGzmuuZNSj()
  { 
      bool UPOndwmreX = false;
      bool qdiyJXtHFu = false;
      bool woqGAOZlUV = false;
      bool YmAmVYyGdK = false;
      bool PQJlTGwLOQ = false;
      bool BRssLfYInP = false;
      bool WBWIzEcwck = false;
      bool YndueefEls = false;
      bool RNWDUGtGap = false;
      bool zpSOmOtAmP = false;
      bool cAidrNnyOf = false;
      bool ATVtxIdSnr = false;
      bool KWUlkhUPHP = false;
      bool keRWPRdsLH = false;
      bool RuqupfUXQe = false;
      bool kaLtEGewrJ = false;
      bool rYpytUByde = false;
      bool dkHgZOeYwE = false;
      bool NxyceJSXRJ = false;
      bool mhACOsFsAU = false;
      string xsJRrXknZh;
      string aPmicZAdjj;
      string ZgnjtGXUee;
      string jwKXuVWNxa;
      string gphzqeJanF;
      string UpGBjPFZXk;
      string zQIuyeFPnH;
      string CmgICiznrO;
      string xwgZJtoIme;
      string QxpMaPgmBM;
      string ZtmfChJyyA;
      string QQgImDjikD;
      string pwbIpEwTgl;
      string CLmfoCJAWa;
      string kqKmNzQhYX;
      string fyCxWcSdpy;
      string loQLyFdASk;
      string sjLKSykouq;
      string nNtIGDIoJL;
      string jeKYowZynI;
      if(xsJRrXknZh == ZtmfChJyyA){UPOndwmreX = true;}
      else if(ZtmfChJyyA == xsJRrXknZh){cAidrNnyOf = true;}
      if(aPmicZAdjj == QQgImDjikD){qdiyJXtHFu = true;}
      else if(QQgImDjikD == aPmicZAdjj){ATVtxIdSnr = true;}
      if(ZgnjtGXUee == pwbIpEwTgl){woqGAOZlUV = true;}
      else if(pwbIpEwTgl == ZgnjtGXUee){KWUlkhUPHP = true;}
      if(jwKXuVWNxa == CLmfoCJAWa){YmAmVYyGdK = true;}
      else if(CLmfoCJAWa == jwKXuVWNxa){keRWPRdsLH = true;}
      if(gphzqeJanF == kqKmNzQhYX){PQJlTGwLOQ = true;}
      else if(kqKmNzQhYX == gphzqeJanF){RuqupfUXQe = true;}
      if(UpGBjPFZXk == fyCxWcSdpy){BRssLfYInP = true;}
      else if(fyCxWcSdpy == UpGBjPFZXk){kaLtEGewrJ = true;}
      if(zQIuyeFPnH == loQLyFdASk){WBWIzEcwck = true;}
      else if(loQLyFdASk == zQIuyeFPnH){rYpytUByde = true;}
      if(CmgICiznrO == sjLKSykouq){YndueefEls = true;}
      if(xwgZJtoIme == nNtIGDIoJL){RNWDUGtGap = true;}
      if(QxpMaPgmBM == jeKYowZynI){zpSOmOtAmP = true;}
      while(sjLKSykouq == CmgICiznrO){dkHgZOeYwE = true;}
      while(nNtIGDIoJL == nNtIGDIoJL){NxyceJSXRJ = true;}
      while(jeKYowZynI == jeKYowZynI){mhACOsFsAU = true;}
      if(UPOndwmreX == true){UPOndwmreX = false;}
      if(qdiyJXtHFu == true){qdiyJXtHFu = false;}
      if(woqGAOZlUV == true){woqGAOZlUV = false;}
      if(YmAmVYyGdK == true){YmAmVYyGdK = false;}
      if(PQJlTGwLOQ == true){PQJlTGwLOQ = false;}
      if(BRssLfYInP == true){BRssLfYInP = false;}
      if(WBWIzEcwck == true){WBWIzEcwck = false;}
      if(YndueefEls == true){YndueefEls = false;}
      if(RNWDUGtGap == true){RNWDUGtGap = false;}
      if(zpSOmOtAmP == true){zpSOmOtAmP = false;}
      if(cAidrNnyOf == true){cAidrNnyOf = false;}
      if(ATVtxIdSnr == true){ATVtxIdSnr = false;}
      if(KWUlkhUPHP == true){KWUlkhUPHP = false;}
      if(keRWPRdsLH == true){keRWPRdsLH = false;}
      if(RuqupfUXQe == true){RuqupfUXQe = false;}
      if(kaLtEGewrJ == true){kaLtEGewrJ = false;}
      if(rYpytUByde == true){rYpytUByde = false;}
      if(dkHgZOeYwE == true){dkHgZOeYwE = false;}
      if(NxyceJSXRJ == true){NxyceJSXRJ = false;}
      if(mhACOsFsAU == true){mhACOsFsAU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QIPVQPVEUO
{ 
  void aiCTeMctJy()
  { 
      bool LAqHRKGrup = false;
      bool oZfiDDjMpu = false;
      bool XbAadpRcDd = false;
      bool lRSBLxhYkd = false;
      bool UkkrdBQjmO = false;
      bool GhluWjzQcb = false;
      bool scoQDWHdXG = false;
      bool VLbMampHwc = false;
      bool ZgpnWEeWea = false;
      bool xQwJjrwUnG = false;
      bool tmMfsuFZUj = false;
      bool sqUACLlAsz = false;
      bool qjzwIFPRbi = false;
      bool qVToilUKub = false;
      bool yFBthFwwHY = false;
      bool GwaoOJnQWm = false;
      bool fmxlsXXeQP = false;
      bool qaERAlhENC = false;
      bool ITKDbttdZY = false;
      bool yDmIwKHPAJ = false;
      string jfZjYRNFRw;
      string PAIAnqImEZ;
      string ONhOzmGiQO;
      string AlayOWkQPc;
      string jakqrKSJNp;
      string egBELWxLjk;
      string KenCwcYKlQ;
      string GqWbonbqVn;
      string TxVajSkWlr;
      string ZYgdSFmhtP;
      string lIXwimKZOX;
      string tFowQMJMSd;
      string rCYtqgKsIj;
      string BsxGCLZYrw;
      string ILHNxTjkMF;
      string uZPQWijcFB;
      string cQQeWBIisF;
      string mkTWVgtKxW;
      string xYfDnadgiS;
      string awSpSqtcqu;
      if(jfZjYRNFRw == lIXwimKZOX){LAqHRKGrup = true;}
      else if(lIXwimKZOX == jfZjYRNFRw){tmMfsuFZUj = true;}
      if(PAIAnqImEZ == tFowQMJMSd){oZfiDDjMpu = true;}
      else if(tFowQMJMSd == PAIAnqImEZ){sqUACLlAsz = true;}
      if(ONhOzmGiQO == rCYtqgKsIj){XbAadpRcDd = true;}
      else if(rCYtqgKsIj == ONhOzmGiQO){qjzwIFPRbi = true;}
      if(AlayOWkQPc == BsxGCLZYrw){lRSBLxhYkd = true;}
      else if(BsxGCLZYrw == AlayOWkQPc){qVToilUKub = true;}
      if(jakqrKSJNp == ILHNxTjkMF){UkkrdBQjmO = true;}
      else if(ILHNxTjkMF == jakqrKSJNp){yFBthFwwHY = true;}
      if(egBELWxLjk == uZPQWijcFB){GhluWjzQcb = true;}
      else if(uZPQWijcFB == egBELWxLjk){GwaoOJnQWm = true;}
      if(KenCwcYKlQ == cQQeWBIisF){scoQDWHdXG = true;}
      else if(cQQeWBIisF == KenCwcYKlQ){fmxlsXXeQP = true;}
      if(GqWbonbqVn == mkTWVgtKxW){VLbMampHwc = true;}
      if(TxVajSkWlr == xYfDnadgiS){ZgpnWEeWea = true;}
      if(ZYgdSFmhtP == awSpSqtcqu){xQwJjrwUnG = true;}
      while(mkTWVgtKxW == GqWbonbqVn){qaERAlhENC = true;}
      while(xYfDnadgiS == xYfDnadgiS){ITKDbttdZY = true;}
      while(awSpSqtcqu == awSpSqtcqu){yDmIwKHPAJ = true;}
      if(LAqHRKGrup == true){LAqHRKGrup = false;}
      if(oZfiDDjMpu == true){oZfiDDjMpu = false;}
      if(XbAadpRcDd == true){XbAadpRcDd = false;}
      if(lRSBLxhYkd == true){lRSBLxhYkd = false;}
      if(UkkrdBQjmO == true){UkkrdBQjmO = false;}
      if(GhluWjzQcb == true){GhluWjzQcb = false;}
      if(scoQDWHdXG == true){scoQDWHdXG = false;}
      if(VLbMampHwc == true){VLbMampHwc = false;}
      if(ZgpnWEeWea == true){ZgpnWEeWea = false;}
      if(xQwJjrwUnG == true){xQwJjrwUnG = false;}
      if(tmMfsuFZUj == true){tmMfsuFZUj = false;}
      if(sqUACLlAsz == true){sqUACLlAsz = false;}
      if(qjzwIFPRbi == true){qjzwIFPRbi = false;}
      if(qVToilUKub == true){qVToilUKub = false;}
      if(yFBthFwwHY == true){yFBthFwwHY = false;}
      if(GwaoOJnQWm == true){GwaoOJnQWm = false;}
      if(fmxlsXXeQP == true){fmxlsXXeQP = false;}
      if(qaERAlhENC == true){qaERAlhENC = false;}
      if(ITKDbttdZY == true){ITKDbttdZY = false;}
      if(yDmIwKHPAJ == true){yDmIwKHPAJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FEULBURJHK
{ 
  void PxAMOFXRcc()
  { 
      bool zMZuAwpsGh = false;
      bool nLTJYkBnTw = false;
      bool DSJdmzzOJt = false;
      bool tVRltZqQjj = false;
      bool FgWSzpxmHc = false;
      bool wgsiIfDuHA = false;
      bool xajLmSthPz = false;
      bool tkDiqWwqdx = false;
      bool BLHfxNchcN = false;
      bool WuIildwKeg = false;
      bool uPXIiLdJql = false;
      bool iLRKylTEOZ = false;
      bool oLEBczbUwM = false;
      bool ZlLxBlmzwk = false;
      bool iSpqeKcxjA = false;
      bool KquFqjKWej = false;
      bool jIMeLxATJM = false;
      bool yUIdpVUNWS = false;
      bool VdkuCipGGQ = false;
      bool bQTLAaFLeg = false;
      string kOHldwhnWk;
      string lNKogoCBbu;
      string xPKqfpEsgJ;
      string KKenjXxaWI;
      string ekRejBBMBP;
      string knqqmUEQGn;
      string qoHMabiVEI;
      string rbeSPLxfGd;
      string oqrQYVSmAO;
      string TgTiQpcFDH;
      string SeHrWIJYab;
      string XXJcyAwpmE;
      string JuVpoKHwBZ;
      string irbyLKiAtP;
      string LsMYquXqjQ;
      string smRyyWIRCM;
      string BNXArewKRn;
      string QwwXrPhxys;
      string mQLjgwacAn;
      string BQxgOyVMfM;
      if(kOHldwhnWk == SeHrWIJYab){zMZuAwpsGh = true;}
      else if(SeHrWIJYab == kOHldwhnWk){uPXIiLdJql = true;}
      if(lNKogoCBbu == XXJcyAwpmE){nLTJYkBnTw = true;}
      else if(XXJcyAwpmE == lNKogoCBbu){iLRKylTEOZ = true;}
      if(xPKqfpEsgJ == JuVpoKHwBZ){DSJdmzzOJt = true;}
      else if(JuVpoKHwBZ == xPKqfpEsgJ){oLEBczbUwM = true;}
      if(KKenjXxaWI == irbyLKiAtP){tVRltZqQjj = true;}
      else if(irbyLKiAtP == KKenjXxaWI){ZlLxBlmzwk = true;}
      if(ekRejBBMBP == LsMYquXqjQ){FgWSzpxmHc = true;}
      else if(LsMYquXqjQ == ekRejBBMBP){iSpqeKcxjA = true;}
      if(knqqmUEQGn == smRyyWIRCM){wgsiIfDuHA = true;}
      else if(smRyyWIRCM == knqqmUEQGn){KquFqjKWej = true;}
      if(qoHMabiVEI == BNXArewKRn){xajLmSthPz = true;}
      else if(BNXArewKRn == qoHMabiVEI){jIMeLxATJM = true;}
      if(rbeSPLxfGd == QwwXrPhxys){tkDiqWwqdx = true;}
      if(oqrQYVSmAO == mQLjgwacAn){BLHfxNchcN = true;}
      if(TgTiQpcFDH == BQxgOyVMfM){WuIildwKeg = true;}
      while(QwwXrPhxys == rbeSPLxfGd){yUIdpVUNWS = true;}
      while(mQLjgwacAn == mQLjgwacAn){VdkuCipGGQ = true;}
      while(BQxgOyVMfM == BQxgOyVMfM){bQTLAaFLeg = true;}
      if(zMZuAwpsGh == true){zMZuAwpsGh = false;}
      if(nLTJYkBnTw == true){nLTJYkBnTw = false;}
      if(DSJdmzzOJt == true){DSJdmzzOJt = false;}
      if(tVRltZqQjj == true){tVRltZqQjj = false;}
      if(FgWSzpxmHc == true){FgWSzpxmHc = false;}
      if(wgsiIfDuHA == true){wgsiIfDuHA = false;}
      if(xajLmSthPz == true){xajLmSthPz = false;}
      if(tkDiqWwqdx == true){tkDiqWwqdx = false;}
      if(BLHfxNchcN == true){BLHfxNchcN = false;}
      if(WuIildwKeg == true){WuIildwKeg = false;}
      if(uPXIiLdJql == true){uPXIiLdJql = false;}
      if(iLRKylTEOZ == true){iLRKylTEOZ = false;}
      if(oLEBczbUwM == true){oLEBczbUwM = false;}
      if(ZlLxBlmzwk == true){ZlLxBlmzwk = false;}
      if(iSpqeKcxjA == true){iSpqeKcxjA = false;}
      if(KquFqjKWej == true){KquFqjKWej = false;}
      if(jIMeLxATJM == true){jIMeLxATJM = false;}
      if(yUIdpVUNWS == true){yUIdpVUNWS = false;}
      if(VdkuCipGGQ == true){VdkuCipGGQ = false;}
      if(bQTLAaFLeg == true){bQTLAaFLeg = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WDGXVSEZFF
{ 
  void KqRzgBIgcC()
  { 
      bool LDXBIbohDz = false;
      bool ydnoGWXVZe = false;
      bool MuzPxcwabs = false;
      bool sChMmIMEJw = false;
      bool pIxCgEuajS = false;
      bool RDelDQDHDe = false;
      bool fOFKGuuaCx = false;
      bool JSLcnEywir = false;
      bool gUpIaoXwIW = false;
      bool FFAdQbClwk = false;
      bool JUYlJyrBxz = false;
      bool RGYLyKXwiQ = false;
      bool XySerESgMM = false;
      bool BLAxiMwEzc = false;
      bool xVzIEygofF = false;
      bool EFDeNKBstE = false;
      bool eKokyGFabk = false;
      bool OTxFSBzKat = false;
      bool XdBHIuHVjX = false;
      bool LDKtxmgNKy = false;
      string MKftkWLnIy;
      string JhqlANwyjs;
      string PGAnGZstVe;
      string MnCoXBTpkM;
      string XxuYUAHdtc;
      string TdyiPGKqnK;
      string cRRckVfBOy;
      string sGMWURKaTG;
      string RPArLioSZQ;
      string qbBZiGfSnz;
      string TFOujMQbNx;
      string wiWhrNRZRB;
      string fNbyGQEmnO;
      string lrmUlMAEIy;
      string SNEbUHFTgV;
      string oGBlSGdQnS;
      string HcnzKDwBuf;
      string MMQSiSIzcz;
      string LkFRYfqACe;
      string ijQppoYFQQ;
      if(MKftkWLnIy == TFOujMQbNx){LDXBIbohDz = true;}
      else if(TFOujMQbNx == MKftkWLnIy){JUYlJyrBxz = true;}
      if(JhqlANwyjs == wiWhrNRZRB){ydnoGWXVZe = true;}
      else if(wiWhrNRZRB == JhqlANwyjs){RGYLyKXwiQ = true;}
      if(PGAnGZstVe == fNbyGQEmnO){MuzPxcwabs = true;}
      else if(fNbyGQEmnO == PGAnGZstVe){XySerESgMM = true;}
      if(MnCoXBTpkM == lrmUlMAEIy){sChMmIMEJw = true;}
      else if(lrmUlMAEIy == MnCoXBTpkM){BLAxiMwEzc = true;}
      if(XxuYUAHdtc == SNEbUHFTgV){pIxCgEuajS = true;}
      else if(SNEbUHFTgV == XxuYUAHdtc){xVzIEygofF = true;}
      if(TdyiPGKqnK == oGBlSGdQnS){RDelDQDHDe = true;}
      else if(oGBlSGdQnS == TdyiPGKqnK){EFDeNKBstE = true;}
      if(cRRckVfBOy == HcnzKDwBuf){fOFKGuuaCx = true;}
      else if(HcnzKDwBuf == cRRckVfBOy){eKokyGFabk = true;}
      if(sGMWURKaTG == MMQSiSIzcz){JSLcnEywir = true;}
      if(RPArLioSZQ == LkFRYfqACe){gUpIaoXwIW = true;}
      if(qbBZiGfSnz == ijQppoYFQQ){FFAdQbClwk = true;}
      while(MMQSiSIzcz == sGMWURKaTG){OTxFSBzKat = true;}
      while(LkFRYfqACe == LkFRYfqACe){XdBHIuHVjX = true;}
      while(ijQppoYFQQ == ijQppoYFQQ){LDKtxmgNKy = true;}
      if(LDXBIbohDz == true){LDXBIbohDz = false;}
      if(ydnoGWXVZe == true){ydnoGWXVZe = false;}
      if(MuzPxcwabs == true){MuzPxcwabs = false;}
      if(sChMmIMEJw == true){sChMmIMEJw = false;}
      if(pIxCgEuajS == true){pIxCgEuajS = false;}
      if(RDelDQDHDe == true){RDelDQDHDe = false;}
      if(fOFKGuuaCx == true){fOFKGuuaCx = false;}
      if(JSLcnEywir == true){JSLcnEywir = false;}
      if(gUpIaoXwIW == true){gUpIaoXwIW = false;}
      if(FFAdQbClwk == true){FFAdQbClwk = false;}
      if(JUYlJyrBxz == true){JUYlJyrBxz = false;}
      if(RGYLyKXwiQ == true){RGYLyKXwiQ = false;}
      if(XySerESgMM == true){XySerESgMM = false;}
      if(BLAxiMwEzc == true){BLAxiMwEzc = false;}
      if(xVzIEygofF == true){xVzIEygofF = false;}
      if(EFDeNKBstE == true){EFDeNKBstE = false;}
      if(eKokyGFabk == true){eKokyGFabk = false;}
      if(OTxFSBzKat == true){OTxFSBzKat = false;}
      if(XdBHIuHVjX == true){XdBHIuHVjX = false;}
      if(LDKtxmgNKy == true){LDKtxmgNKy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TMUZTMPAYR
{ 
  void eEGXfqwJhh()
  { 
      bool JIHSDQYpLd = false;
      bool SYliqTSbWb = false;
      bool eYYIVAstQV = false;
      bool FWahrNIsKc = false;
      bool fDNFcfolhC = false;
      bool hSqglkiywC = false;
      bool WdZoXuyEqc = false;
      bool LZRtuaKTfo = false;
      bool KHMOnmesuG = false;
      bool eKwHFBdXgg = false;
      bool hjuFqIAxlK = false;
      bool aORGqPRLBk = false;
      bool PFIjyBcKns = false;
      bool xllyHaMHXf = false;
      bool lhHkfProPp = false;
      bool jSWwRbbKGo = false;
      bool epIrMncSzQ = false;
      bool CKmVzhxXDi = false;
      bool JntyJeOpge = false;
      bool yBAhAuYPXq = false;
      string ersDQTlKoM;
      string WbwKlKfSOs;
      string uksuebpSOZ;
      string tSmAhNjnZz;
      string TwnBSJdCSa;
      string EdyQetyxYn;
      string cTuOREnucY;
      string ZrrPZzwrbU;
      string xmznUQTWVm;
      string OmPkVzAYjO;
      string ViZtAMkiqf;
      string xGVDppTHtB;
      string kRZoJmbGsZ;
      string yXHMzONFmH;
      string ejfppIQLtf;
      string fsdhdEIkjN;
      string keoQgYyIEb;
      string cOTmWAkjyB;
      string VQLNtFtZiu;
      string FbUZWAtOIQ;
      if(ersDQTlKoM == ViZtAMkiqf){JIHSDQYpLd = true;}
      else if(ViZtAMkiqf == ersDQTlKoM){hjuFqIAxlK = true;}
      if(WbwKlKfSOs == xGVDppTHtB){SYliqTSbWb = true;}
      else if(xGVDppTHtB == WbwKlKfSOs){aORGqPRLBk = true;}
      if(uksuebpSOZ == kRZoJmbGsZ){eYYIVAstQV = true;}
      else if(kRZoJmbGsZ == uksuebpSOZ){PFIjyBcKns = true;}
      if(tSmAhNjnZz == yXHMzONFmH){FWahrNIsKc = true;}
      else if(yXHMzONFmH == tSmAhNjnZz){xllyHaMHXf = true;}
      if(TwnBSJdCSa == ejfppIQLtf){fDNFcfolhC = true;}
      else if(ejfppIQLtf == TwnBSJdCSa){lhHkfProPp = true;}
      if(EdyQetyxYn == fsdhdEIkjN){hSqglkiywC = true;}
      else if(fsdhdEIkjN == EdyQetyxYn){jSWwRbbKGo = true;}
      if(cTuOREnucY == keoQgYyIEb){WdZoXuyEqc = true;}
      else if(keoQgYyIEb == cTuOREnucY){epIrMncSzQ = true;}
      if(ZrrPZzwrbU == cOTmWAkjyB){LZRtuaKTfo = true;}
      if(xmznUQTWVm == VQLNtFtZiu){KHMOnmesuG = true;}
      if(OmPkVzAYjO == FbUZWAtOIQ){eKwHFBdXgg = true;}
      while(cOTmWAkjyB == ZrrPZzwrbU){CKmVzhxXDi = true;}
      while(VQLNtFtZiu == VQLNtFtZiu){JntyJeOpge = true;}
      while(FbUZWAtOIQ == FbUZWAtOIQ){yBAhAuYPXq = true;}
      if(JIHSDQYpLd == true){JIHSDQYpLd = false;}
      if(SYliqTSbWb == true){SYliqTSbWb = false;}
      if(eYYIVAstQV == true){eYYIVAstQV = false;}
      if(FWahrNIsKc == true){FWahrNIsKc = false;}
      if(fDNFcfolhC == true){fDNFcfolhC = false;}
      if(hSqglkiywC == true){hSqglkiywC = false;}
      if(WdZoXuyEqc == true){WdZoXuyEqc = false;}
      if(LZRtuaKTfo == true){LZRtuaKTfo = false;}
      if(KHMOnmesuG == true){KHMOnmesuG = false;}
      if(eKwHFBdXgg == true){eKwHFBdXgg = false;}
      if(hjuFqIAxlK == true){hjuFqIAxlK = false;}
      if(aORGqPRLBk == true){aORGqPRLBk = false;}
      if(PFIjyBcKns == true){PFIjyBcKns = false;}
      if(xllyHaMHXf == true){xllyHaMHXf = false;}
      if(lhHkfProPp == true){lhHkfProPp = false;}
      if(jSWwRbbKGo == true){jSWwRbbKGo = false;}
      if(epIrMncSzQ == true){epIrMncSzQ = false;}
      if(CKmVzhxXDi == true){CKmVzhxXDi = false;}
      if(JntyJeOpge == true){JntyJeOpge = false;}
      if(yBAhAuYPXq == true){yBAhAuYPXq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QJQWOZDJHR
{ 
  void QdTPInXyDC()
  { 
      bool WbTuMZdwQz = false;
      bool KxurfJGCGf = false;
      bool HCJVRaCKgH = false;
      bool VfcrxKEtVr = false;
      bool LVwIulzmPe = false;
      bool VahEFURxdl = false;
      bool WSPIddTDtV = false;
      bool FLgNUkFBZe = false;
      bool aPuBkuyYlb = false;
      bool rGQiVsGmeR = false;
      bool SwrsZKtTjU = false;
      bool PQXyKHWNXh = false;
      bool fuRScztxra = false;
      bool eRVIMeWBbZ = false;
      bool BdnRMbhSYN = false;
      bool VrPwkWdZQZ = false;
      bool QwbIoytOkf = false;
      bool jVkxgCGAGB = false;
      bool RbYdFlBYXo = false;
      bool AwkCJEBfbN = false;
      string jzGLGloGRp;
      string QJCxmTsAfa;
      string XbyLBgbDUA;
      string YmQdGQPRTM;
      string TrdaqEEkqE;
      string bQVFbIAtJY;
      string JllRRdrXpW;
      string DagTHfxXUx;
      string bqLQXIQLCy;
      string PUHtCgGMJM;
      string LgtTBAiCyu;
      string KcZMxSZVsI;
      string NgUrHhSmRZ;
      string ZkNHkuwrOW;
      string YNMGmluuxi;
      string HjSAzeLBMS;
      string hwkYVZOSbB;
      string VhGWjzTOFU;
      string mXphRoRMIF;
      string jueuobcnkN;
      if(jzGLGloGRp == LgtTBAiCyu){WbTuMZdwQz = true;}
      else if(LgtTBAiCyu == jzGLGloGRp){SwrsZKtTjU = true;}
      if(QJCxmTsAfa == KcZMxSZVsI){KxurfJGCGf = true;}
      else if(KcZMxSZVsI == QJCxmTsAfa){PQXyKHWNXh = true;}
      if(XbyLBgbDUA == NgUrHhSmRZ){HCJVRaCKgH = true;}
      else if(NgUrHhSmRZ == XbyLBgbDUA){fuRScztxra = true;}
      if(YmQdGQPRTM == ZkNHkuwrOW){VfcrxKEtVr = true;}
      else if(ZkNHkuwrOW == YmQdGQPRTM){eRVIMeWBbZ = true;}
      if(TrdaqEEkqE == YNMGmluuxi){LVwIulzmPe = true;}
      else if(YNMGmluuxi == TrdaqEEkqE){BdnRMbhSYN = true;}
      if(bQVFbIAtJY == HjSAzeLBMS){VahEFURxdl = true;}
      else if(HjSAzeLBMS == bQVFbIAtJY){VrPwkWdZQZ = true;}
      if(JllRRdrXpW == hwkYVZOSbB){WSPIddTDtV = true;}
      else if(hwkYVZOSbB == JllRRdrXpW){QwbIoytOkf = true;}
      if(DagTHfxXUx == VhGWjzTOFU){FLgNUkFBZe = true;}
      if(bqLQXIQLCy == mXphRoRMIF){aPuBkuyYlb = true;}
      if(PUHtCgGMJM == jueuobcnkN){rGQiVsGmeR = true;}
      while(VhGWjzTOFU == DagTHfxXUx){jVkxgCGAGB = true;}
      while(mXphRoRMIF == mXphRoRMIF){RbYdFlBYXo = true;}
      while(jueuobcnkN == jueuobcnkN){AwkCJEBfbN = true;}
      if(WbTuMZdwQz == true){WbTuMZdwQz = false;}
      if(KxurfJGCGf == true){KxurfJGCGf = false;}
      if(HCJVRaCKgH == true){HCJVRaCKgH = false;}
      if(VfcrxKEtVr == true){VfcrxKEtVr = false;}
      if(LVwIulzmPe == true){LVwIulzmPe = false;}
      if(VahEFURxdl == true){VahEFURxdl = false;}
      if(WSPIddTDtV == true){WSPIddTDtV = false;}
      if(FLgNUkFBZe == true){FLgNUkFBZe = false;}
      if(aPuBkuyYlb == true){aPuBkuyYlb = false;}
      if(rGQiVsGmeR == true){rGQiVsGmeR = false;}
      if(SwrsZKtTjU == true){SwrsZKtTjU = false;}
      if(PQXyKHWNXh == true){PQXyKHWNXh = false;}
      if(fuRScztxra == true){fuRScztxra = false;}
      if(eRVIMeWBbZ == true){eRVIMeWBbZ = false;}
      if(BdnRMbhSYN == true){BdnRMbhSYN = false;}
      if(VrPwkWdZQZ == true){VrPwkWdZQZ = false;}
      if(QwbIoytOkf == true){QwbIoytOkf = false;}
      if(jVkxgCGAGB == true){jVkxgCGAGB = false;}
      if(RbYdFlBYXo == true){RbYdFlBYXo = false;}
      if(AwkCJEBfbN == true){AwkCJEBfbN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CHWSDPIPAF
{ 
  void XxMaYhTQKN()
  { 
      bool hKFSFeeioT = false;
      bool FjtIElHODV = false;
      bool aRHhDxKAXU = false;
      bool dnKLNrYAog = false;
      bool stsKbmayxU = false;
      bool rmAZKPDWOI = false;
      bool TdKUoRYiuL = false;
      bool MHZWcgYxFQ = false;
      bool VMDDcqmkAx = false;
      bool oClfMOjXUK = false;
      bool NrCkIkMdxd = false;
      bool qGQxlHaHry = false;
      bool dnhyWKVsJN = false;
      bool wAbuDdJRtq = false;
      bool dJmVFikntF = false;
      bool ncfVfYJEOy = false;
      bool PTUjgqdVtI = false;
      bool xIMAUUAMrm = false;
      bool YhzEYTTFOd = false;
      bool TFDwbKoDnO = false;
      string XBnpoaAVRH;
      string OInTXUWhye;
      string aSKxEqBepZ;
      string zjDurSFDjc;
      string uZJMYPjIlX;
      string jkwJCFqpky;
      string YUerHeFrbF;
      string PSyVjBIxYo;
      string HsrSoNmssm;
      string bORkDZtREs;
      string JqfgIRmaZB;
      string wsRwqfcMDt;
      string rLzncAAMRJ;
      string GqsefPNKJX;
      string ZLTqqhxVtX;
      string xhNJAxrrDH;
      string QZJPuEAlQa;
      string SRCeUgrHxu;
      string DeStMYcieh;
      string XUGBGzCEwl;
      if(XBnpoaAVRH == JqfgIRmaZB){hKFSFeeioT = true;}
      else if(JqfgIRmaZB == XBnpoaAVRH){NrCkIkMdxd = true;}
      if(OInTXUWhye == wsRwqfcMDt){FjtIElHODV = true;}
      else if(wsRwqfcMDt == OInTXUWhye){qGQxlHaHry = true;}
      if(aSKxEqBepZ == rLzncAAMRJ){aRHhDxKAXU = true;}
      else if(rLzncAAMRJ == aSKxEqBepZ){dnhyWKVsJN = true;}
      if(zjDurSFDjc == GqsefPNKJX){dnKLNrYAog = true;}
      else if(GqsefPNKJX == zjDurSFDjc){wAbuDdJRtq = true;}
      if(uZJMYPjIlX == ZLTqqhxVtX){stsKbmayxU = true;}
      else if(ZLTqqhxVtX == uZJMYPjIlX){dJmVFikntF = true;}
      if(jkwJCFqpky == xhNJAxrrDH){rmAZKPDWOI = true;}
      else if(xhNJAxrrDH == jkwJCFqpky){ncfVfYJEOy = true;}
      if(YUerHeFrbF == QZJPuEAlQa){TdKUoRYiuL = true;}
      else if(QZJPuEAlQa == YUerHeFrbF){PTUjgqdVtI = true;}
      if(PSyVjBIxYo == SRCeUgrHxu){MHZWcgYxFQ = true;}
      if(HsrSoNmssm == DeStMYcieh){VMDDcqmkAx = true;}
      if(bORkDZtREs == XUGBGzCEwl){oClfMOjXUK = true;}
      while(SRCeUgrHxu == PSyVjBIxYo){xIMAUUAMrm = true;}
      while(DeStMYcieh == DeStMYcieh){YhzEYTTFOd = true;}
      while(XUGBGzCEwl == XUGBGzCEwl){TFDwbKoDnO = true;}
      if(hKFSFeeioT == true){hKFSFeeioT = false;}
      if(FjtIElHODV == true){FjtIElHODV = false;}
      if(aRHhDxKAXU == true){aRHhDxKAXU = false;}
      if(dnKLNrYAog == true){dnKLNrYAog = false;}
      if(stsKbmayxU == true){stsKbmayxU = false;}
      if(rmAZKPDWOI == true){rmAZKPDWOI = false;}
      if(TdKUoRYiuL == true){TdKUoRYiuL = false;}
      if(MHZWcgYxFQ == true){MHZWcgYxFQ = false;}
      if(VMDDcqmkAx == true){VMDDcqmkAx = false;}
      if(oClfMOjXUK == true){oClfMOjXUK = false;}
      if(NrCkIkMdxd == true){NrCkIkMdxd = false;}
      if(qGQxlHaHry == true){qGQxlHaHry = false;}
      if(dnhyWKVsJN == true){dnhyWKVsJN = false;}
      if(wAbuDdJRtq == true){wAbuDdJRtq = false;}
      if(dJmVFikntF == true){dJmVFikntF = false;}
      if(ncfVfYJEOy == true){ncfVfYJEOy = false;}
      if(PTUjgqdVtI == true){PTUjgqdVtI = false;}
      if(xIMAUUAMrm == true){xIMAUUAMrm = false;}
      if(YhzEYTTFOd == true){YhzEYTTFOd = false;}
      if(TFDwbKoDnO == true){TFDwbKoDnO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DRRTFIYFNI
{ 
  void MUENxaXAyx()
  { 
      bool HXCQFEufAS = false;
      bool kANKduONYn = false;
      bool jlfLhuiDcy = false;
      bool gAkGwEhshL = false;
      bool qOPBksPMIZ = false;
      bool DXUjcUqqhJ = false;
      bool VqDCkkPHNY = false;
      bool MVeNwUWeZo = false;
      bool fVuCtJfAcb = false;
      bool NqKXpAKNPA = false;
      bool dHziNgrGnH = false;
      bool yLQsdAwBHb = false;
      bool EOdlLULHXw = false;
      bool JJPZLXVLoN = false;
      bool QYMVcCaQto = false;
      bool DJFmuOdNXr = false;
      bool VkhqxouphG = false;
      bool dkLtLeCIIJ = false;
      bool ttNhsADcdw = false;
      bool tImwqfqUXm = false;
      string qKEqGoHUtF;
      string LWINuQDFfq;
      string ziPzwDCzlr;
      string aCtxFnjObA;
      string MEeGBkfAjl;
      string pFxtuItEtD;
      string lIiBeJcLHH;
      string focfeuDJxK;
      string XCOzCUEzBw;
      string seTYNDVdbq;
      string ITrcGBcRPA;
      string EjHjfisizJ;
      string FcJKNAUDRa;
      string YZhGBDKlsB;
      string rliArJXjsd;
      string KldRleJgOF;
      string fYAobeCAhb;
      string YGpyRStJSE;
      string bkKmPmuPTV;
      string sdXmUSksYM;
      if(qKEqGoHUtF == ITrcGBcRPA){HXCQFEufAS = true;}
      else if(ITrcGBcRPA == qKEqGoHUtF){dHziNgrGnH = true;}
      if(LWINuQDFfq == EjHjfisizJ){kANKduONYn = true;}
      else if(EjHjfisizJ == LWINuQDFfq){yLQsdAwBHb = true;}
      if(ziPzwDCzlr == FcJKNAUDRa){jlfLhuiDcy = true;}
      else if(FcJKNAUDRa == ziPzwDCzlr){EOdlLULHXw = true;}
      if(aCtxFnjObA == YZhGBDKlsB){gAkGwEhshL = true;}
      else if(YZhGBDKlsB == aCtxFnjObA){JJPZLXVLoN = true;}
      if(MEeGBkfAjl == rliArJXjsd){qOPBksPMIZ = true;}
      else if(rliArJXjsd == MEeGBkfAjl){QYMVcCaQto = true;}
      if(pFxtuItEtD == KldRleJgOF){DXUjcUqqhJ = true;}
      else if(KldRleJgOF == pFxtuItEtD){DJFmuOdNXr = true;}
      if(lIiBeJcLHH == fYAobeCAhb){VqDCkkPHNY = true;}
      else if(fYAobeCAhb == lIiBeJcLHH){VkhqxouphG = true;}
      if(focfeuDJxK == YGpyRStJSE){MVeNwUWeZo = true;}
      if(XCOzCUEzBw == bkKmPmuPTV){fVuCtJfAcb = true;}
      if(seTYNDVdbq == sdXmUSksYM){NqKXpAKNPA = true;}
      while(YGpyRStJSE == focfeuDJxK){dkLtLeCIIJ = true;}
      while(bkKmPmuPTV == bkKmPmuPTV){ttNhsADcdw = true;}
      while(sdXmUSksYM == sdXmUSksYM){tImwqfqUXm = true;}
      if(HXCQFEufAS == true){HXCQFEufAS = false;}
      if(kANKduONYn == true){kANKduONYn = false;}
      if(jlfLhuiDcy == true){jlfLhuiDcy = false;}
      if(gAkGwEhshL == true){gAkGwEhshL = false;}
      if(qOPBksPMIZ == true){qOPBksPMIZ = false;}
      if(DXUjcUqqhJ == true){DXUjcUqqhJ = false;}
      if(VqDCkkPHNY == true){VqDCkkPHNY = false;}
      if(MVeNwUWeZo == true){MVeNwUWeZo = false;}
      if(fVuCtJfAcb == true){fVuCtJfAcb = false;}
      if(NqKXpAKNPA == true){NqKXpAKNPA = false;}
      if(dHziNgrGnH == true){dHziNgrGnH = false;}
      if(yLQsdAwBHb == true){yLQsdAwBHb = false;}
      if(EOdlLULHXw == true){EOdlLULHXw = false;}
      if(JJPZLXVLoN == true){JJPZLXVLoN = false;}
      if(QYMVcCaQto == true){QYMVcCaQto = false;}
      if(DJFmuOdNXr == true){DJFmuOdNXr = false;}
      if(VkhqxouphG == true){VkhqxouphG = false;}
      if(dkLtLeCIIJ == true){dkLtLeCIIJ = false;}
      if(ttNhsADcdw == true){ttNhsADcdw = false;}
      if(tImwqfqUXm == true){tImwqfqUXm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PUUUNMTHYI
{ 
  void oAnNLcFMTk()
  { 
      bool GQUSBqGixq = false;
      bool VRoQmidbLm = false;
      bool cuAVoGwaDM = false;
      bool mVQmpLfsif = false;
      bool YoHKoWTwds = false;
      bool CcrLqLPzdu = false;
      bool teekDzABZa = false;
      bool DkrfOmnpgS = false;
      bool lAxJKKdFwe = false;
      bool ugOnksKWGo = false;
      bool VRVCpRjLxD = false;
      bool WZRLJtgVar = false;
      bool UtFteWVEuU = false;
      bool cQiqwTduTr = false;
      bool aSIsIVODeq = false;
      bool LnwNTaMaWS = false;
      bool ufQbZFpZOo = false;
      bool PlBhRClLlJ = false;
      bool HSZDIoUUXU = false;
      bool HDXAxuBfTC = false;
      string plmmhtNReB;
      string eXKlQamhbg;
      string pDnTixseCi;
      string TlONyEZsem;
      string VnFmmSMNgL;
      string gAESzmyTDj;
      string AiPwhsJwnM;
      string AsCnLCxBfQ;
      string kfxqBShuOK;
      string FFoulmXJEt;
      string JPZUKnxPHx;
      string oxmmyUkYtO;
      string KNhcGtDHCY;
      string dmwkGVFbfN;
      string VRmRVISCwQ;
      string BzPpjwhwOP;
      string joNiSBBCgc;
      string PJrfrrZPHx;
      string bBLkWeXFBK;
      string nJMQcfEDzT;
      if(plmmhtNReB == JPZUKnxPHx){GQUSBqGixq = true;}
      else if(JPZUKnxPHx == plmmhtNReB){VRVCpRjLxD = true;}
      if(eXKlQamhbg == oxmmyUkYtO){VRoQmidbLm = true;}
      else if(oxmmyUkYtO == eXKlQamhbg){WZRLJtgVar = true;}
      if(pDnTixseCi == KNhcGtDHCY){cuAVoGwaDM = true;}
      else if(KNhcGtDHCY == pDnTixseCi){UtFteWVEuU = true;}
      if(TlONyEZsem == dmwkGVFbfN){mVQmpLfsif = true;}
      else if(dmwkGVFbfN == TlONyEZsem){cQiqwTduTr = true;}
      if(VnFmmSMNgL == VRmRVISCwQ){YoHKoWTwds = true;}
      else if(VRmRVISCwQ == VnFmmSMNgL){aSIsIVODeq = true;}
      if(gAESzmyTDj == BzPpjwhwOP){CcrLqLPzdu = true;}
      else if(BzPpjwhwOP == gAESzmyTDj){LnwNTaMaWS = true;}
      if(AiPwhsJwnM == joNiSBBCgc){teekDzABZa = true;}
      else if(joNiSBBCgc == AiPwhsJwnM){ufQbZFpZOo = true;}
      if(AsCnLCxBfQ == PJrfrrZPHx){DkrfOmnpgS = true;}
      if(kfxqBShuOK == bBLkWeXFBK){lAxJKKdFwe = true;}
      if(FFoulmXJEt == nJMQcfEDzT){ugOnksKWGo = true;}
      while(PJrfrrZPHx == AsCnLCxBfQ){PlBhRClLlJ = true;}
      while(bBLkWeXFBK == bBLkWeXFBK){HSZDIoUUXU = true;}
      while(nJMQcfEDzT == nJMQcfEDzT){HDXAxuBfTC = true;}
      if(GQUSBqGixq == true){GQUSBqGixq = false;}
      if(VRoQmidbLm == true){VRoQmidbLm = false;}
      if(cuAVoGwaDM == true){cuAVoGwaDM = false;}
      if(mVQmpLfsif == true){mVQmpLfsif = false;}
      if(YoHKoWTwds == true){YoHKoWTwds = false;}
      if(CcrLqLPzdu == true){CcrLqLPzdu = false;}
      if(teekDzABZa == true){teekDzABZa = false;}
      if(DkrfOmnpgS == true){DkrfOmnpgS = false;}
      if(lAxJKKdFwe == true){lAxJKKdFwe = false;}
      if(ugOnksKWGo == true){ugOnksKWGo = false;}
      if(VRVCpRjLxD == true){VRVCpRjLxD = false;}
      if(WZRLJtgVar == true){WZRLJtgVar = false;}
      if(UtFteWVEuU == true){UtFteWVEuU = false;}
      if(cQiqwTduTr == true){cQiqwTduTr = false;}
      if(aSIsIVODeq == true){aSIsIVODeq = false;}
      if(LnwNTaMaWS == true){LnwNTaMaWS = false;}
      if(ufQbZFpZOo == true){ufQbZFpZOo = false;}
      if(PlBhRClLlJ == true){PlBhRClLlJ = false;}
      if(HSZDIoUUXU == true){HSZDIoUUXU = false;}
      if(HDXAxuBfTC == true){HDXAxuBfTC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PMZSGCJZVB
{ 
  void HuuFVsKqzw()
  { 
      bool GgPOCElacR = false;
      bool kSderOpYah = false;
      bool wxMyVpxHBF = false;
      bool RsVlhsAbTd = false;
      bool EHGzCODhhi = false;
      bool ScNOUJTRsL = false;
      bool HdYWAqxrwz = false;
      bool ONFxcnqNuG = false;
      bool lOYSmnfGQY = false;
      bool ZhiHVZhiyL = false;
      bool RLliUFXQkf = false;
      bool RFVlYsFGiL = false;
      bool LpaXYfdxaD = false;
      bool FHPjnacoyh = false;
      bool PPVkGTLxhc = false;
      bool UbzmzbkNKo = false;
      bool aZohZJplyy = false;
      bool PPIBuwUpRE = false;
      bool TgOxhdHbjM = false;
      bool oOeIGpZmKw = false;
      string LlDuarONfH;
      string tHsRuMEEHf;
      string lNHOuKJujf;
      string UoSAhYgsFw;
      string abCjInRORj;
      string HnBMuyenzP;
      string hTcAlEnriV;
      string eKyKEKKXGH;
      string ruczYfnGFj;
      string tssoNGyBxD;
      string AYfuFwpqHj;
      string nGhrXmqcbF;
      string QgmzblRIwy;
      string LIZDGTOgtb;
      string pwyUwmLBRa;
      string PUZqlSkgTS;
      string egtJYefdkD;
      string YKizOThSCo;
      string aPqViphiQm;
      string hhNaxJCowQ;
      if(LlDuarONfH == AYfuFwpqHj){GgPOCElacR = true;}
      else if(AYfuFwpqHj == LlDuarONfH){RLliUFXQkf = true;}
      if(tHsRuMEEHf == nGhrXmqcbF){kSderOpYah = true;}
      else if(nGhrXmqcbF == tHsRuMEEHf){RFVlYsFGiL = true;}
      if(lNHOuKJujf == QgmzblRIwy){wxMyVpxHBF = true;}
      else if(QgmzblRIwy == lNHOuKJujf){LpaXYfdxaD = true;}
      if(UoSAhYgsFw == LIZDGTOgtb){RsVlhsAbTd = true;}
      else if(LIZDGTOgtb == UoSAhYgsFw){FHPjnacoyh = true;}
      if(abCjInRORj == pwyUwmLBRa){EHGzCODhhi = true;}
      else if(pwyUwmLBRa == abCjInRORj){PPVkGTLxhc = true;}
      if(HnBMuyenzP == PUZqlSkgTS){ScNOUJTRsL = true;}
      else if(PUZqlSkgTS == HnBMuyenzP){UbzmzbkNKo = true;}
      if(hTcAlEnriV == egtJYefdkD){HdYWAqxrwz = true;}
      else if(egtJYefdkD == hTcAlEnriV){aZohZJplyy = true;}
      if(eKyKEKKXGH == YKizOThSCo){ONFxcnqNuG = true;}
      if(ruczYfnGFj == aPqViphiQm){lOYSmnfGQY = true;}
      if(tssoNGyBxD == hhNaxJCowQ){ZhiHVZhiyL = true;}
      while(YKizOThSCo == eKyKEKKXGH){PPIBuwUpRE = true;}
      while(aPqViphiQm == aPqViphiQm){TgOxhdHbjM = true;}
      while(hhNaxJCowQ == hhNaxJCowQ){oOeIGpZmKw = true;}
      if(GgPOCElacR == true){GgPOCElacR = false;}
      if(kSderOpYah == true){kSderOpYah = false;}
      if(wxMyVpxHBF == true){wxMyVpxHBF = false;}
      if(RsVlhsAbTd == true){RsVlhsAbTd = false;}
      if(EHGzCODhhi == true){EHGzCODhhi = false;}
      if(ScNOUJTRsL == true){ScNOUJTRsL = false;}
      if(HdYWAqxrwz == true){HdYWAqxrwz = false;}
      if(ONFxcnqNuG == true){ONFxcnqNuG = false;}
      if(lOYSmnfGQY == true){lOYSmnfGQY = false;}
      if(ZhiHVZhiyL == true){ZhiHVZhiyL = false;}
      if(RLliUFXQkf == true){RLliUFXQkf = false;}
      if(RFVlYsFGiL == true){RFVlYsFGiL = false;}
      if(LpaXYfdxaD == true){LpaXYfdxaD = false;}
      if(FHPjnacoyh == true){FHPjnacoyh = false;}
      if(PPVkGTLxhc == true){PPVkGTLxhc = false;}
      if(UbzmzbkNKo == true){UbzmzbkNKo = false;}
      if(aZohZJplyy == true){aZohZJplyy = false;}
      if(PPIBuwUpRE == true){PPIBuwUpRE = false;}
      if(TgOxhdHbjM == true){TgOxhdHbjM = false;}
      if(oOeIGpZmKw == true){oOeIGpZmKw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PDJEHDGGHI
{ 
  void jGSZCPyLBB()
  { 
      bool EVZUudPzoU = false;
      bool BLEjTIZhFU = false;
      bool bkeYhjfgGx = false;
      bool mUyxiUbGHU = false;
      bool loSxektbsY = false;
      bool yGqikTpAlK = false;
      bool xhsqhfNHAz = false;
      bool fSajhmRgsx = false;
      bool pEmgzgcOLP = false;
      bool daUwWDwbPz = false;
      bool WlatrefHBy = false;
      bool CIGnUrtLqB = false;
      bool CzwkUEydMd = false;
      bool qoRtaDdsnT = false;
      bool AVNMNVTgad = false;
      bool JjjOwWpoYm = false;
      bool gBtWHMqMNR = false;
      bool OBWXAgliWz = false;
      bool kQChUGFDxD = false;
      bool MnTAlyapEB = false;
      string UajYaybcJA;
      string DbVbuJBCuy;
      string lelFcgiBMt;
      string zMdiOzOhjo;
      string auhJpmdRGR;
      string LuWzZpoeus;
      string OZPOAqylNw;
      string uTRGXqwbPo;
      string CRuuiDodJX;
      string KjlpZBeTIy;
      string nqEKVqcQbi;
      string sMPGqwdUex;
      string BUcokLgafW;
      string McTQALNkYq;
      string ZYCadxUfzd;
      string WKnLxTOOYK;
      string JBjEVQsyFe;
      string isunWbSCKn;
      string pBbCzBoISg;
      string xyMNgYnxEN;
      if(UajYaybcJA == nqEKVqcQbi){EVZUudPzoU = true;}
      else if(nqEKVqcQbi == UajYaybcJA){WlatrefHBy = true;}
      if(DbVbuJBCuy == sMPGqwdUex){BLEjTIZhFU = true;}
      else if(sMPGqwdUex == DbVbuJBCuy){CIGnUrtLqB = true;}
      if(lelFcgiBMt == BUcokLgafW){bkeYhjfgGx = true;}
      else if(BUcokLgafW == lelFcgiBMt){CzwkUEydMd = true;}
      if(zMdiOzOhjo == McTQALNkYq){mUyxiUbGHU = true;}
      else if(McTQALNkYq == zMdiOzOhjo){qoRtaDdsnT = true;}
      if(auhJpmdRGR == ZYCadxUfzd){loSxektbsY = true;}
      else if(ZYCadxUfzd == auhJpmdRGR){AVNMNVTgad = true;}
      if(LuWzZpoeus == WKnLxTOOYK){yGqikTpAlK = true;}
      else if(WKnLxTOOYK == LuWzZpoeus){JjjOwWpoYm = true;}
      if(OZPOAqylNw == JBjEVQsyFe){xhsqhfNHAz = true;}
      else if(JBjEVQsyFe == OZPOAqylNw){gBtWHMqMNR = true;}
      if(uTRGXqwbPo == isunWbSCKn){fSajhmRgsx = true;}
      if(CRuuiDodJX == pBbCzBoISg){pEmgzgcOLP = true;}
      if(KjlpZBeTIy == xyMNgYnxEN){daUwWDwbPz = true;}
      while(isunWbSCKn == uTRGXqwbPo){OBWXAgliWz = true;}
      while(pBbCzBoISg == pBbCzBoISg){kQChUGFDxD = true;}
      while(xyMNgYnxEN == xyMNgYnxEN){MnTAlyapEB = true;}
      if(EVZUudPzoU == true){EVZUudPzoU = false;}
      if(BLEjTIZhFU == true){BLEjTIZhFU = false;}
      if(bkeYhjfgGx == true){bkeYhjfgGx = false;}
      if(mUyxiUbGHU == true){mUyxiUbGHU = false;}
      if(loSxektbsY == true){loSxektbsY = false;}
      if(yGqikTpAlK == true){yGqikTpAlK = false;}
      if(xhsqhfNHAz == true){xhsqhfNHAz = false;}
      if(fSajhmRgsx == true){fSajhmRgsx = false;}
      if(pEmgzgcOLP == true){pEmgzgcOLP = false;}
      if(daUwWDwbPz == true){daUwWDwbPz = false;}
      if(WlatrefHBy == true){WlatrefHBy = false;}
      if(CIGnUrtLqB == true){CIGnUrtLqB = false;}
      if(CzwkUEydMd == true){CzwkUEydMd = false;}
      if(qoRtaDdsnT == true){qoRtaDdsnT = false;}
      if(AVNMNVTgad == true){AVNMNVTgad = false;}
      if(JjjOwWpoYm == true){JjjOwWpoYm = false;}
      if(gBtWHMqMNR == true){gBtWHMqMNR = false;}
      if(OBWXAgliWz == true){OBWXAgliWz = false;}
      if(kQChUGFDxD == true){kQChUGFDxD = false;}
      if(MnTAlyapEB == true){MnTAlyapEB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DVNBFLLVPH
{ 
  void DPZalMrPgx()
  { 
      bool eAJtzTJwoK = false;
      bool ynSmDASESd = false;
      bool urtKMlWPJJ = false;
      bool HMwlbtLqGK = false;
      bool xYaOxlBILB = false;
      bool ZleGfmFyhN = false;
      bool xtleDkrfKJ = false;
      bool nbyHfWReMa = false;
      bool kNDHjVgoFo = false;
      bool nMIrNDluLM = false;
      bool sUyCNynXZJ = false;
      bool VakHcipeug = false;
      bool DCZVUaSFsc = false;
      bool jVTAgPPBtk = false;
      bool hJOwTNbBPe = false;
      bool MHAdIqtbgl = false;
      bool ehdxZAGkZM = false;
      bool TojJztmdBG = false;
      bool zRoOPcUoro = false;
      bool VmTIGzeRVT = false;
      string BhRlDeHxaC;
      string mzXDTEfEmt;
      string hiQVCAeSbI;
      string eQiQGUSOUN;
      string iDLPdffmnz;
      string aItZMSRujD;
      string AnUSWVPGwy;
      string lRjYLeWBrV;
      string UrebhVVJDn;
      string DWeQNuhssI;
      string PYpoNIXVNh;
      string uQDNiIIskA;
      string zGDItOmTLQ;
      string wdHiFuCpOY;
      string xAuVsmTQhF;
      string loIrJJVhty;
      string VEKxkQITHm;
      string FSeLDyLgOH;
      string tVlFeHVPZw;
      string ZTuPCspykb;
      if(BhRlDeHxaC == PYpoNIXVNh){eAJtzTJwoK = true;}
      else if(PYpoNIXVNh == BhRlDeHxaC){sUyCNynXZJ = true;}
      if(mzXDTEfEmt == uQDNiIIskA){ynSmDASESd = true;}
      else if(uQDNiIIskA == mzXDTEfEmt){VakHcipeug = true;}
      if(hiQVCAeSbI == zGDItOmTLQ){urtKMlWPJJ = true;}
      else if(zGDItOmTLQ == hiQVCAeSbI){DCZVUaSFsc = true;}
      if(eQiQGUSOUN == wdHiFuCpOY){HMwlbtLqGK = true;}
      else if(wdHiFuCpOY == eQiQGUSOUN){jVTAgPPBtk = true;}
      if(iDLPdffmnz == xAuVsmTQhF){xYaOxlBILB = true;}
      else if(xAuVsmTQhF == iDLPdffmnz){hJOwTNbBPe = true;}
      if(aItZMSRujD == loIrJJVhty){ZleGfmFyhN = true;}
      else if(loIrJJVhty == aItZMSRujD){MHAdIqtbgl = true;}
      if(AnUSWVPGwy == VEKxkQITHm){xtleDkrfKJ = true;}
      else if(VEKxkQITHm == AnUSWVPGwy){ehdxZAGkZM = true;}
      if(lRjYLeWBrV == FSeLDyLgOH){nbyHfWReMa = true;}
      if(UrebhVVJDn == tVlFeHVPZw){kNDHjVgoFo = true;}
      if(DWeQNuhssI == ZTuPCspykb){nMIrNDluLM = true;}
      while(FSeLDyLgOH == lRjYLeWBrV){TojJztmdBG = true;}
      while(tVlFeHVPZw == tVlFeHVPZw){zRoOPcUoro = true;}
      while(ZTuPCspykb == ZTuPCspykb){VmTIGzeRVT = true;}
      if(eAJtzTJwoK == true){eAJtzTJwoK = false;}
      if(ynSmDASESd == true){ynSmDASESd = false;}
      if(urtKMlWPJJ == true){urtKMlWPJJ = false;}
      if(HMwlbtLqGK == true){HMwlbtLqGK = false;}
      if(xYaOxlBILB == true){xYaOxlBILB = false;}
      if(ZleGfmFyhN == true){ZleGfmFyhN = false;}
      if(xtleDkrfKJ == true){xtleDkrfKJ = false;}
      if(nbyHfWReMa == true){nbyHfWReMa = false;}
      if(kNDHjVgoFo == true){kNDHjVgoFo = false;}
      if(nMIrNDluLM == true){nMIrNDluLM = false;}
      if(sUyCNynXZJ == true){sUyCNynXZJ = false;}
      if(VakHcipeug == true){VakHcipeug = false;}
      if(DCZVUaSFsc == true){DCZVUaSFsc = false;}
      if(jVTAgPPBtk == true){jVTAgPPBtk = false;}
      if(hJOwTNbBPe == true){hJOwTNbBPe = false;}
      if(MHAdIqtbgl == true){MHAdIqtbgl = false;}
      if(ehdxZAGkZM == true){ehdxZAGkZM = false;}
      if(TojJztmdBG == true){TojJztmdBG = false;}
      if(zRoOPcUoro == true){zRoOPcUoro = false;}
      if(VmTIGzeRVT == true){VmTIGzeRVT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WLIINILTBN
{ 
  void IkPzRVHmIG()
  { 
      bool UVwtiyeIbm = false;
      bool KXWJGoVCaz = false;
      bool MdZiJoeEOS = false;
      bool rmxXSoCnbr = false;
      bool AhBtEdlWko = false;
      bool SbbizquMkt = false;
      bool TZsJuXazpR = false;
      bool lhZhzebede = false;
      bool DHTZzPLOXb = false;
      bool spChYanrzg = false;
      bool DlfSCkBsjU = false;
      bool OPhGqLFTJe = false;
      bool MMbOXwerJX = false;
      bool qjfxbQaAWO = false;
      bool ORdCMcIwpM = false;
      bool VThUUpKRuG = false;
      bool FJjLdejjdZ = false;
      bool kExuQthYBu = false;
      bool niBwCWCLck = false;
      bool GJFQJEhSYh = false;
      string mARiVVMDJo;
      string FKOWkLOLRm;
      string ikeCoxlakt;
      string xrKUMUsGxe;
      string lxddlLrPfX;
      string qIsVWCVPtY;
      string jkROqNAsJP;
      string RHOmBkHAPc;
      string FVuNCoyoyK;
      string ycMYuAeOlY;
      string LyGPFitGTk;
      string uVzQSNpEMC;
      string OKtzRnuorB;
      string FrNqQGPeIS;
      string trUNPcdwGg;
      string JaCjJAEGyQ;
      string nFNSeCHHzL;
      string xLwsHWpfjn;
      string KziLqepdhP;
      string CIgqeodCiM;
      if(mARiVVMDJo == LyGPFitGTk){UVwtiyeIbm = true;}
      else if(LyGPFitGTk == mARiVVMDJo){DlfSCkBsjU = true;}
      if(FKOWkLOLRm == uVzQSNpEMC){KXWJGoVCaz = true;}
      else if(uVzQSNpEMC == FKOWkLOLRm){OPhGqLFTJe = true;}
      if(ikeCoxlakt == OKtzRnuorB){MdZiJoeEOS = true;}
      else if(OKtzRnuorB == ikeCoxlakt){MMbOXwerJX = true;}
      if(xrKUMUsGxe == FrNqQGPeIS){rmxXSoCnbr = true;}
      else if(FrNqQGPeIS == xrKUMUsGxe){qjfxbQaAWO = true;}
      if(lxddlLrPfX == trUNPcdwGg){AhBtEdlWko = true;}
      else if(trUNPcdwGg == lxddlLrPfX){ORdCMcIwpM = true;}
      if(qIsVWCVPtY == JaCjJAEGyQ){SbbizquMkt = true;}
      else if(JaCjJAEGyQ == qIsVWCVPtY){VThUUpKRuG = true;}
      if(jkROqNAsJP == nFNSeCHHzL){TZsJuXazpR = true;}
      else if(nFNSeCHHzL == jkROqNAsJP){FJjLdejjdZ = true;}
      if(RHOmBkHAPc == xLwsHWpfjn){lhZhzebede = true;}
      if(FVuNCoyoyK == KziLqepdhP){DHTZzPLOXb = true;}
      if(ycMYuAeOlY == CIgqeodCiM){spChYanrzg = true;}
      while(xLwsHWpfjn == RHOmBkHAPc){kExuQthYBu = true;}
      while(KziLqepdhP == KziLqepdhP){niBwCWCLck = true;}
      while(CIgqeodCiM == CIgqeodCiM){GJFQJEhSYh = true;}
      if(UVwtiyeIbm == true){UVwtiyeIbm = false;}
      if(KXWJGoVCaz == true){KXWJGoVCaz = false;}
      if(MdZiJoeEOS == true){MdZiJoeEOS = false;}
      if(rmxXSoCnbr == true){rmxXSoCnbr = false;}
      if(AhBtEdlWko == true){AhBtEdlWko = false;}
      if(SbbizquMkt == true){SbbizquMkt = false;}
      if(TZsJuXazpR == true){TZsJuXazpR = false;}
      if(lhZhzebede == true){lhZhzebede = false;}
      if(DHTZzPLOXb == true){DHTZzPLOXb = false;}
      if(spChYanrzg == true){spChYanrzg = false;}
      if(DlfSCkBsjU == true){DlfSCkBsjU = false;}
      if(OPhGqLFTJe == true){OPhGqLFTJe = false;}
      if(MMbOXwerJX == true){MMbOXwerJX = false;}
      if(qjfxbQaAWO == true){qjfxbQaAWO = false;}
      if(ORdCMcIwpM == true){ORdCMcIwpM = false;}
      if(VThUUpKRuG == true){VThUUpKRuG = false;}
      if(FJjLdejjdZ == true){FJjLdejjdZ = false;}
      if(kExuQthYBu == true){kExuQthYBu = false;}
      if(niBwCWCLck == true){niBwCWCLck = false;}
      if(GJFQJEhSYh == true){GJFQJEhSYh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LATRTRJXJU
{ 
  void VbIcwOZHmZ()
  { 
      bool fPJHcNadIw = false;
      bool JFeDimcPwh = false;
      bool WLszzuzTnI = false;
      bool BahTfsqyXG = false;
      bool gTPVKbjMfL = false;
      bool SCXjITVKHh = false;
      bool NHoAuhWLWp = false;
      bool weqoADwXnW = false;
      bool KlmcoyDyKL = false;
      bool WgposZdPac = false;
      bool EXppPnLhiE = false;
      bool XyVjWaTLKx = false;
      bool dYAKSOoBwa = false;
      bool YfxLSmwwMB = false;
      bool mUOFnXLFSR = false;
      bool FSEcyfWeXA = false;
      bool zJlIfrAJRj = false;
      bool eXyJJJJuWt = false;
      bool DjidUGaTNm = false;
      bool PQYMCchGZu = false;
      string eMOTYFBqNW;
      string cNbFxYWjBS;
      string tmNRTzeeRc;
      string igjyYjSVhS;
      string uZehPMtIFi;
      string UcpkdqzwzZ;
      string aVYjnucdfN;
      string GBBhXTIeCH;
      string EVYiIFqyTh;
      string jVVIrZAttT;
      string iQbrbTbHNK;
      string hUtrUEwpbh;
      string DeKycjespx;
      string SOjhXnsfGn;
      string djjJzPbdnn;
      string cKRMPEGdPq;
      string ohRQqTjLTg;
      string KPinETtAyM;
      string IBFBTIxXRT;
      string gPQPzuSTSe;
      if(eMOTYFBqNW == iQbrbTbHNK){fPJHcNadIw = true;}
      else if(iQbrbTbHNK == eMOTYFBqNW){EXppPnLhiE = true;}
      if(cNbFxYWjBS == hUtrUEwpbh){JFeDimcPwh = true;}
      else if(hUtrUEwpbh == cNbFxYWjBS){XyVjWaTLKx = true;}
      if(tmNRTzeeRc == DeKycjespx){WLszzuzTnI = true;}
      else if(DeKycjespx == tmNRTzeeRc){dYAKSOoBwa = true;}
      if(igjyYjSVhS == SOjhXnsfGn){BahTfsqyXG = true;}
      else if(SOjhXnsfGn == igjyYjSVhS){YfxLSmwwMB = true;}
      if(uZehPMtIFi == djjJzPbdnn){gTPVKbjMfL = true;}
      else if(djjJzPbdnn == uZehPMtIFi){mUOFnXLFSR = true;}
      if(UcpkdqzwzZ == cKRMPEGdPq){SCXjITVKHh = true;}
      else if(cKRMPEGdPq == UcpkdqzwzZ){FSEcyfWeXA = true;}
      if(aVYjnucdfN == ohRQqTjLTg){NHoAuhWLWp = true;}
      else if(ohRQqTjLTg == aVYjnucdfN){zJlIfrAJRj = true;}
      if(GBBhXTIeCH == KPinETtAyM){weqoADwXnW = true;}
      if(EVYiIFqyTh == IBFBTIxXRT){KlmcoyDyKL = true;}
      if(jVVIrZAttT == gPQPzuSTSe){WgposZdPac = true;}
      while(KPinETtAyM == GBBhXTIeCH){eXyJJJJuWt = true;}
      while(IBFBTIxXRT == IBFBTIxXRT){DjidUGaTNm = true;}
      while(gPQPzuSTSe == gPQPzuSTSe){PQYMCchGZu = true;}
      if(fPJHcNadIw == true){fPJHcNadIw = false;}
      if(JFeDimcPwh == true){JFeDimcPwh = false;}
      if(WLszzuzTnI == true){WLszzuzTnI = false;}
      if(BahTfsqyXG == true){BahTfsqyXG = false;}
      if(gTPVKbjMfL == true){gTPVKbjMfL = false;}
      if(SCXjITVKHh == true){SCXjITVKHh = false;}
      if(NHoAuhWLWp == true){NHoAuhWLWp = false;}
      if(weqoADwXnW == true){weqoADwXnW = false;}
      if(KlmcoyDyKL == true){KlmcoyDyKL = false;}
      if(WgposZdPac == true){WgposZdPac = false;}
      if(EXppPnLhiE == true){EXppPnLhiE = false;}
      if(XyVjWaTLKx == true){XyVjWaTLKx = false;}
      if(dYAKSOoBwa == true){dYAKSOoBwa = false;}
      if(YfxLSmwwMB == true){YfxLSmwwMB = false;}
      if(mUOFnXLFSR == true){mUOFnXLFSR = false;}
      if(FSEcyfWeXA == true){FSEcyfWeXA = false;}
      if(zJlIfrAJRj == true){zJlIfrAJRj = false;}
      if(eXyJJJJuWt == true){eXyJJJJuWt = false;}
      if(DjidUGaTNm == true){DjidUGaTNm = false;}
      if(PQYMCchGZu == true){PQYMCchGZu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QIWQSKLGYC
{ 
  void NYTrsjcqZY()
  { 
      bool iTPWhWeHrZ = false;
      bool OOqrRRjPna = false;
      bool AefYsVKoLA = false;
      bool wXfuzDkwMo = false;
      bool WsAtcIOkpz = false;
      bool mnqklEnpcL = false;
      bool MpwqcqhurO = false;
      bool gnCyEGdlwj = false;
      bool HZyLMHfuJu = false;
      bool PtTAcgxIQl = false;
      bool aSrbRykrge = false;
      bool ZBtrElqjdU = false;
      bool KmlwylUCVX = false;
      bool JVVJJogWtN = false;
      bool WnOdVNAawp = false;
      bool oMduamguce = false;
      bool diWIVEfjhP = false;
      bool FTpQyWYHak = false;
      bool ksFehxqZhW = false;
      bool IlsOjnRFIR = false;
      string BcreOqczaX;
      string HhrtRulzOo;
      string KeYQssNOUc;
      string nOxLypotGs;
      string QFuAhjWosm;
      string qCjuNWDbkU;
      string dNdxjsjfwb;
      string FqrUYFiXhJ;
      string WXVgkkdCxa;
      string lNJlPCaxPo;
      string zpsQhAJPWI;
      string mfsbxFrxMz;
      string OhhUorhGEV;
      string elIsJfdEzj;
      string aOeomGTgnJ;
      string JcjPZRHYbp;
      string SDKAcBWzRR;
      string bmycXACsgi;
      string XAStLBBrTr;
      string XFcIRXDeMl;
      if(BcreOqczaX == zpsQhAJPWI){iTPWhWeHrZ = true;}
      else if(zpsQhAJPWI == BcreOqczaX){aSrbRykrge = true;}
      if(HhrtRulzOo == mfsbxFrxMz){OOqrRRjPna = true;}
      else if(mfsbxFrxMz == HhrtRulzOo){ZBtrElqjdU = true;}
      if(KeYQssNOUc == OhhUorhGEV){AefYsVKoLA = true;}
      else if(OhhUorhGEV == KeYQssNOUc){KmlwylUCVX = true;}
      if(nOxLypotGs == elIsJfdEzj){wXfuzDkwMo = true;}
      else if(elIsJfdEzj == nOxLypotGs){JVVJJogWtN = true;}
      if(QFuAhjWosm == aOeomGTgnJ){WsAtcIOkpz = true;}
      else if(aOeomGTgnJ == QFuAhjWosm){WnOdVNAawp = true;}
      if(qCjuNWDbkU == JcjPZRHYbp){mnqklEnpcL = true;}
      else if(JcjPZRHYbp == qCjuNWDbkU){oMduamguce = true;}
      if(dNdxjsjfwb == SDKAcBWzRR){MpwqcqhurO = true;}
      else if(SDKAcBWzRR == dNdxjsjfwb){diWIVEfjhP = true;}
      if(FqrUYFiXhJ == bmycXACsgi){gnCyEGdlwj = true;}
      if(WXVgkkdCxa == XAStLBBrTr){HZyLMHfuJu = true;}
      if(lNJlPCaxPo == XFcIRXDeMl){PtTAcgxIQl = true;}
      while(bmycXACsgi == FqrUYFiXhJ){FTpQyWYHak = true;}
      while(XAStLBBrTr == XAStLBBrTr){ksFehxqZhW = true;}
      while(XFcIRXDeMl == XFcIRXDeMl){IlsOjnRFIR = true;}
      if(iTPWhWeHrZ == true){iTPWhWeHrZ = false;}
      if(OOqrRRjPna == true){OOqrRRjPna = false;}
      if(AefYsVKoLA == true){AefYsVKoLA = false;}
      if(wXfuzDkwMo == true){wXfuzDkwMo = false;}
      if(WsAtcIOkpz == true){WsAtcIOkpz = false;}
      if(mnqklEnpcL == true){mnqklEnpcL = false;}
      if(MpwqcqhurO == true){MpwqcqhurO = false;}
      if(gnCyEGdlwj == true){gnCyEGdlwj = false;}
      if(HZyLMHfuJu == true){HZyLMHfuJu = false;}
      if(PtTAcgxIQl == true){PtTAcgxIQl = false;}
      if(aSrbRykrge == true){aSrbRykrge = false;}
      if(ZBtrElqjdU == true){ZBtrElqjdU = false;}
      if(KmlwylUCVX == true){KmlwylUCVX = false;}
      if(JVVJJogWtN == true){JVVJJogWtN = false;}
      if(WnOdVNAawp == true){WnOdVNAawp = false;}
      if(oMduamguce == true){oMduamguce = false;}
      if(diWIVEfjhP == true){diWIVEfjhP = false;}
      if(FTpQyWYHak == true){FTpQyWYHak = false;}
      if(ksFehxqZhW == true){ksFehxqZhW = false;}
      if(IlsOjnRFIR == true){IlsOjnRFIR = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WPBIPBDMBE
{ 
  void fkiHtIXYKA()
  { 
      bool QoDmdspaQa = false;
      bool OjAXdJramX = false;
      bool QkpcDblSyd = false;
      bool nyxnFFNmhB = false;
      bool jdBsaUMHLt = false;
      bool OwaClezaiC = false;
      bool GWcmmPmLSX = false;
      bool uFrlnmixBh = false;
      bool OeHDPqXuBZ = false;
      bool GmDftMAOFL = false;
      bool hBomyIEoNy = false;
      bool wCZAnbOZXu = false;
      bool rqJrRIBLqa = false;
      bool mUqpANoUEG = false;
      bool mhgDdDNDrR = false;
      bool xapKSpqwVp = false;
      bool HrxmLaLiAa = false;
      bool mtURIJKGNN = false;
      bool uyTYcLdfWJ = false;
      bool VryOytjtqC = false;
      string ngJVzjlQnQ;
      string pgHalKqqRY;
      string TnXahMXfMI;
      string ppmOdCzuEY;
      string LYrnKewHUe;
      string LbMCRAIhml;
      string LFDeZAtrwj;
      string LwmsNtuiQC;
      string QQLgwOdkwd;
      string sQHRNGUMNJ;
      string kDKkTPnSoX;
      string EYRkVtsViL;
      string WIwRptnVfG;
      string YhVHRMFaNf;
      string ZKwYZTzWuS;
      string MydCmbHrtY;
      string bSmhyVDJSI;
      string nSICkLMXrV;
      string uRZMtZESTp;
      string FMfoEHXNZD;
      if(ngJVzjlQnQ == kDKkTPnSoX){QoDmdspaQa = true;}
      else if(kDKkTPnSoX == ngJVzjlQnQ){hBomyIEoNy = true;}
      if(pgHalKqqRY == EYRkVtsViL){OjAXdJramX = true;}
      else if(EYRkVtsViL == pgHalKqqRY){wCZAnbOZXu = true;}
      if(TnXahMXfMI == WIwRptnVfG){QkpcDblSyd = true;}
      else if(WIwRptnVfG == TnXahMXfMI){rqJrRIBLqa = true;}
      if(ppmOdCzuEY == YhVHRMFaNf){nyxnFFNmhB = true;}
      else if(YhVHRMFaNf == ppmOdCzuEY){mUqpANoUEG = true;}
      if(LYrnKewHUe == ZKwYZTzWuS){jdBsaUMHLt = true;}
      else if(ZKwYZTzWuS == LYrnKewHUe){mhgDdDNDrR = true;}
      if(LbMCRAIhml == MydCmbHrtY){OwaClezaiC = true;}
      else if(MydCmbHrtY == LbMCRAIhml){xapKSpqwVp = true;}
      if(LFDeZAtrwj == bSmhyVDJSI){GWcmmPmLSX = true;}
      else if(bSmhyVDJSI == LFDeZAtrwj){HrxmLaLiAa = true;}
      if(LwmsNtuiQC == nSICkLMXrV){uFrlnmixBh = true;}
      if(QQLgwOdkwd == uRZMtZESTp){OeHDPqXuBZ = true;}
      if(sQHRNGUMNJ == FMfoEHXNZD){GmDftMAOFL = true;}
      while(nSICkLMXrV == LwmsNtuiQC){mtURIJKGNN = true;}
      while(uRZMtZESTp == uRZMtZESTp){uyTYcLdfWJ = true;}
      while(FMfoEHXNZD == FMfoEHXNZD){VryOytjtqC = true;}
      if(QoDmdspaQa == true){QoDmdspaQa = false;}
      if(OjAXdJramX == true){OjAXdJramX = false;}
      if(QkpcDblSyd == true){QkpcDblSyd = false;}
      if(nyxnFFNmhB == true){nyxnFFNmhB = false;}
      if(jdBsaUMHLt == true){jdBsaUMHLt = false;}
      if(OwaClezaiC == true){OwaClezaiC = false;}
      if(GWcmmPmLSX == true){GWcmmPmLSX = false;}
      if(uFrlnmixBh == true){uFrlnmixBh = false;}
      if(OeHDPqXuBZ == true){OeHDPqXuBZ = false;}
      if(GmDftMAOFL == true){GmDftMAOFL = false;}
      if(hBomyIEoNy == true){hBomyIEoNy = false;}
      if(wCZAnbOZXu == true){wCZAnbOZXu = false;}
      if(rqJrRIBLqa == true){rqJrRIBLqa = false;}
      if(mUqpANoUEG == true){mUqpANoUEG = false;}
      if(mhgDdDNDrR == true){mhgDdDNDrR = false;}
      if(xapKSpqwVp == true){xapKSpqwVp = false;}
      if(HrxmLaLiAa == true){HrxmLaLiAa = false;}
      if(mtURIJKGNN == true){mtURIJKGNN = false;}
      if(uyTYcLdfWJ == true){uyTYcLdfWJ = false;}
      if(VryOytjtqC == true){VryOytjtqC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DUUMXXBBHJ
{ 
  void VExJBblJXs()
  { 
      bool nLLqeoPxSQ = false;
      bool ksSiAIfCeE = false;
      bool bXMhQcfnGk = false;
      bool usmTPLXlIt = false;
      bool rDzqJgPuTh = false;
      bool MkgSyQzdZC = false;
      bool JaqgVrFzSo = false;
      bool kssjMYYVXj = false;
      bool MjUYXJLUUz = false;
      bool MsLGkQLRUR = false;
      bool aQbexTrwFY = false;
      bool qcFpoYGRJL = false;
      bool fjdTbqglzw = false;
      bool MGAGJwHpbD = false;
      bool ROPuHinVRL = false;
      bool KtHIkplICF = false;
      bool mGxJMkbzfr = false;
      bool QPpFtRUhLs = false;
      bool DKTnlSyqVS = false;
      bool JzmVQLRGiW = false;
      string dZrCIdtnXV;
      string TpJHyUyZHI;
      string JfbuhGHcOq;
      string OFkhcgGXHY;
      string uAHjaaatnr;
      string roEMDyxxUB;
      string WRYfGNEaeR;
      string OaShNIXrUG;
      string eMnWzZAYee;
      string iJCFBdabcM;
      string VAOVdQWWyt;
      string ZEbNWKcCfg;
      string MfuAiEXMfj;
      string gwOHkexRsi;
      string CdajEdgoiq;
      string AYuwzpEOmY;
      string rJpOMcpUcq;
      string MKIxdiFKuE;
      string XidnjIARHt;
      string ZieTpsmLWa;
      if(dZrCIdtnXV == VAOVdQWWyt){nLLqeoPxSQ = true;}
      else if(VAOVdQWWyt == dZrCIdtnXV){aQbexTrwFY = true;}
      if(TpJHyUyZHI == ZEbNWKcCfg){ksSiAIfCeE = true;}
      else if(ZEbNWKcCfg == TpJHyUyZHI){qcFpoYGRJL = true;}
      if(JfbuhGHcOq == MfuAiEXMfj){bXMhQcfnGk = true;}
      else if(MfuAiEXMfj == JfbuhGHcOq){fjdTbqglzw = true;}
      if(OFkhcgGXHY == gwOHkexRsi){usmTPLXlIt = true;}
      else if(gwOHkexRsi == OFkhcgGXHY){MGAGJwHpbD = true;}
      if(uAHjaaatnr == CdajEdgoiq){rDzqJgPuTh = true;}
      else if(CdajEdgoiq == uAHjaaatnr){ROPuHinVRL = true;}
      if(roEMDyxxUB == AYuwzpEOmY){MkgSyQzdZC = true;}
      else if(AYuwzpEOmY == roEMDyxxUB){KtHIkplICF = true;}
      if(WRYfGNEaeR == rJpOMcpUcq){JaqgVrFzSo = true;}
      else if(rJpOMcpUcq == WRYfGNEaeR){mGxJMkbzfr = true;}
      if(OaShNIXrUG == MKIxdiFKuE){kssjMYYVXj = true;}
      if(eMnWzZAYee == XidnjIARHt){MjUYXJLUUz = true;}
      if(iJCFBdabcM == ZieTpsmLWa){MsLGkQLRUR = true;}
      while(MKIxdiFKuE == OaShNIXrUG){QPpFtRUhLs = true;}
      while(XidnjIARHt == XidnjIARHt){DKTnlSyqVS = true;}
      while(ZieTpsmLWa == ZieTpsmLWa){JzmVQLRGiW = true;}
      if(nLLqeoPxSQ == true){nLLqeoPxSQ = false;}
      if(ksSiAIfCeE == true){ksSiAIfCeE = false;}
      if(bXMhQcfnGk == true){bXMhQcfnGk = false;}
      if(usmTPLXlIt == true){usmTPLXlIt = false;}
      if(rDzqJgPuTh == true){rDzqJgPuTh = false;}
      if(MkgSyQzdZC == true){MkgSyQzdZC = false;}
      if(JaqgVrFzSo == true){JaqgVrFzSo = false;}
      if(kssjMYYVXj == true){kssjMYYVXj = false;}
      if(MjUYXJLUUz == true){MjUYXJLUUz = false;}
      if(MsLGkQLRUR == true){MsLGkQLRUR = false;}
      if(aQbexTrwFY == true){aQbexTrwFY = false;}
      if(qcFpoYGRJL == true){qcFpoYGRJL = false;}
      if(fjdTbqglzw == true){fjdTbqglzw = false;}
      if(MGAGJwHpbD == true){MGAGJwHpbD = false;}
      if(ROPuHinVRL == true){ROPuHinVRL = false;}
      if(KtHIkplICF == true){KtHIkplICF = false;}
      if(mGxJMkbzfr == true){mGxJMkbzfr = false;}
      if(QPpFtRUhLs == true){QPpFtRUhLs = false;}
      if(DKTnlSyqVS == true){DKTnlSyqVS = false;}
      if(JzmVQLRGiW == true){JzmVQLRGiW = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IBTFBEUBTW
{ 
  void BWxnBOWyyg()
  { 
      bool dIRxxzPLzU = false;
      bool ewrhAZrFKz = false;
      bool JKmrkBJsyc = false;
      bool TAyfRxRTki = false;
      bool ZtEiZNBNiF = false;
      bool yFrVzTVZLu = false;
      bool pVMVBdyMPD = false;
      bool IXTbYPFESW = false;
      bool AuJsdOHbfy = false;
      bool brIprDgihu = false;
      bool PnxfxakQxj = false;
      bool NQGnutMTql = false;
      bool VVgRaQPdiU = false;
      bool hWWXHLyduD = false;
      bool dpewjKkTty = false;
      bool qNquYsHeEf = false;
      bool KGhbBqtigi = false;
      bool uhXnIeqjhQ = false;
      bool ewijcUfazO = false;
      bool ETYOrWICts = false;
      string LiprbTCuhu;
      string UEaIueLbFc;
      string lplKijWeVL;
      string HYcJHbbKFN;
      string GnhaaLNylQ;
      string KoIBXUxfJr;
      string wXZVmubQBT;
      string DAWrVyIKjX;
      string WYrEASQakl;
      string ewBNQiOLne;
      string iwxSNKSQeZ;
      string MlukVXeUVU;
      string eAfOoGzIbl;
      string jwWEnZbZqp;
      string AAwLZrAPrs;
      string EspSAiQhdG;
      string KJnrpPOqxu;
      string uEdeTeETow;
      string GjQihpFDEE;
      string JtujfqLDVJ;
      if(LiprbTCuhu == iwxSNKSQeZ){dIRxxzPLzU = true;}
      else if(iwxSNKSQeZ == LiprbTCuhu){PnxfxakQxj = true;}
      if(UEaIueLbFc == MlukVXeUVU){ewrhAZrFKz = true;}
      else if(MlukVXeUVU == UEaIueLbFc){NQGnutMTql = true;}
      if(lplKijWeVL == eAfOoGzIbl){JKmrkBJsyc = true;}
      else if(eAfOoGzIbl == lplKijWeVL){VVgRaQPdiU = true;}
      if(HYcJHbbKFN == jwWEnZbZqp){TAyfRxRTki = true;}
      else if(jwWEnZbZqp == HYcJHbbKFN){hWWXHLyduD = true;}
      if(GnhaaLNylQ == AAwLZrAPrs){ZtEiZNBNiF = true;}
      else if(AAwLZrAPrs == GnhaaLNylQ){dpewjKkTty = true;}
      if(KoIBXUxfJr == EspSAiQhdG){yFrVzTVZLu = true;}
      else if(EspSAiQhdG == KoIBXUxfJr){qNquYsHeEf = true;}
      if(wXZVmubQBT == KJnrpPOqxu){pVMVBdyMPD = true;}
      else if(KJnrpPOqxu == wXZVmubQBT){KGhbBqtigi = true;}
      if(DAWrVyIKjX == uEdeTeETow){IXTbYPFESW = true;}
      if(WYrEASQakl == GjQihpFDEE){AuJsdOHbfy = true;}
      if(ewBNQiOLne == JtujfqLDVJ){brIprDgihu = true;}
      while(uEdeTeETow == DAWrVyIKjX){uhXnIeqjhQ = true;}
      while(GjQihpFDEE == GjQihpFDEE){ewijcUfazO = true;}
      while(JtujfqLDVJ == JtujfqLDVJ){ETYOrWICts = true;}
      if(dIRxxzPLzU == true){dIRxxzPLzU = false;}
      if(ewrhAZrFKz == true){ewrhAZrFKz = false;}
      if(JKmrkBJsyc == true){JKmrkBJsyc = false;}
      if(TAyfRxRTki == true){TAyfRxRTki = false;}
      if(ZtEiZNBNiF == true){ZtEiZNBNiF = false;}
      if(yFrVzTVZLu == true){yFrVzTVZLu = false;}
      if(pVMVBdyMPD == true){pVMVBdyMPD = false;}
      if(IXTbYPFESW == true){IXTbYPFESW = false;}
      if(AuJsdOHbfy == true){AuJsdOHbfy = false;}
      if(brIprDgihu == true){brIprDgihu = false;}
      if(PnxfxakQxj == true){PnxfxakQxj = false;}
      if(NQGnutMTql == true){NQGnutMTql = false;}
      if(VVgRaQPdiU == true){VVgRaQPdiU = false;}
      if(hWWXHLyduD == true){hWWXHLyduD = false;}
      if(dpewjKkTty == true){dpewjKkTty = false;}
      if(qNquYsHeEf == true){qNquYsHeEf = false;}
      if(KGhbBqtigi == true){KGhbBqtigi = false;}
      if(uhXnIeqjhQ == true){uhXnIeqjhQ = false;}
      if(ewijcUfazO == true){ewijcUfazO = false;}
      if(ETYOrWICts == true){ETYOrWICts = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PQHCNEKIZK
{ 
  void trSHfZOYGO()
  { 
      bool aDOcLtXLNc = false;
      bool mcMEeEXogN = false;
      bool QknYIxsKLm = false;
      bool cNjQtzppFL = false;
      bool orFKVjLraE = false;
      bool CUIWBoOoTO = false;
      bool BMzZzZruKe = false;
      bool NgiVXYecCY = false;
      bool kAiGjIEMPF = false;
      bool kBVqJcreKu = false;
      bool qQhwqUkmyc = false;
      bool KlmtdAcsoq = false;
      bool VffFtMKrLu = false;
      bool fzSGhUSmPP = false;
      bool WROmtyNbNJ = false;
      bool zypTZLkcCe = false;
      bool JsaDwKTfLJ = false;
      bool hpdZTERrKu = false;
      bool yieCwhJygW = false;
      bool iojjGcpKpj = false;
      string iskBEjQamY;
      string iThyASZqVC;
      string ygybqlJDRX;
      string YEcMpWWqwy;
      string MCgCAPyYEM;
      string tBeTXCtJKS;
      string ZNJPzdpUIc;
      string whtXpuzwba;
      string mjzEdWYInn;
      string cMfdXzsRCV;
      string ZdQaQLGHdc;
      string CxgiepOElc;
      string FhfUAiCQAp;
      string BhsFBJTXnR;
      string PRdLkeewyh;
      string kneZQqxuah;
      string JpHmaTEbhV;
      string VpQTMmDUiV;
      string DFSRFNDSnJ;
      string MYxFIZJpjR;
      if(iskBEjQamY == ZdQaQLGHdc){aDOcLtXLNc = true;}
      else if(ZdQaQLGHdc == iskBEjQamY){qQhwqUkmyc = true;}
      if(iThyASZqVC == CxgiepOElc){mcMEeEXogN = true;}
      else if(CxgiepOElc == iThyASZqVC){KlmtdAcsoq = true;}
      if(ygybqlJDRX == FhfUAiCQAp){QknYIxsKLm = true;}
      else if(FhfUAiCQAp == ygybqlJDRX){VffFtMKrLu = true;}
      if(YEcMpWWqwy == BhsFBJTXnR){cNjQtzppFL = true;}
      else if(BhsFBJTXnR == YEcMpWWqwy){fzSGhUSmPP = true;}
      if(MCgCAPyYEM == PRdLkeewyh){orFKVjLraE = true;}
      else if(PRdLkeewyh == MCgCAPyYEM){WROmtyNbNJ = true;}
      if(tBeTXCtJKS == kneZQqxuah){CUIWBoOoTO = true;}
      else if(kneZQqxuah == tBeTXCtJKS){zypTZLkcCe = true;}
      if(ZNJPzdpUIc == JpHmaTEbhV){BMzZzZruKe = true;}
      else if(JpHmaTEbhV == ZNJPzdpUIc){JsaDwKTfLJ = true;}
      if(whtXpuzwba == VpQTMmDUiV){NgiVXYecCY = true;}
      if(mjzEdWYInn == DFSRFNDSnJ){kAiGjIEMPF = true;}
      if(cMfdXzsRCV == MYxFIZJpjR){kBVqJcreKu = true;}
      while(VpQTMmDUiV == whtXpuzwba){hpdZTERrKu = true;}
      while(DFSRFNDSnJ == DFSRFNDSnJ){yieCwhJygW = true;}
      while(MYxFIZJpjR == MYxFIZJpjR){iojjGcpKpj = true;}
      if(aDOcLtXLNc == true){aDOcLtXLNc = false;}
      if(mcMEeEXogN == true){mcMEeEXogN = false;}
      if(QknYIxsKLm == true){QknYIxsKLm = false;}
      if(cNjQtzppFL == true){cNjQtzppFL = false;}
      if(orFKVjLraE == true){orFKVjLraE = false;}
      if(CUIWBoOoTO == true){CUIWBoOoTO = false;}
      if(BMzZzZruKe == true){BMzZzZruKe = false;}
      if(NgiVXYecCY == true){NgiVXYecCY = false;}
      if(kAiGjIEMPF == true){kAiGjIEMPF = false;}
      if(kBVqJcreKu == true){kBVqJcreKu = false;}
      if(qQhwqUkmyc == true){qQhwqUkmyc = false;}
      if(KlmtdAcsoq == true){KlmtdAcsoq = false;}
      if(VffFtMKrLu == true){VffFtMKrLu = false;}
      if(fzSGhUSmPP == true){fzSGhUSmPP = false;}
      if(WROmtyNbNJ == true){WROmtyNbNJ = false;}
      if(zypTZLkcCe == true){zypTZLkcCe = false;}
      if(JsaDwKTfLJ == true){JsaDwKTfLJ = false;}
      if(hpdZTERrKu == true){hpdZTERrKu = false;}
      if(yieCwhJygW == true){yieCwhJygW = false;}
      if(iojjGcpKpj == true){iojjGcpKpj = false;}
    } 
}; 
