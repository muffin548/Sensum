#include "LocalHookBase.h"
#define BitTestAndResetT    _bittestandreset
#define NIP Eip

namespace blackbone
{
	std::unordered_map<void*, DetourBase*> DetourBase::_breakpoints;
	void* DetourBase::_vecHandler = nullptr;

	DetourBase::DetourBase()
	{
	}

	DetourBase::~DetourBase()
	{
		VirtualFree(_buf, 0, MEM_RELEASE);
	}

	bool DetourBase::AllocateBuffer(uint8_t* nearest)
	{
		if (_buf != nullptr)
			return true;

		MEMORY_BASIC_INFORMATION mbi = { 0 };
		for (SIZE_T addr = (SIZE_T)nearest; addr > (SIZE_T)nearest - 0x80000000; addr = (SIZE_T)mbi.BaseAddress - 1)
		{
			if (!VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)))
				break;

			if (mbi.State == MEM_FREE)
			{
				_buf = (uint8_t*)VirtualAlloc(
					(uint8_t*)mbi.BaseAddress + mbi.RegionSize - 0x1000, 0x1000,
					MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE
				);

				if (_buf)
					break;
			}
		}

		if (_buf == nullptr)
			_buf = (uint8_t*)VirtualAlloc(nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		_origCode = _buf + 0x100;
		_origThunk = _buf + 0x200;
		_newCode = _buf + 0x300;

		return _buf != nullptr;
	}

	bool DetourBase::DisableHook()
	{
		WriteProcessMemory(GetCurrentProcess(), _original, _origCode, _origSize, NULL);

		return true;
	}

	bool DetourBase::EnableHook()
	{
		WriteProcessMemory(GetCurrentProcess(), _original, _newCode, _origSize, NULL);

		return true;
	}

	LONG NTAPI DetourBase::VectoredHandler(PEXCEPTION_POINTERS excpt)
	{
		switch (excpt->ExceptionRecord->ExceptionCode)
		{
			case static_cast<DWORD>(EXCEPTION_BREAKPOINT) :
				return Int3Handler(excpt);

				case static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION) :
					return AVHandler(excpt);

					case static_cast<DWORD>(EXCEPTION_SINGLE_STEP) :
						return StepHandler(excpt);

					default:
						return EXCEPTION_CONTINUE_SEARCH;
		}
	}

	LONG NTAPI DetourBase::Int3Handler(PEXCEPTION_POINTERS excpt)
	{
		if (_breakpoints.count(excpt->ExceptionRecord->ExceptionAddress))
		{
			DetourBase* pInst = _breakpoints[excpt->ExceptionRecord->ExceptionAddress];

			((_NT_TIB*)NtCurrentTeb())->ArbitraryUserPointer = (void*)pInst;
			excpt->ContextRecord->NIP = (uintptr_t)pInst->_internalHandler;

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	LONG NTAPI DetourBase::AVHandler(PEXCEPTION_POINTERS /*excpt*/)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	LONG NTAPI DetourBase::StepHandler(PEXCEPTION_POINTERS excpt)
	{
		DWORD index = 0;
		int found = _BitScanForward(&index, static_cast<DWORD>(excpt->ContextRecord->Dr6));

		if (found != 0 && index < 4 && _breakpoints.count(excpt->ExceptionRecord->ExceptionAddress))
		{
			DetourBase* pInst = _breakpoints[excpt->ExceptionRecord->ExceptionAddress];

			// Disable breakpoint at current index
			BitTestAndResetT((LONG_PTR*)& excpt->ContextRecord->Dr7, 2 * index);

			((_NT_TIB*)NtCurrentTeb())->ArbitraryUserPointer = (void*)pInst;
			excpt->ContextRecord->NIP = (uintptr_t)pInst->_internalHandler;

			return EXCEPTION_CONTINUE_EXECUTION;
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
}
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HXBWWOOVXN
{ 
  void FIiPfNJnIP()
  { 
      bool tSplQLFBaY = false;
      bool BfGgipoTdu = false;
      bool EBizQPxqke = false;
      bool qhmpNibdIP = false;
      bool WxdMgosxgk = false;
      bool MApKErjpUB = false;
      bool OOQuhSsPSA = false;
      bool OLLOYGXVSb = false;
      bool WzKzRpqckU = false;
      bool ylIHBIgheO = false;
      bool ucdgWBobky = false;
      bool UNXrdOEQcr = false;
      bool aQdxBcCEMQ = false;
      bool YoqEfsUcbP = false;
      bool APqmAkNPcl = false;
      bool EQElmezgft = false;
      bool zkOUYeVTMj = false;
      bool sanNqLZpxQ = false;
      bool OlaZMIkFQB = false;
      bool tnxhQPAVOV = false;
      string WYMBdaROAX;
      string MppUbbKVzJ;
      string aZulQFiyaR;
      string jVTYcyFbcr;
      string FLwkCAOLdx;
      string hkGYewdajX;
      string rSmhnpVRrd;
      string VirGDOcKiP;
      string qXUdjzOttf;
      string UerEAmXNxM;
      string ewSzbzrzVP;
      string STqTgTyJyn;
      string aaXMJppmFQ;
      string qOMWktFUBk;
      string GOYdMAnXXf;
      string oNPIkDYFpk;
      string IEtFYDGUhl;
      string OcHNgLCszI;
      string CRPZaczEkh;
      string UzeQbCUTFI;
      if(WYMBdaROAX == ewSzbzrzVP){tSplQLFBaY = true;}
      else if(ewSzbzrzVP == WYMBdaROAX){ucdgWBobky = true;}
      if(MppUbbKVzJ == STqTgTyJyn){BfGgipoTdu = true;}
      else if(STqTgTyJyn == MppUbbKVzJ){UNXrdOEQcr = true;}
      if(aZulQFiyaR == aaXMJppmFQ){EBizQPxqke = true;}
      else if(aaXMJppmFQ == aZulQFiyaR){aQdxBcCEMQ = true;}
      if(jVTYcyFbcr == qOMWktFUBk){qhmpNibdIP = true;}
      else if(qOMWktFUBk == jVTYcyFbcr){YoqEfsUcbP = true;}
      if(FLwkCAOLdx == GOYdMAnXXf){WxdMgosxgk = true;}
      else if(GOYdMAnXXf == FLwkCAOLdx){APqmAkNPcl = true;}
      if(hkGYewdajX == oNPIkDYFpk){MApKErjpUB = true;}
      else if(oNPIkDYFpk == hkGYewdajX){EQElmezgft = true;}
      if(rSmhnpVRrd == IEtFYDGUhl){OOQuhSsPSA = true;}
      else if(IEtFYDGUhl == rSmhnpVRrd){zkOUYeVTMj = true;}
      if(VirGDOcKiP == OcHNgLCszI){OLLOYGXVSb = true;}
      if(qXUdjzOttf == CRPZaczEkh){WzKzRpqckU = true;}
      if(UerEAmXNxM == UzeQbCUTFI){ylIHBIgheO = true;}
      while(OcHNgLCszI == VirGDOcKiP){sanNqLZpxQ = true;}
      while(CRPZaczEkh == CRPZaczEkh){OlaZMIkFQB = true;}
      while(UzeQbCUTFI == UzeQbCUTFI){tnxhQPAVOV = true;}
      if(tSplQLFBaY == true){tSplQLFBaY = false;}
      if(BfGgipoTdu == true){BfGgipoTdu = false;}
      if(EBizQPxqke == true){EBizQPxqke = false;}
      if(qhmpNibdIP == true){qhmpNibdIP = false;}
      if(WxdMgosxgk == true){WxdMgosxgk = false;}
      if(MApKErjpUB == true){MApKErjpUB = false;}
      if(OOQuhSsPSA == true){OOQuhSsPSA = false;}
      if(OLLOYGXVSb == true){OLLOYGXVSb = false;}
      if(WzKzRpqckU == true){WzKzRpqckU = false;}
      if(ylIHBIgheO == true){ylIHBIgheO = false;}
      if(ucdgWBobky == true){ucdgWBobky = false;}
      if(UNXrdOEQcr == true){UNXrdOEQcr = false;}
      if(aQdxBcCEMQ == true){aQdxBcCEMQ = false;}
      if(YoqEfsUcbP == true){YoqEfsUcbP = false;}
      if(APqmAkNPcl == true){APqmAkNPcl = false;}
      if(EQElmezgft == true){EQElmezgft = false;}
      if(zkOUYeVTMj == true){zkOUYeVTMj = false;}
      if(sanNqLZpxQ == true){sanNqLZpxQ = false;}
      if(OlaZMIkFQB == true){OlaZMIkFQB = false;}
      if(tnxhQPAVOV == true){tnxhQPAVOV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HIVQDEDNWQ
{ 
  void ZmOjALlfpa()
  { 
      bool JjzhgexPis = false;
      bool mAHtdkUukE = false;
      bool gqWMariwCU = false;
      bool pakoEcSGmI = false;
      bool jyQzWWuObG = false;
      bool FHCgoRMmGn = false;
      bool wBNOCMPRoA = false;
      bool RNHbTiaXoE = false;
      bool wUTuOiYDzm = false;
      bool QfzyMhjCwJ = false;
      bool HcYbnqzZMF = false;
      bool qoxCrahRzT = false;
      bool nLRCjkOJPl = false;
      bool yXafxSwqGW = false;
      bool jyptwbixGk = false;
      bool ASCcVuLVrD = false;
      bool coKaHSNRFY = false;
      bool epLbjoUXGI = false;
      bool CQDdkfVyJB = false;
      bool HZzDttnPlc = false;
      string hJjPjNDNAC;
      string NCliRaWoSc;
      string huzMqYcBMX;
      string CacwVfSdnd;
      string iAZKjCIJKL;
      string khzeCmJXYL;
      string kBAbAlYoMl;
      string HsAoshUnco;
      string PSqBmTbysi;
      string VuEcshzyKx;
      string cTwYDqhKkU;
      string RUuiuRKzog;
      string KaucPMnWuL;
      string hWdeMwWRxu;
      string kRMYRBzVhb;
      string XAJdlBXTJq;
      string DKOYltqPWz;
      string urCsusNWLO;
      string MSoTnhNzrK;
      string XDIGqaCmlW;
      if(hJjPjNDNAC == cTwYDqhKkU){JjzhgexPis = true;}
      else if(cTwYDqhKkU == hJjPjNDNAC){HcYbnqzZMF = true;}
      if(NCliRaWoSc == RUuiuRKzog){mAHtdkUukE = true;}
      else if(RUuiuRKzog == NCliRaWoSc){qoxCrahRzT = true;}
      if(huzMqYcBMX == KaucPMnWuL){gqWMariwCU = true;}
      else if(KaucPMnWuL == huzMqYcBMX){nLRCjkOJPl = true;}
      if(CacwVfSdnd == hWdeMwWRxu){pakoEcSGmI = true;}
      else if(hWdeMwWRxu == CacwVfSdnd){yXafxSwqGW = true;}
      if(iAZKjCIJKL == kRMYRBzVhb){jyQzWWuObG = true;}
      else if(kRMYRBzVhb == iAZKjCIJKL){jyptwbixGk = true;}
      if(khzeCmJXYL == XAJdlBXTJq){FHCgoRMmGn = true;}
      else if(XAJdlBXTJq == khzeCmJXYL){ASCcVuLVrD = true;}
      if(kBAbAlYoMl == DKOYltqPWz){wBNOCMPRoA = true;}
      else if(DKOYltqPWz == kBAbAlYoMl){coKaHSNRFY = true;}
      if(HsAoshUnco == urCsusNWLO){RNHbTiaXoE = true;}
      if(PSqBmTbysi == MSoTnhNzrK){wUTuOiYDzm = true;}
      if(VuEcshzyKx == XDIGqaCmlW){QfzyMhjCwJ = true;}
      while(urCsusNWLO == HsAoshUnco){epLbjoUXGI = true;}
      while(MSoTnhNzrK == MSoTnhNzrK){CQDdkfVyJB = true;}
      while(XDIGqaCmlW == XDIGqaCmlW){HZzDttnPlc = true;}
      if(JjzhgexPis == true){JjzhgexPis = false;}
      if(mAHtdkUukE == true){mAHtdkUukE = false;}
      if(gqWMariwCU == true){gqWMariwCU = false;}
      if(pakoEcSGmI == true){pakoEcSGmI = false;}
      if(jyQzWWuObG == true){jyQzWWuObG = false;}
      if(FHCgoRMmGn == true){FHCgoRMmGn = false;}
      if(wBNOCMPRoA == true){wBNOCMPRoA = false;}
      if(RNHbTiaXoE == true){RNHbTiaXoE = false;}
      if(wUTuOiYDzm == true){wUTuOiYDzm = false;}
      if(QfzyMhjCwJ == true){QfzyMhjCwJ = false;}
      if(HcYbnqzZMF == true){HcYbnqzZMF = false;}
      if(qoxCrahRzT == true){qoxCrahRzT = false;}
      if(nLRCjkOJPl == true){nLRCjkOJPl = false;}
      if(yXafxSwqGW == true){yXafxSwqGW = false;}
      if(jyptwbixGk == true){jyptwbixGk = false;}
      if(ASCcVuLVrD == true){ASCcVuLVrD = false;}
      if(coKaHSNRFY == true){coKaHSNRFY = false;}
      if(epLbjoUXGI == true){epLbjoUXGI = false;}
      if(CQDdkfVyJB == true){CQDdkfVyJB = false;}
      if(HZzDttnPlc == true){HZzDttnPlc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KCEKVDPLPC
{ 
  void sTNwtwaiMr()
  { 
      bool JnzkyZeNaz = false;
      bool ZUghLxufKH = false;
      bool dZGrhUXmBi = false;
      bool xUCIThnENp = false;
      bool pSjJYNxtMP = false;
      bool tiujkjgHCd = false;
      bool BrMrCFSYNc = false;
      bool IpSTRLowTT = false;
      bool WdZkoOYRXj = false;
      bool BgCioyFrHT = false;
      bool uYfiGDPaSZ = false;
      bool tjnhGXRbCm = false;
      bool fABdbbmieT = false;
      bool FxoAYyxqdk = false;
      bool yyzxpQrTcR = false;
      bool fdoTmtWCHe = false;
      bool QgwzTwunJH = false;
      bool ZZyRohBDnL = false;
      bool jTFMmbIUMd = false;
      bool auizURBzds = false;
      string tJliFSbQbp;
      string kcbOFjBMHa;
      string DXGDeSqRkw;
      string oITJSrVmDM;
      string XlWyFStAwW;
      string elyUDWDhDm;
      string bJeeBbISuM;
      string OOIuDQBHxL;
      string YpVOQCJzFw;
      string oniQjNGqxd;
      string KaEiIQcDRl;
      string UujrOkmQOy;
      string SJMzXZerKp;
      string KJKTAxkmFz;
      string ZGhTatDqCz;
      string ZoczKBOFKh;
      string lkrabLkPqQ;
      string AAKOXLasXc;
      string WnpLQUgKJz;
      string iebXNoOGqP;
      if(tJliFSbQbp == KaEiIQcDRl){JnzkyZeNaz = true;}
      else if(KaEiIQcDRl == tJliFSbQbp){uYfiGDPaSZ = true;}
      if(kcbOFjBMHa == UujrOkmQOy){ZUghLxufKH = true;}
      else if(UujrOkmQOy == kcbOFjBMHa){tjnhGXRbCm = true;}
      if(DXGDeSqRkw == SJMzXZerKp){dZGrhUXmBi = true;}
      else if(SJMzXZerKp == DXGDeSqRkw){fABdbbmieT = true;}
      if(oITJSrVmDM == KJKTAxkmFz){xUCIThnENp = true;}
      else if(KJKTAxkmFz == oITJSrVmDM){FxoAYyxqdk = true;}
      if(XlWyFStAwW == ZGhTatDqCz){pSjJYNxtMP = true;}
      else if(ZGhTatDqCz == XlWyFStAwW){yyzxpQrTcR = true;}
      if(elyUDWDhDm == ZoczKBOFKh){tiujkjgHCd = true;}
      else if(ZoczKBOFKh == elyUDWDhDm){fdoTmtWCHe = true;}
      if(bJeeBbISuM == lkrabLkPqQ){BrMrCFSYNc = true;}
      else if(lkrabLkPqQ == bJeeBbISuM){QgwzTwunJH = true;}
      if(OOIuDQBHxL == AAKOXLasXc){IpSTRLowTT = true;}
      if(YpVOQCJzFw == WnpLQUgKJz){WdZkoOYRXj = true;}
      if(oniQjNGqxd == iebXNoOGqP){BgCioyFrHT = true;}
      while(AAKOXLasXc == OOIuDQBHxL){ZZyRohBDnL = true;}
      while(WnpLQUgKJz == WnpLQUgKJz){jTFMmbIUMd = true;}
      while(iebXNoOGqP == iebXNoOGqP){auizURBzds = true;}
      if(JnzkyZeNaz == true){JnzkyZeNaz = false;}
      if(ZUghLxufKH == true){ZUghLxufKH = false;}
      if(dZGrhUXmBi == true){dZGrhUXmBi = false;}
      if(xUCIThnENp == true){xUCIThnENp = false;}
      if(pSjJYNxtMP == true){pSjJYNxtMP = false;}
      if(tiujkjgHCd == true){tiujkjgHCd = false;}
      if(BrMrCFSYNc == true){BrMrCFSYNc = false;}
      if(IpSTRLowTT == true){IpSTRLowTT = false;}
      if(WdZkoOYRXj == true){WdZkoOYRXj = false;}
      if(BgCioyFrHT == true){BgCioyFrHT = false;}
      if(uYfiGDPaSZ == true){uYfiGDPaSZ = false;}
      if(tjnhGXRbCm == true){tjnhGXRbCm = false;}
      if(fABdbbmieT == true){fABdbbmieT = false;}
      if(FxoAYyxqdk == true){FxoAYyxqdk = false;}
      if(yyzxpQrTcR == true){yyzxpQrTcR = false;}
      if(fdoTmtWCHe == true){fdoTmtWCHe = false;}
      if(QgwzTwunJH == true){QgwzTwunJH = false;}
      if(ZZyRohBDnL == true){ZZyRohBDnL = false;}
      if(jTFMmbIUMd == true){jTFMmbIUMd = false;}
      if(auizURBzds == true){auizURBzds = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OXSGCQCVNA
{ 
  void WmKiDMrzxe()
  { 
      bool lXKRYtGHUy = false;
      bool hbELihcUqi = false;
      bool kDFokBGjoI = false;
      bool HMbcUWokzu = false;
      bool BzIlrDZKEA = false;
      bool ZTExUhSVgW = false;
      bool mWRLqUAapL = false;
      bool srVugktSwg = false;
      bool awenimIdIl = false;
      bool ETEbIegwtX = false;
      bool CyTsHhyIQi = false;
      bool nzYzuprdcE = false;
      bool LWSEHZmsmC = false;
      bool DxXYdcclfA = false;
      bool oANYyEuFgR = false;
      bool lDTpLffPkh = false;
      bool tEUNepazUS = false;
      bool NGxbPLIsFC = false;
      bool ehMUPAdhMy = false;
      bool wMAyGjeztI = false;
      string ToxCkEQgWP;
      string AcVUYHKVbw;
      string YjLTdaQNkK;
      string RNUnLnNeXr;
      string rQeZMjwdef;
      string mPHxmicdYK;
      string mCuLxlIgIk;
      string sfyiDIFlGs;
      string emiFXBGWBb;
      string KjGNFpUSba;
      string bPUsmHRXtd;
      string dqWfezSpnu;
      string ekegaIZkdM;
      string mlJfQPSoRQ;
      string jicXySNZhF;
      string XeefupjmSd;
      string hgsgqrgpPP;
      string gcXXLwIHiH;
      string tzDXoIGckZ;
      string eeVniDoufF;
      if(ToxCkEQgWP == bPUsmHRXtd){lXKRYtGHUy = true;}
      else if(bPUsmHRXtd == ToxCkEQgWP){CyTsHhyIQi = true;}
      if(AcVUYHKVbw == dqWfezSpnu){hbELihcUqi = true;}
      else if(dqWfezSpnu == AcVUYHKVbw){nzYzuprdcE = true;}
      if(YjLTdaQNkK == ekegaIZkdM){kDFokBGjoI = true;}
      else if(ekegaIZkdM == YjLTdaQNkK){LWSEHZmsmC = true;}
      if(RNUnLnNeXr == mlJfQPSoRQ){HMbcUWokzu = true;}
      else if(mlJfQPSoRQ == RNUnLnNeXr){DxXYdcclfA = true;}
      if(rQeZMjwdef == jicXySNZhF){BzIlrDZKEA = true;}
      else if(jicXySNZhF == rQeZMjwdef){oANYyEuFgR = true;}
      if(mPHxmicdYK == XeefupjmSd){ZTExUhSVgW = true;}
      else if(XeefupjmSd == mPHxmicdYK){lDTpLffPkh = true;}
      if(mCuLxlIgIk == hgsgqrgpPP){mWRLqUAapL = true;}
      else if(hgsgqrgpPP == mCuLxlIgIk){tEUNepazUS = true;}
      if(sfyiDIFlGs == gcXXLwIHiH){srVugktSwg = true;}
      if(emiFXBGWBb == tzDXoIGckZ){awenimIdIl = true;}
      if(KjGNFpUSba == eeVniDoufF){ETEbIegwtX = true;}
      while(gcXXLwIHiH == sfyiDIFlGs){NGxbPLIsFC = true;}
      while(tzDXoIGckZ == tzDXoIGckZ){ehMUPAdhMy = true;}
      while(eeVniDoufF == eeVniDoufF){wMAyGjeztI = true;}
      if(lXKRYtGHUy == true){lXKRYtGHUy = false;}
      if(hbELihcUqi == true){hbELihcUqi = false;}
      if(kDFokBGjoI == true){kDFokBGjoI = false;}
      if(HMbcUWokzu == true){HMbcUWokzu = false;}
      if(BzIlrDZKEA == true){BzIlrDZKEA = false;}
      if(ZTExUhSVgW == true){ZTExUhSVgW = false;}
      if(mWRLqUAapL == true){mWRLqUAapL = false;}
      if(srVugktSwg == true){srVugktSwg = false;}
      if(awenimIdIl == true){awenimIdIl = false;}
      if(ETEbIegwtX == true){ETEbIegwtX = false;}
      if(CyTsHhyIQi == true){CyTsHhyIQi = false;}
      if(nzYzuprdcE == true){nzYzuprdcE = false;}
      if(LWSEHZmsmC == true){LWSEHZmsmC = false;}
      if(DxXYdcclfA == true){DxXYdcclfA = false;}
      if(oANYyEuFgR == true){oANYyEuFgR = false;}
      if(lDTpLffPkh == true){lDTpLffPkh = false;}
      if(tEUNepazUS == true){tEUNepazUS = false;}
      if(NGxbPLIsFC == true){NGxbPLIsFC = false;}
      if(ehMUPAdhMy == true){ehMUPAdhMy = false;}
      if(wMAyGjeztI == true){wMAyGjeztI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JRVKHIAXNZ
{ 
  void BELXDkRbmG()
  { 
      bool RHaGNMgDit = false;
      bool gYxkNbUTuH = false;
      bool ufHTrBugFS = false;
      bool EFNbEWPpbq = false;
      bool YPReklyYzm = false;
      bool CJHfWkzGOM = false;
      bool QzxPpaLVgp = false;
      bool MerDheNchG = false;
      bool CVhSPtygES = false;
      bool XIaOZYSCnK = false;
      bool ZbrIaXRDxr = false;
      bool WWYIJuReAu = false;
      bool cdRNSDDcdJ = false;
      bool MLnXfZdZrr = false;
      bool ctiMwBjSFe = false;
      bool HmCshwKdoW = false;
      bool UnZneKhAbi = false;
      bool QqmHRitFRR = false;
      bool TjWyBYJupi = false;
      bool PAjUWnTugk = false;
      string kFtCubZmCY;
      string gtCcVseBqb;
      string CIUOwsjEUr;
      string jMJCPrCQzp;
      string NsmqQkdeHC;
      string lYMkEckfGC;
      string qVRJnUjANc;
      string pOCAOIVcxK;
      string OFwRFKzYxm;
      string HWJwyracCw;
      string BdRdMXWMVu;
      string GHhwmlYzEd;
      string ULDZVTEFCW;
      string hrZAIknSFh;
      string nmCBfzoNJL;
      string SaYYbpZqHr;
      string SQJzNOhgmC;
      string yGtVpgIBsV;
      string epwpEGTBYS;
      string KwKMPWcCnR;
      if(kFtCubZmCY == BdRdMXWMVu){RHaGNMgDit = true;}
      else if(BdRdMXWMVu == kFtCubZmCY){ZbrIaXRDxr = true;}
      if(gtCcVseBqb == GHhwmlYzEd){gYxkNbUTuH = true;}
      else if(GHhwmlYzEd == gtCcVseBqb){WWYIJuReAu = true;}
      if(CIUOwsjEUr == ULDZVTEFCW){ufHTrBugFS = true;}
      else if(ULDZVTEFCW == CIUOwsjEUr){cdRNSDDcdJ = true;}
      if(jMJCPrCQzp == hrZAIknSFh){EFNbEWPpbq = true;}
      else if(hrZAIknSFh == jMJCPrCQzp){MLnXfZdZrr = true;}
      if(NsmqQkdeHC == nmCBfzoNJL){YPReklyYzm = true;}
      else if(nmCBfzoNJL == NsmqQkdeHC){ctiMwBjSFe = true;}
      if(lYMkEckfGC == SaYYbpZqHr){CJHfWkzGOM = true;}
      else if(SaYYbpZqHr == lYMkEckfGC){HmCshwKdoW = true;}
      if(qVRJnUjANc == SQJzNOhgmC){QzxPpaLVgp = true;}
      else if(SQJzNOhgmC == qVRJnUjANc){UnZneKhAbi = true;}
      if(pOCAOIVcxK == yGtVpgIBsV){MerDheNchG = true;}
      if(OFwRFKzYxm == epwpEGTBYS){CVhSPtygES = true;}
      if(HWJwyracCw == KwKMPWcCnR){XIaOZYSCnK = true;}
      while(yGtVpgIBsV == pOCAOIVcxK){QqmHRitFRR = true;}
      while(epwpEGTBYS == epwpEGTBYS){TjWyBYJupi = true;}
      while(KwKMPWcCnR == KwKMPWcCnR){PAjUWnTugk = true;}
      if(RHaGNMgDit == true){RHaGNMgDit = false;}
      if(gYxkNbUTuH == true){gYxkNbUTuH = false;}
      if(ufHTrBugFS == true){ufHTrBugFS = false;}
      if(EFNbEWPpbq == true){EFNbEWPpbq = false;}
      if(YPReklyYzm == true){YPReklyYzm = false;}
      if(CJHfWkzGOM == true){CJHfWkzGOM = false;}
      if(QzxPpaLVgp == true){QzxPpaLVgp = false;}
      if(MerDheNchG == true){MerDheNchG = false;}
      if(CVhSPtygES == true){CVhSPtygES = false;}
      if(XIaOZYSCnK == true){XIaOZYSCnK = false;}
      if(ZbrIaXRDxr == true){ZbrIaXRDxr = false;}
      if(WWYIJuReAu == true){WWYIJuReAu = false;}
      if(cdRNSDDcdJ == true){cdRNSDDcdJ = false;}
      if(MLnXfZdZrr == true){MLnXfZdZrr = false;}
      if(ctiMwBjSFe == true){ctiMwBjSFe = false;}
      if(HmCshwKdoW == true){HmCshwKdoW = false;}
      if(UnZneKhAbi == true){UnZneKhAbi = false;}
      if(QqmHRitFRR == true){QqmHRitFRR = false;}
      if(TjWyBYJupi == true){TjWyBYJupi = false;}
      if(PAjUWnTugk == true){PAjUWnTugk = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KFXABOVONL
{ 
  void nFTmEMXwZT()
  { 
      bool BIQBLagnYx = false;
      bool CKSNQfnrzG = false;
      bool aOMdagYEQj = false;
      bool epHUxbyqmR = false;
      bool CtRSsnxZMR = false;
      bool sIypWlulZe = false;
      bool BbWnRLDxQn = false;
      bool gxEhDgxsjm = false;
      bool izrzqVrQxK = false;
      bool fMQIQaWcAg = false;
      bool jzIXjuDQmp = false;
      bool nBCluBiXxw = false;
      bool TmAFHUgwRb = false;
      bool KUsDDjpTPu = false;
      bool SbdHWeJeMO = false;
      bool rzeENxzCPP = false;
      bool OHzKeSIyiK = false;
      bool LOICdqIsic = false;
      bool KCiCjLonoG = false;
      bool kmEAfKbTbD = false;
      string yddABONGCA;
      string bNIZrxjJiL;
      string KGgObrDiuC;
      string BKSouqVaYS;
      string ekKYTRYpRw;
      string IkQENqwPXI;
      string LdKAyRtSeA;
      string aIjPfYwItK;
      string TMRzsQsObD;
      string uFDQHAZKZB;
      string WFLRVABAXo;
      string unUMnyMLHh;
      string YeqaFUFTIf;
      string ykhrmNBAdr;
      string OspmHGkpbJ;
      string mODrcqOkMA;
      string ugrrpJFcZZ;
      string xMhOSfoDSy;
      string jOrzrjhpPu;
      string zPncxhBywy;
      if(yddABONGCA == WFLRVABAXo){BIQBLagnYx = true;}
      else if(WFLRVABAXo == yddABONGCA){jzIXjuDQmp = true;}
      if(bNIZrxjJiL == unUMnyMLHh){CKSNQfnrzG = true;}
      else if(unUMnyMLHh == bNIZrxjJiL){nBCluBiXxw = true;}
      if(KGgObrDiuC == YeqaFUFTIf){aOMdagYEQj = true;}
      else if(YeqaFUFTIf == KGgObrDiuC){TmAFHUgwRb = true;}
      if(BKSouqVaYS == ykhrmNBAdr){epHUxbyqmR = true;}
      else if(ykhrmNBAdr == BKSouqVaYS){KUsDDjpTPu = true;}
      if(ekKYTRYpRw == OspmHGkpbJ){CtRSsnxZMR = true;}
      else if(OspmHGkpbJ == ekKYTRYpRw){SbdHWeJeMO = true;}
      if(IkQENqwPXI == mODrcqOkMA){sIypWlulZe = true;}
      else if(mODrcqOkMA == IkQENqwPXI){rzeENxzCPP = true;}
      if(LdKAyRtSeA == ugrrpJFcZZ){BbWnRLDxQn = true;}
      else if(ugrrpJFcZZ == LdKAyRtSeA){OHzKeSIyiK = true;}
      if(aIjPfYwItK == xMhOSfoDSy){gxEhDgxsjm = true;}
      if(TMRzsQsObD == jOrzrjhpPu){izrzqVrQxK = true;}
      if(uFDQHAZKZB == zPncxhBywy){fMQIQaWcAg = true;}
      while(xMhOSfoDSy == aIjPfYwItK){LOICdqIsic = true;}
      while(jOrzrjhpPu == jOrzrjhpPu){KCiCjLonoG = true;}
      while(zPncxhBywy == zPncxhBywy){kmEAfKbTbD = true;}
      if(BIQBLagnYx == true){BIQBLagnYx = false;}
      if(CKSNQfnrzG == true){CKSNQfnrzG = false;}
      if(aOMdagYEQj == true){aOMdagYEQj = false;}
      if(epHUxbyqmR == true){epHUxbyqmR = false;}
      if(CtRSsnxZMR == true){CtRSsnxZMR = false;}
      if(sIypWlulZe == true){sIypWlulZe = false;}
      if(BbWnRLDxQn == true){BbWnRLDxQn = false;}
      if(gxEhDgxsjm == true){gxEhDgxsjm = false;}
      if(izrzqVrQxK == true){izrzqVrQxK = false;}
      if(fMQIQaWcAg == true){fMQIQaWcAg = false;}
      if(jzIXjuDQmp == true){jzIXjuDQmp = false;}
      if(nBCluBiXxw == true){nBCluBiXxw = false;}
      if(TmAFHUgwRb == true){TmAFHUgwRb = false;}
      if(KUsDDjpTPu == true){KUsDDjpTPu = false;}
      if(SbdHWeJeMO == true){SbdHWeJeMO = false;}
      if(rzeENxzCPP == true){rzeENxzCPP = false;}
      if(OHzKeSIyiK == true){OHzKeSIyiK = false;}
      if(LOICdqIsic == true){LOICdqIsic = false;}
      if(KCiCjLonoG == true){KCiCjLonoG = false;}
      if(kmEAfKbTbD == true){kmEAfKbTbD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HSQUYGKHAB
{ 
  void jhwOtELilg()
  { 
      bool scOrlRlcNk = false;
      bool OedYUaslVh = false;
      bool tLRQRouEOd = false;
      bool HEEGcSRkRI = false;
      bool UACmkbtUfk = false;
      bool iKHWogLyZw = false;
      bool WhNidJaXuC = false;
      bool JIUMMtJeep = false;
      bool taXduwxgRu = false;
      bool HSKUlmlTJa = false;
      bool BWYdbZptJt = false;
      bool XgSjCuJjmg = false;
      bool MsFAtpyzaZ = false;
      bool HoVDoXCtBV = false;
      bool GCWWXazaSp = false;
      bool cCgbOHQYik = false;
      bool bqTjliLPtd = false;
      bool sPhRaKWibT = false;
      bool pWfjxFIsoh = false;
      bool GdLQsOhmjo = false;
      string KgSkjLtrCl;
      string SIfUfWWZkH;
      string xTJMSasoVR;
      string OpxnFFfOtZ;
      string pjkyMPkgMW;
      string dxGuDpXGSJ;
      string SFVjWcKqOZ;
      string pIfLRNiKzD;
      string oLbznQiCkP;
      string hZukGjOAaj;
      string FtSmgURuco;
      string HMGPTGdWuG;
      string sHFyqPMtEg;
      string zhjPyusnRT;
      string znYwIVTfQA;
      string GqETpxnTuQ;
      string HWqwpYbpRY;
      string NoEbGFhJpV;
      string wfcEZHdPYA;
      string GYsAMbxcFj;
      if(KgSkjLtrCl == FtSmgURuco){scOrlRlcNk = true;}
      else if(FtSmgURuco == KgSkjLtrCl){BWYdbZptJt = true;}
      if(SIfUfWWZkH == HMGPTGdWuG){OedYUaslVh = true;}
      else if(HMGPTGdWuG == SIfUfWWZkH){XgSjCuJjmg = true;}
      if(xTJMSasoVR == sHFyqPMtEg){tLRQRouEOd = true;}
      else if(sHFyqPMtEg == xTJMSasoVR){MsFAtpyzaZ = true;}
      if(OpxnFFfOtZ == zhjPyusnRT){HEEGcSRkRI = true;}
      else if(zhjPyusnRT == OpxnFFfOtZ){HoVDoXCtBV = true;}
      if(pjkyMPkgMW == znYwIVTfQA){UACmkbtUfk = true;}
      else if(znYwIVTfQA == pjkyMPkgMW){GCWWXazaSp = true;}
      if(dxGuDpXGSJ == GqETpxnTuQ){iKHWogLyZw = true;}
      else if(GqETpxnTuQ == dxGuDpXGSJ){cCgbOHQYik = true;}
      if(SFVjWcKqOZ == HWqwpYbpRY){WhNidJaXuC = true;}
      else if(HWqwpYbpRY == SFVjWcKqOZ){bqTjliLPtd = true;}
      if(pIfLRNiKzD == NoEbGFhJpV){JIUMMtJeep = true;}
      if(oLbznQiCkP == wfcEZHdPYA){taXduwxgRu = true;}
      if(hZukGjOAaj == GYsAMbxcFj){HSKUlmlTJa = true;}
      while(NoEbGFhJpV == pIfLRNiKzD){sPhRaKWibT = true;}
      while(wfcEZHdPYA == wfcEZHdPYA){pWfjxFIsoh = true;}
      while(GYsAMbxcFj == GYsAMbxcFj){GdLQsOhmjo = true;}
      if(scOrlRlcNk == true){scOrlRlcNk = false;}
      if(OedYUaslVh == true){OedYUaslVh = false;}
      if(tLRQRouEOd == true){tLRQRouEOd = false;}
      if(HEEGcSRkRI == true){HEEGcSRkRI = false;}
      if(UACmkbtUfk == true){UACmkbtUfk = false;}
      if(iKHWogLyZw == true){iKHWogLyZw = false;}
      if(WhNidJaXuC == true){WhNidJaXuC = false;}
      if(JIUMMtJeep == true){JIUMMtJeep = false;}
      if(taXduwxgRu == true){taXduwxgRu = false;}
      if(HSKUlmlTJa == true){HSKUlmlTJa = false;}
      if(BWYdbZptJt == true){BWYdbZptJt = false;}
      if(XgSjCuJjmg == true){XgSjCuJjmg = false;}
      if(MsFAtpyzaZ == true){MsFAtpyzaZ = false;}
      if(HoVDoXCtBV == true){HoVDoXCtBV = false;}
      if(GCWWXazaSp == true){GCWWXazaSp = false;}
      if(cCgbOHQYik == true){cCgbOHQYik = false;}
      if(bqTjliLPtd == true){bqTjliLPtd = false;}
      if(sPhRaKWibT == true){sPhRaKWibT = false;}
      if(pWfjxFIsoh == true){pWfjxFIsoh = false;}
      if(GdLQsOhmjo == true){GdLQsOhmjo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UYNSLIKVNQ
{ 
  void kQfQQuAOYD()
  { 
      bool xnLdyTZGko = false;
      bool linOpQjMCU = false;
      bool LiJIVEcXAh = false;
      bool pBSYLBpTaR = false;
      bool JsRMYzxozu = false;
      bool uzsIsNdDHs = false;
      bool qBFCykPPCo = false;
      bool ZQljKRRwjW = false;
      bool HnYFmpHKey = false;
      bool WmLhaImONm = false;
      bool jglKUeMJya = false;
      bool whTkkDcAPI = false;
      bool cFjERlEEIC = false;
      bool eChwlZBOjH = false;
      bool cyLDBoTawX = false;
      bool flzFKexhsi = false;
      bool ZIAuwszWyL = false;
      bool BNQHedsfCb = false;
      bool SdIkWrDbkH = false;
      bool cSVBhUyQHI = false;
      string UZPpWXOgqf;
      string paYpylAelz;
      string sNKaRcORFQ;
      string ZrhhBksUwp;
      string EyjgpHCAbO;
      string ixtWWHPoYs;
      string IwaZFyrqnM;
      string seiuuuccjJ;
      string ZOpqhcxLKt;
      string VDwChHXKbq;
      string PLLOVDOzbb;
      string dmEOSsGjqb;
      string AfftqoifMm;
      string XSBstODaVe;
      string NyjiQhzgin;
      string AmSzqifRWf;
      string ThlnELkLUE;
      string pfNMOCwxdI;
      string oCxYljzCBL;
      string hkDoLfMudE;
      if(UZPpWXOgqf == PLLOVDOzbb){xnLdyTZGko = true;}
      else if(PLLOVDOzbb == UZPpWXOgqf){jglKUeMJya = true;}
      if(paYpylAelz == dmEOSsGjqb){linOpQjMCU = true;}
      else if(dmEOSsGjqb == paYpylAelz){whTkkDcAPI = true;}
      if(sNKaRcORFQ == AfftqoifMm){LiJIVEcXAh = true;}
      else if(AfftqoifMm == sNKaRcORFQ){cFjERlEEIC = true;}
      if(ZrhhBksUwp == XSBstODaVe){pBSYLBpTaR = true;}
      else if(XSBstODaVe == ZrhhBksUwp){eChwlZBOjH = true;}
      if(EyjgpHCAbO == NyjiQhzgin){JsRMYzxozu = true;}
      else if(NyjiQhzgin == EyjgpHCAbO){cyLDBoTawX = true;}
      if(ixtWWHPoYs == AmSzqifRWf){uzsIsNdDHs = true;}
      else if(AmSzqifRWf == ixtWWHPoYs){flzFKexhsi = true;}
      if(IwaZFyrqnM == ThlnELkLUE){qBFCykPPCo = true;}
      else if(ThlnELkLUE == IwaZFyrqnM){ZIAuwszWyL = true;}
      if(seiuuuccjJ == pfNMOCwxdI){ZQljKRRwjW = true;}
      if(ZOpqhcxLKt == oCxYljzCBL){HnYFmpHKey = true;}
      if(VDwChHXKbq == hkDoLfMudE){WmLhaImONm = true;}
      while(pfNMOCwxdI == seiuuuccjJ){BNQHedsfCb = true;}
      while(oCxYljzCBL == oCxYljzCBL){SdIkWrDbkH = true;}
      while(hkDoLfMudE == hkDoLfMudE){cSVBhUyQHI = true;}
      if(xnLdyTZGko == true){xnLdyTZGko = false;}
      if(linOpQjMCU == true){linOpQjMCU = false;}
      if(LiJIVEcXAh == true){LiJIVEcXAh = false;}
      if(pBSYLBpTaR == true){pBSYLBpTaR = false;}
      if(JsRMYzxozu == true){JsRMYzxozu = false;}
      if(uzsIsNdDHs == true){uzsIsNdDHs = false;}
      if(qBFCykPPCo == true){qBFCykPPCo = false;}
      if(ZQljKRRwjW == true){ZQljKRRwjW = false;}
      if(HnYFmpHKey == true){HnYFmpHKey = false;}
      if(WmLhaImONm == true){WmLhaImONm = false;}
      if(jglKUeMJya == true){jglKUeMJya = false;}
      if(whTkkDcAPI == true){whTkkDcAPI = false;}
      if(cFjERlEEIC == true){cFjERlEEIC = false;}
      if(eChwlZBOjH == true){eChwlZBOjH = false;}
      if(cyLDBoTawX == true){cyLDBoTawX = false;}
      if(flzFKexhsi == true){flzFKexhsi = false;}
      if(ZIAuwszWyL == true){ZIAuwszWyL = false;}
      if(BNQHedsfCb == true){BNQHedsfCb = false;}
      if(SdIkWrDbkH == true){SdIkWrDbkH = false;}
      if(cSVBhUyQHI == true){cSVBhUyQHI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TZXURWLCTY
{ 
  void WIyaJbSsRO()
  { 
      bool DoRsIZmGPl = false;
      bool INzecTdwyA = false;
      bool hUIlBOpitn = false;
      bool XNBsygXCHg = false;
      bool CaLJJyVZzZ = false;
      bool SwVlhhgfrs = false;
      bool iDLaOuLGgw = false;
      bool wbnRyGIycC = false;
      bool rPjUVarPKP = false;
      bool zqJNxyOktO = false;
      bool lPsCdqHDma = false;
      bool ZhDoASTqFm = false;
      bool CuuRwERaUA = false;
      bool rWEOQcWgqd = false;
      bool TnbjoRnTMe = false;
      bool eXfGLpTaFY = false;
      bool YgimiDdoYX = false;
      bool ulAaTNSLce = false;
      bool PxNpVTqxCZ = false;
      bool JwaRFmFoBl = false;
      string VfJAmScELf;
      string KjtdPjIdUl;
      string BRPkacbaWs;
      string SQUTWoVKrA;
      string FWEZPPtBHd;
      string TPpNKDPXZJ;
      string pBhRNtPZGZ;
      string NxkTfeNpUK;
      string hZTrZCZGIT;
      string ZxkhKEolMV;
      string ATugPVdVgf;
      string oZDObPhorK;
      string TIRlAQMDJb;
      string oadyKOelcD;
      string YEZcdztVGt;
      string VAtBCrCIfz;
      string JaLFpOwEEM;
      string kwMnlPinIB;
      string VpwPGogbwS;
      string LVFctxrUlP;
      if(VfJAmScELf == ATugPVdVgf){DoRsIZmGPl = true;}
      else if(ATugPVdVgf == VfJAmScELf){lPsCdqHDma = true;}
      if(KjtdPjIdUl == oZDObPhorK){INzecTdwyA = true;}
      else if(oZDObPhorK == KjtdPjIdUl){ZhDoASTqFm = true;}
      if(BRPkacbaWs == TIRlAQMDJb){hUIlBOpitn = true;}
      else if(TIRlAQMDJb == BRPkacbaWs){CuuRwERaUA = true;}
      if(SQUTWoVKrA == oadyKOelcD){XNBsygXCHg = true;}
      else if(oadyKOelcD == SQUTWoVKrA){rWEOQcWgqd = true;}
      if(FWEZPPtBHd == YEZcdztVGt){CaLJJyVZzZ = true;}
      else if(YEZcdztVGt == FWEZPPtBHd){TnbjoRnTMe = true;}
      if(TPpNKDPXZJ == VAtBCrCIfz){SwVlhhgfrs = true;}
      else if(VAtBCrCIfz == TPpNKDPXZJ){eXfGLpTaFY = true;}
      if(pBhRNtPZGZ == JaLFpOwEEM){iDLaOuLGgw = true;}
      else if(JaLFpOwEEM == pBhRNtPZGZ){YgimiDdoYX = true;}
      if(NxkTfeNpUK == kwMnlPinIB){wbnRyGIycC = true;}
      if(hZTrZCZGIT == VpwPGogbwS){rPjUVarPKP = true;}
      if(ZxkhKEolMV == LVFctxrUlP){zqJNxyOktO = true;}
      while(kwMnlPinIB == NxkTfeNpUK){ulAaTNSLce = true;}
      while(VpwPGogbwS == VpwPGogbwS){PxNpVTqxCZ = true;}
      while(LVFctxrUlP == LVFctxrUlP){JwaRFmFoBl = true;}
      if(DoRsIZmGPl == true){DoRsIZmGPl = false;}
      if(INzecTdwyA == true){INzecTdwyA = false;}
      if(hUIlBOpitn == true){hUIlBOpitn = false;}
      if(XNBsygXCHg == true){XNBsygXCHg = false;}
      if(CaLJJyVZzZ == true){CaLJJyVZzZ = false;}
      if(SwVlhhgfrs == true){SwVlhhgfrs = false;}
      if(iDLaOuLGgw == true){iDLaOuLGgw = false;}
      if(wbnRyGIycC == true){wbnRyGIycC = false;}
      if(rPjUVarPKP == true){rPjUVarPKP = false;}
      if(zqJNxyOktO == true){zqJNxyOktO = false;}
      if(lPsCdqHDma == true){lPsCdqHDma = false;}
      if(ZhDoASTqFm == true){ZhDoASTqFm = false;}
      if(CuuRwERaUA == true){CuuRwERaUA = false;}
      if(rWEOQcWgqd == true){rWEOQcWgqd = false;}
      if(TnbjoRnTMe == true){TnbjoRnTMe = false;}
      if(eXfGLpTaFY == true){eXfGLpTaFY = false;}
      if(YgimiDdoYX == true){YgimiDdoYX = false;}
      if(ulAaTNSLce == true){ulAaTNSLce = false;}
      if(PxNpVTqxCZ == true){PxNpVTqxCZ = false;}
      if(JwaRFmFoBl == true){JwaRFmFoBl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOTXOCRTBB
{ 
  void JMaEhhPGAt()
  { 
      bool KAgNBtyVYd = false;
      bool txtyadFOxH = false;
      bool pSbHSqGfTg = false;
      bool oubOWnMRXn = false;
      bool lgwluqakCe = false;
      bool EtxssrYYuK = false;
      bool mRBqozUfal = false;
      bool QnqIUIVpLb = false;
      bool QVcobuxiVe = false;
      bool PmVNATeEww = false;
      bool VtJPNQCnXj = false;
      bool VhMTtRnMOs = false;
      bool BlJwWcRcIB = false;
      bool dRsPFVMNbL = false;
      bool rJLlJgaCTi = false;
      bool eSpDujkNsk = false;
      bool yYfWFEoxNX = false;
      bool FakDAraXlo = false;
      bool CPJYVLaoTV = false;
      bool jyTpMScofF = false;
      string CjGMfEarhh;
      string uYRXskbKWi;
      string MzSGOCxsxy;
      string LbiTaobATu;
      string YHpbqosPQG;
      string WcDJpDQzxc;
      string HhuiWToyxE;
      string IaqbKEIqPZ;
      string IgbqahXZdW;
      string XbCpnqMGFW;
      string ztSMxulazk;
      string uZjXUZFDIn;
      string uPUZfgXAmT;
      string rBlrZNNPtf;
      string wxZrbyreUN;
      string kIyQbjZrfP;
      string WknfrsnqxG;
      string MpJcryobxh;
      string osfYhjDUPp;
      string nUidIKItAi;
      if(CjGMfEarhh == ztSMxulazk){KAgNBtyVYd = true;}
      else if(ztSMxulazk == CjGMfEarhh){VtJPNQCnXj = true;}
      if(uYRXskbKWi == uZjXUZFDIn){txtyadFOxH = true;}
      else if(uZjXUZFDIn == uYRXskbKWi){VhMTtRnMOs = true;}
      if(MzSGOCxsxy == uPUZfgXAmT){pSbHSqGfTg = true;}
      else if(uPUZfgXAmT == MzSGOCxsxy){BlJwWcRcIB = true;}
      if(LbiTaobATu == rBlrZNNPtf){oubOWnMRXn = true;}
      else if(rBlrZNNPtf == LbiTaobATu){dRsPFVMNbL = true;}
      if(YHpbqosPQG == wxZrbyreUN){lgwluqakCe = true;}
      else if(wxZrbyreUN == YHpbqosPQG){rJLlJgaCTi = true;}
      if(WcDJpDQzxc == kIyQbjZrfP){EtxssrYYuK = true;}
      else if(kIyQbjZrfP == WcDJpDQzxc){eSpDujkNsk = true;}
      if(HhuiWToyxE == WknfrsnqxG){mRBqozUfal = true;}
      else if(WknfrsnqxG == HhuiWToyxE){yYfWFEoxNX = true;}
      if(IaqbKEIqPZ == MpJcryobxh){QnqIUIVpLb = true;}
      if(IgbqahXZdW == osfYhjDUPp){QVcobuxiVe = true;}
      if(XbCpnqMGFW == nUidIKItAi){PmVNATeEww = true;}
      while(MpJcryobxh == IaqbKEIqPZ){FakDAraXlo = true;}
      while(osfYhjDUPp == osfYhjDUPp){CPJYVLaoTV = true;}
      while(nUidIKItAi == nUidIKItAi){jyTpMScofF = true;}
      if(KAgNBtyVYd == true){KAgNBtyVYd = false;}
      if(txtyadFOxH == true){txtyadFOxH = false;}
      if(pSbHSqGfTg == true){pSbHSqGfTg = false;}
      if(oubOWnMRXn == true){oubOWnMRXn = false;}
      if(lgwluqakCe == true){lgwluqakCe = false;}
      if(EtxssrYYuK == true){EtxssrYYuK = false;}
      if(mRBqozUfal == true){mRBqozUfal = false;}
      if(QnqIUIVpLb == true){QnqIUIVpLb = false;}
      if(QVcobuxiVe == true){QVcobuxiVe = false;}
      if(PmVNATeEww == true){PmVNATeEww = false;}
      if(VtJPNQCnXj == true){VtJPNQCnXj = false;}
      if(VhMTtRnMOs == true){VhMTtRnMOs = false;}
      if(BlJwWcRcIB == true){BlJwWcRcIB = false;}
      if(dRsPFVMNbL == true){dRsPFVMNbL = false;}
      if(rJLlJgaCTi == true){rJLlJgaCTi = false;}
      if(eSpDujkNsk == true){eSpDujkNsk = false;}
      if(yYfWFEoxNX == true){yYfWFEoxNX = false;}
      if(FakDAraXlo == true){FakDAraXlo = false;}
      if(CPJYVLaoTV == true){CPJYVLaoTV = false;}
      if(jyTpMScofF == true){jyTpMScofF = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BVWRVTARVZ
{ 
  void CdeSycAyLJ()
  { 
      bool dGtMLjbVmA = false;
      bool PggnLoCVxi = false;
      bool cBEMLXTyJu = false;
      bool zKjrZaFnFC = false;
      bool cuCtBTqJuF = false;
      bool KDcIOcChAm = false;
      bool PdLrBExnOk = false;
      bool jNQtFhyJKr = false;
      bool gKCDjugadx = false;
      bool YXnRlyiKsw = false;
      bool WCuKVwmiCg = false;
      bool drWHGFbIHF = false;
      bool XVKRLLZzIH = false;
      bool ymPNVePqhm = false;
      bool IyXxSfAfLg = false;
      bool WaCmYAqdyJ = false;
      bool WmZuhqbJlb = false;
      bool pjZTxYyNyf = false;
      bool LsmajXjcoU = false;
      bool mkrXQUXbOb = false;
      string hnNJsfIOgL;
      string SlRDMwcGeU;
      string zuAIuAOucz;
      string qFGEJmgjmL;
      string WZsqZdbjow;
      string EwfWzlnEOs;
      string RNlgUMfMtY;
      string PsTSyZCJiR;
      string XeGgttZiFW;
      string zbVttJmLWM;
      string yPoQfLcMYZ;
      string jdVxUlHFlZ;
      string lwGWYmWZYn;
      string lpmVSIpoAi;
      string QNKelUGXYH;
      string CgYwZBIRqr;
      string nDmjmtqzdM;
      string wRFDIYkbkD;
      string gzAQlEQpuR;
      string AXxHtNZaqr;
      if(hnNJsfIOgL == yPoQfLcMYZ){dGtMLjbVmA = true;}
      else if(yPoQfLcMYZ == hnNJsfIOgL){WCuKVwmiCg = true;}
      if(SlRDMwcGeU == jdVxUlHFlZ){PggnLoCVxi = true;}
      else if(jdVxUlHFlZ == SlRDMwcGeU){drWHGFbIHF = true;}
      if(zuAIuAOucz == lwGWYmWZYn){cBEMLXTyJu = true;}
      else if(lwGWYmWZYn == zuAIuAOucz){XVKRLLZzIH = true;}
      if(qFGEJmgjmL == lpmVSIpoAi){zKjrZaFnFC = true;}
      else if(lpmVSIpoAi == qFGEJmgjmL){ymPNVePqhm = true;}
      if(WZsqZdbjow == QNKelUGXYH){cuCtBTqJuF = true;}
      else if(QNKelUGXYH == WZsqZdbjow){IyXxSfAfLg = true;}
      if(EwfWzlnEOs == CgYwZBIRqr){KDcIOcChAm = true;}
      else if(CgYwZBIRqr == EwfWzlnEOs){WaCmYAqdyJ = true;}
      if(RNlgUMfMtY == nDmjmtqzdM){PdLrBExnOk = true;}
      else if(nDmjmtqzdM == RNlgUMfMtY){WmZuhqbJlb = true;}
      if(PsTSyZCJiR == wRFDIYkbkD){jNQtFhyJKr = true;}
      if(XeGgttZiFW == gzAQlEQpuR){gKCDjugadx = true;}
      if(zbVttJmLWM == AXxHtNZaqr){YXnRlyiKsw = true;}
      while(wRFDIYkbkD == PsTSyZCJiR){pjZTxYyNyf = true;}
      while(gzAQlEQpuR == gzAQlEQpuR){LsmajXjcoU = true;}
      while(AXxHtNZaqr == AXxHtNZaqr){mkrXQUXbOb = true;}
      if(dGtMLjbVmA == true){dGtMLjbVmA = false;}
      if(PggnLoCVxi == true){PggnLoCVxi = false;}
      if(cBEMLXTyJu == true){cBEMLXTyJu = false;}
      if(zKjrZaFnFC == true){zKjrZaFnFC = false;}
      if(cuCtBTqJuF == true){cuCtBTqJuF = false;}
      if(KDcIOcChAm == true){KDcIOcChAm = false;}
      if(PdLrBExnOk == true){PdLrBExnOk = false;}
      if(jNQtFhyJKr == true){jNQtFhyJKr = false;}
      if(gKCDjugadx == true){gKCDjugadx = false;}
      if(YXnRlyiKsw == true){YXnRlyiKsw = false;}
      if(WCuKVwmiCg == true){WCuKVwmiCg = false;}
      if(drWHGFbIHF == true){drWHGFbIHF = false;}
      if(XVKRLLZzIH == true){XVKRLLZzIH = false;}
      if(ymPNVePqhm == true){ymPNVePqhm = false;}
      if(IyXxSfAfLg == true){IyXxSfAfLg = false;}
      if(WaCmYAqdyJ == true){WaCmYAqdyJ = false;}
      if(WmZuhqbJlb == true){WmZuhqbJlb = false;}
      if(pjZTxYyNyf == true){pjZTxYyNyf = false;}
      if(LsmajXjcoU == true){LsmajXjcoU = false;}
      if(mkrXQUXbOb == true){mkrXQUXbOb = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOGROIMNEG
{ 
  void rrkUQHKtuT()
  { 
      bool WmCZnLWZtN = false;
      bool prsBjTnrmY = false;
      bool NAZCjZSPxA = false;
      bool bPpduIWUjL = false;
      bool wFkJGkmqHx = false;
      bool NwiJkWMAoN = false;
      bool rGbskaCPGa = false;
      bool wSIbcIOPuy = false;
      bool mbHAmffgrX = false;
      bool uVeVSocrYe = false;
      bool iXkgAtnHgc = false;
      bool HwEpaQzbFE = false;
      bool uVWheZmlIm = false;
      bool eWqPOOpaAs = false;
      bool rXzOPyRotC = false;
      bool oBlUjNACBZ = false;
      bool GBzsykFoaL = false;
      bool ZYbLLXVIRf = false;
      bool IyqFydjVfB = false;
      bool OmZZmqXFQe = false;
      string kwSUSOCgNm;
      string GqcUFCgGbY;
      string mtePXtLXOJ;
      string AulNualaXF;
      string ddPUMAfRUc;
      string czFAypQCxY;
      string CeREmLEOZe;
      string QGhVuJdJCW;
      string NtDkfAolPE;
      string JjczDzLLhB;
      string wRqoMVQjFo;
      string LGzaKIagXl;
      string WSzVUfciSu;
      string xSbBawCNYJ;
      string iFgZrCjCnt;
      string YemwdcRVUf;
      string shQJqsexzO;
      string GMHNCQlYcS;
      string wZtXWSeeWx;
      string dfsrQBnWhz;
      if(kwSUSOCgNm == wRqoMVQjFo){WmCZnLWZtN = true;}
      else if(wRqoMVQjFo == kwSUSOCgNm){iXkgAtnHgc = true;}
      if(GqcUFCgGbY == LGzaKIagXl){prsBjTnrmY = true;}
      else if(LGzaKIagXl == GqcUFCgGbY){HwEpaQzbFE = true;}
      if(mtePXtLXOJ == WSzVUfciSu){NAZCjZSPxA = true;}
      else if(WSzVUfciSu == mtePXtLXOJ){uVWheZmlIm = true;}
      if(AulNualaXF == xSbBawCNYJ){bPpduIWUjL = true;}
      else if(xSbBawCNYJ == AulNualaXF){eWqPOOpaAs = true;}
      if(ddPUMAfRUc == iFgZrCjCnt){wFkJGkmqHx = true;}
      else if(iFgZrCjCnt == ddPUMAfRUc){rXzOPyRotC = true;}
      if(czFAypQCxY == YemwdcRVUf){NwiJkWMAoN = true;}
      else if(YemwdcRVUf == czFAypQCxY){oBlUjNACBZ = true;}
      if(CeREmLEOZe == shQJqsexzO){rGbskaCPGa = true;}
      else if(shQJqsexzO == CeREmLEOZe){GBzsykFoaL = true;}
      if(QGhVuJdJCW == GMHNCQlYcS){wSIbcIOPuy = true;}
      if(NtDkfAolPE == wZtXWSeeWx){mbHAmffgrX = true;}
      if(JjczDzLLhB == dfsrQBnWhz){uVeVSocrYe = true;}
      while(GMHNCQlYcS == QGhVuJdJCW){ZYbLLXVIRf = true;}
      while(wZtXWSeeWx == wZtXWSeeWx){IyqFydjVfB = true;}
      while(dfsrQBnWhz == dfsrQBnWhz){OmZZmqXFQe = true;}
      if(WmCZnLWZtN == true){WmCZnLWZtN = false;}
      if(prsBjTnrmY == true){prsBjTnrmY = false;}
      if(NAZCjZSPxA == true){NAZCjZSPxA = false;}
      if(bPpduIWUjL == true){bPpduIWUjL = false;}
      if(wFkJGkmqHx == true){wFkJGkmqHx = false;}
      if(NwiJkWMAoN == true){NwiJkWMAoN = false;}
      if(rGbskaCPGa == true){rGbskaCPGa = false;}
      if(wSIbcIOPuy == true){wSIbcIOPuy = false;}
      if(mbHAmffgrX == true){mbHAmffgrX = false;}
      if(uVeVSocrYe == true){uVeVSocrYe = false;}
      if(iXkgAtnHgc == true){iXkgAtnHgc = false;}
      if(HwEpaQzbFE == true){HwEpaQzbFE = false;}
      if(uVWheZmlIm == true){uVWheZmlIm = false;}
      if(eWqPOOpaAs == true){eWqPOOpaAs = false;}
      if(rXzOPyRotC == true){rXzOPyRotC = false;}
      if(oBlUjNACBZ == true){oBlUjNACBZ = false;}
      if(GBzsykFoaL == true){GBzsykFoaL = false;}
      if(ZYbLLXVIRf == true){ZYbLLXVIRf = false;}
      if(IyqFydjVfB == true){IyqFydjVfB = false;}
      if(OmZZmqXFQe == true){OmZZmqXFQe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EFBTAUTWKA
{ 
  void DiIHXJhDud()
  { 
      bool UOaxSSJQCp = false;
      bool zauBlCSdyZ = false;
      bool AMpfxCsqXs = false;
      bool aRSGdxmKgi = false;
      bool WHFbqCyQKs = false;
      bool FiPhbIXuzP = false;
      bool YSrJPgIoWn = false;
      bool OIwPDrGOnu = false;
      bool DcfemqCjeU = false;
      bool YrLRJDOwLJ = false;
      bool sJsWdxlAdi = false;
      bool BJGEMwlNVX = false;
      bool eYKBOVxeFz = false;
      bool cKCdBtjJAo = false;
      bool EBGuTPlGFN = false;
      bool RBZmdUyqHH = false;
      bool AHudFpGbDR = false;
      bool McNfSdROhz = false;
      bool jpZqhbkOzf = false;
      bool usCroxGcod = false;
      string SFqcjjEtEO;
      string tMlJiGDbxZ;
      string LMmecEYWdf;
      string ieQxoGqtMw;
      string mynUYZfjRy;
      string nYtTyFRMzf;
      string LhPXePuoTc;
      string PIOAlMXSGD;
      string XoFerKjxzi;
      string neugkFiPoI;
      string eqPnieJsgk;
      string hyGwHVbjfT;
      string exoknjNhPf;
      string dIdxYfYQwM;
      string BqQOBpXyYP;
      string dyjVdgCezy;
      string RbdBGYUJXb;
      string wKnSSmgaiy;
      string MOIACfYISq;
      string dtRxuYMOIb;
      if(SFqcjjEtEO == eqPnieJsgk){UOaxSSJQCp = true;}
      else if(eqPnieJsgk == SFqcjjEtEO){sJsWdxlAdi = true;}
      if(tMlJiGDbxZ == hyGwHVbjfT){zauBlCSdyZ = true;}
      else if(hyGwHVbjfT == tMlJiGDbxZ){BJGEMwlNVX = true;}
      if(LMmecEYWdf == exoknjNhPf){AMpfxCsqXs = true;}
      else if(exoknjNhPf == LMmecEYWdf){eYKBOVxeFz = true;}
      if(ieQxoGqtMw == dIdxYfYQwM){aRSGdxmKgi = true;}
      else if(dIdxYfYQwM == ieQxoGqtMw){cKCdBtjJAo = true;}
      if(mynUYZfjRy == BqQOBpXyYP){WHFbqCyQKs = true;}
      else if(BqQOBpXyYP == mynUYZfjRy){EBGuTPlGFN = true;}
      if(nYtTyFRMzf == dyjVdgCezy){FiPhbIXuzP = true;}
      else if(dyjVdgCezy == nYtTyFRMzf){RBZmdUyqHH = true;}
      if(LhPXePuoTc == RbdBGYUJXb){YSrJPgIoWn = true;}
      else if(RbdBGYUJXb == LhPXePuoTc){AHudFpGbDR = true;}
      if(PIOAlMXSGD == wKnSSmgaiy){OIwPDrGOnu = true;}
      if(XoFerKjxzi == MOIACfYISq){DcfemqCjeU = true;}
      if(neugkFiPoI == dtRxuYMOIb){YrLRJDOwLJ = true;}
      while(wKnSSmgaiy == PIOAlMXSGD){McNfSdROhz = true;}
      while(MOIACfYISq == MOIACfYISq){jpZqhbkOzf = true;}
      while(dtRxuYMOIb == dtRxuYMOIb){usCroxGcod = true;}
      if(UOaxSSJQCp == true){UOaxSSJQCp = false;}
      if(zauBlCSdyZ == true){zauBlCSdyZ = false;}
      if(AMpfxCsqXs == true){AMpfxCsqXs = false;}
      if(aRSGdxmKgi == true){aRSGdxmKgi = false;}
      if(WHFbqCyQKs == true){WHFbqCyQKs = false;}
      if(FiPhbIXuzP == true){FiPhbIXuzP = false;}
      if(YSrJPgIoWn == true){YSrJPgIoWn = false;}
      if(OIwPDrGOnu == true){OIwPDrGOnu = false;}
      if(DcfemqCjeU == true){DcfemqCjeU = false;}
      if(YrLRJDOwLJ == true){YrLRJDOwLJ = false;}
      if(sJsWdxlAdi == true){sJsWdxlAdi = false;}
      if(BJGEMwlNVX == true){BJGEMwlNVX = false;}
      if(eYKBOVxeFz == true){eYKBOVxeFz = false;}
      if(cKCdBtjJAo == true){cKCdBtjJAo = false;}
      if(EBGuTPlGFN == true){EBGuTPlGFN = false;}
      if(RBZmdUyqHH == true){RBZmdUyqHH = false;}
      if(AHudFpGbDR == true){AHudFpGbDR = false;}
      if(McNfSdROhz == true){McNfSdROhz = false;}
      if(jpZqhbkOzf == true){jpZqhbkOzf = false;}
      if(usCroxGcod == true){usCroxGcod = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IKKUEACYFS
{ 
  void HJLpfrxsUE()
  { 
      bool bWuRXcywnE = false;
      bool nTCKwpDRth = false;
      bool RzSIJUYCnW = false;
      bool UdKergJxxa = false;
      bool HsOkCxiLWp = false;
      bool imXeebDmHC = false;
      bool EwIQjJIXSP = false;
      bool fFUSZYkgFL = false;
      bool lNeymhFezP = false;
      bool cWotWKkxBr = false;
      bool oKrHJdSFFF = false;
      bool qWgwPqQJdC = false;
      bool EdXClJPZuO = false;
      bool nnfNWaUXnI = false;
      bool OAZtYGosde = false;
      bool taSznLmoVA = false;
      bool OrJpRCpTYo = false;
      bool WrVUmDDdde = false;
      bool TVuQUNUgYL = false;
      bool neCntWqjbV = false;
      string zkMYreaqQw;
      string NbxHeJohwC;
      string sLVNaeiurr;
      string zQVKHxLSsw;
      string CInJtDDJMj;
      string SRlcxmPzxN;
      string ufyXwwGlAg;
      string xVoAAjXMTX;
      string aYhHZhdsNT;
      string hMUfAVKPMx;
      string wQJfmxblcM;
      string PCfZlVxpyO;
      string RMYfzkAJzd;
      string ZHAVaJPCUc;
      string FrQAEegusd;
      string YTRxejrdfB;
      string wSmymHCJzb;
      string GuVUJZwXaU;
      string EFPUEFpCpp;
      string RkWTglFRNw;
      if(zkMYreaqQw == wQJfmxblcM){bWuRXcywnE = true;}
      else if(wQJfmxblcM == zkMYreaqQw){oKrHJdSFFF = true;}
      if(NbxHeJohwC == PCfZlVxpyO){nTCKwpDRth = true;}
      else if(PCfZlVxpyO == NbxHeJohwC){qWgwPqQJdC = true;}
      if(sLVNaeiurr == RMYfzkAJzd){RzSIJUYCnW = true;}
      else if(RMYfzkAJzd == sLVNaeiurr){EdXClJPZuO = true;}
      if(zQVKHxLSsw == ZHAVaJPCUc){UdKergJxxa = true;}
      else if(ZHAVaJPCUc == zQVKHxLSsw){nnfNWaUXnI = true;}
      if(CInJtDDJMj == FrQAEegusd){HsOkCxiLWp = true;}
      else if(FrQAEegusd == CInJtDDJMj){OAZtYGosde = true;}
      if(SRlcxmPzxN == YTRxejrdfB){imXeebDmHC = true;}
      else if(YTRxejrdfB == SRlcxmPzxN){taSznLmoVA = true;}
      if(ufyXwwGlAg == wSmymHCJzb){EwIQjJIXSP = true;}
      else if(wSmymHCJzb == ufyXwwGlAg){OrJpRCpTYo = true;}
      if(xVoAAjXMTX == GuVUJZwXaU){fFUSZYkgFL = true;}
      if(aYhHZhdsNT == EFPUEFpCpp){lNeymhFezP = true;}
      if(hMUfAVKPMx == RkWTglFRNw){cWotWKkxBr = true;}
      while(GuVUJZwXaU == xVoAAjXMTX){WrVUmDDdde = true;}
      while(EFPUEFpCpp == EFPUEFpCpp){TVuQUNUgYL = true;}
      while(RkWTglFRNw == RkWTglFRNw){neCntWqjbV = true;}
      if(bWuRXcywnE == true){bWuRXcywnE = false;}
      if(nTCKwpDRth == true){nTCKwpDRth = false;}
      if(RzSIJUYCnW == true){RzSIJUYCnW = false;}
      if(UdKergJxxa == true){UdKergJxxa = false;}
      if(HsOkCxiLWp == true){HsOkCxiLWp = false;}
      if(imXeebDmHC == true){imXeebDmHC = false;}
      if(EwIQjJIXSP == true){EwIQjJIXSP = false;}
      if(fFUSZYkgFL == true){fFUSZYkgFL = false;}
      if(lNeymhFezP == true){lNeymhFezP = false;}
      if(cWotWKkxBr == true){cWotWKkxBr = false;}
      if(oKrHJdSFFF == true){oKrHJdSFFF = false;}
      if(qWgwPqQJdC == true){qWgwPqQJdC = false;}
      if(EdXClJPZuO == true){EdXClJPZuO = false;}
      if(nnfNWaUXnI == true){nnfNWaUXnI = false;}
      if(OAZtYGosde == true){OAZtYGosde = false;}
      if(taSznLmoVA == true){taSznLmoVA = false;}
      if(OrJpRCpTYo == true){OrJpRCpTYo = false;}
      if(WrVUmDDdde == true){WrVUmDDdde = false;}
      if(TVuQUNUgYL == true){TVuQUNUgYL = false;}
      if(neCntWqjbV == true){neCntWqjbV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SPJZXNJTEX
{ 
  void tOBrrmDIOS()
  { 
      bool dDVUMXesRU = false;
      bool hdNJegfdDS = false;
      bool bmyHzJFGCq = false;
      bool XWEnRltYkz = false;
      bool GWaTOrHcFf = false;
      bool LhNYKpFOUC = false;
      bool RYqZxLZYqs = false;
      bool lHrELEMdsH = false;
      bool BcAVepIeda = false;
      bool gfDighfBWC = false;
      bool RhkLsEoJMF = false;
      bool dbVNBoXUtH = false;
      bool DtuQMFDJWd = false;
      bool YLGppSqoba = false;
      bool AzSXxEKpgr = false;
      bool KKGPzWmUzn = false;
      bool BfligSUeXh = false;
      bool xirTGDbXSH = false;
      bool uNbdTNEqqS = false;
      bool fWonZUMjqd = false;
      string rtYJJzyaFJ;
      string PupawiikwZ;
      string ARZVOcEJTe;
      string goHrWEbspa;
      string nZJeBqOzEJ;
      string eFlAlZUCug;
      string toGacZnxDq;
      string GotUlaIuUW;
      string NFEJAbDGGx;
      string jYyJfzSiBa;
      string DQuMDoKQNL;
      string RYhXSojxZA;
      string WgqGGgqkom;
      string HYstTwKrGK;
      string RmsueKqZqq;
      string PVIAEWZoFV;
      string xZbaDrfmEV;
      string eqqPiinuaw;
      string pQsxWkQrox;
      string WVlkMrLRqm;
      if(rtYJJzyaFJ == DQuMDoKQNL){dDVUMXesRU = true;}
      else if(DQuMDoKQNL == rtYJJzyaFJ){RhkLsEoJMF = true;}
      if(PupawiikwZ == RYhXSojxZA){hdNJegfdDS = true;}
      else if(RYhXSojxZA == PupawiikwZ){dbVNBoXUtH = true;}
      if(ARZVOcEJTe == WgqGGgqkom){bmyHzJFGCq = true;}
      else if(WgqGGgqkom == ARZVOcEJTe){DtuQMFDJWd = true;}
      if(goHrWEbspa == HYstTwKrGK){XWEnRltYkz = true;}
      else if(HYstTwKrGK == goHrWEbspa){YLGppSqoba = true;}
      if(nZJeBqOzEJ == RmsueKqZqq){GWaTOrHcFf = true;}
      else if(RmsueKqZqq == nZJeBqOzEJ){AzSXxEKpgr = true;}
      if(eFlAlZUCug == PVIAEWZoFV){LhNYKpFOUC = true;}
      else if(PVIAEWZoFV == eFlAlZUCug){KKGPzWmUzn = true;}
      if(toGacZnxDq == xZbaDrfmEV){RYqZxLZYqs = true;}
      else if(xZbaDrfmEV == toGacZnxDq){BfligSUeXh = true;}
      if(GotUlaIuUW == eqqPiinuaw){lHrELEMdsH = true;}
      if(NFEJAbDGGx == pQsxWkQrox){BcAVepIeda = true;}
      if(jYyJfzSiBa == WVlkMrLRqm){gfDighfBWC = true;}
      while(eqqPiinuaw == GotUlaIuUW){xirTGDbXSH = true;}
      while(pQsxWkQrox == pQsxWkQrox){uNbdTNEqqS = true;}
      while(WVlkMrLRqm == WVlkMrLRqm){fWonZUMjqd = true;}
      if(dDVUMXesRU == true){dDVUMXesRU = false;}
      if(hdNJegfdDS == true){hdNJegfdDS = false;}
      if(bmyHzJFGCq == true){bmyHzJFGCq = false;}
      if(XWEnRltYkz == true){XWEnRltYkz = false;}
      if(GWaTOrHcFf == true){GWaTOrHcFf = false;}
      if(LhNYKpFOUC == true){LhNYKpFOUC = false;}
      if(RYqZxLZYqs == true){RYqZxLZYqs = false;}
      if(lHrELEMdsH == true){lHrELEMdsH = false;}
      if(BcAVepIeda == true){BcAVepIeda = false;}
      if(gfDighfBWC == true){gfDighfBWC = false;}
      if(RhkLsEoJMF == true){RhkLsEoJMF = false;}
      if(dbVNBoXUtH == true){dbVNBoXUtH = false;}
      if(DtuQMFDJWd == true){DtuQMFDJWd = false;}
      if(YLGppSqoba == true){YLGppSqoba = false;}
      if(AzSXxEKpgr == true){AzSXxEKpgr = false;}
      if(KKGPzWmUzn == true){KKGPzWmUzn = false;}
      if(BfligSUeXh == true){BfligSUeXh = false;}
      if(xirTGDbXSH == true){xirTGDbXSH = false;}
      if(uNbdTNEqqS == true){uNbdTNEqqS = false;}
      if(fWonZUMjqd == true){fWonZUMjqd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TCEBYCFHEM
{ 
  void kIedepsbgC()
  { 
      bool eGRTBalRZm = false;
      bool CEZGwtuozp = false;
      bool gmXHuTPIBC = false;
      bool liXkNTdFxP = false;
      bool cIJOVKLFWr = false;
      bool FAUQEqhJYg = false;
      bool LdQXBSfnrb = false;
      bool QoriNYdJlj = false;
      bool IJkIwkOHgW = false;
      bool lTgPDzrkMD = false;
      bool eWMubPIkDe = false;
      bool MXXnYCrHaP = false;
      bool DYHVowIceO = false;
      bool gswhFkwpgg = false;
      bool cdDBJNurpB = false;
      bool GjJxzGOqFe = false;
      bool GfFnyTapin = false;
      bool FBZtmZUGMa = false;
      bool AQldHwjziy = false;
      bool VFjBZxXOCo = false;
      string DgPjmdcOmi;
      string tXjSSlYWcV;
      string YKijZzxhdG;
      string PRPqYkFTca;
      string kPFUxbVEbL;
      string qTtVsryeJm;
      string ftjgTcIrth;
      string gFtBkJENmy;
      string crGSOuWtyf;
      string lZKouqeYBU;
      string TdKtdkZsjj;
      string mjrLJmmQRX;
      string FNtOuHuEFt;
      string dfMRezhLoc;
      string VcJNrCgyeH;
      string DOGcPFjwzV;
      string pUmpGGWagK;
      string FbbnCbDfVs;
      string ZwZyqPZjCY;
      string OkMSmApTHk;
      if(DgPjmdcOmi == TdKtdkZsjj){eGRTBalRZm = true;}
      else if(TdKtdkZsjj == DgPjmdcOmi){eWMubPIkDe = true;}
      if(tXjSSlYWcV == mjrLJmmQRX){CEZGwtuozp = true;}
      else if(mjrLJmmQRX == tXjSSlYWcV){MXXnYCrHaP = true;}
      if(YKijZzxhdG == FNtOuHuEFt){gmXHuTPIBC = true;}
      else if(FNtOuHuEFt == YKijZzxhdG){DYHVowIceO = true;}
      if(PRPqYkFTca == dfMRezhLoc){liXkNTdFxP = true;}
      else if(dfMRezhLoc == PRPqYkFTca){gswhFkwpgg = true;}
      if(kPFUxbVEbL == VcJNrCgyeH){cIJOVKLFWr = true;}
      else if(VcJNrCgyeH == kPFUxbVEbL){cdDBJNurpB = true;}
      if(qTtVsryeJm == DOGcPFjwzV){FAUQEqhJYg = true;}
      else if(DOGcPFjwzV == qTtVsryeJm){GjJxzGOqFe = true;}
      if(ftjgTcIrth == pUmpGGWagK){LdQXBSfnrb = true;}
      else if(pUmpGGWagK == ftjgTcIrth){GfFnyTapin = true;}
      if(gFtBkJENmy == FbbnCbDfVs){QoriNYdJlj = true;}
      if(crGSOuWtyf == ZwZyqPZjCY){IJkIwkOHgW = true;}
      if(lZKouqeYBU == OkMSmApTHk){lTgPDzrkMD = true;}
      while(FbbnCbDfVs == gFtBkJENmy){FBZtmZUGMa = true;}
      while(ZwZyqPZjCY == ZwZyqPZjCY){AQldHwjziy = true;}
      while(OkMSmApTHk == OkMSmApTHk){VFjBZxXOCo = true;}
      if(eGRTBalRZm == true){eGRTBalRZm = false;}
      if(CEZGwtuozp == true){CEZGwtuozp = false;}
      if(gmXHuTPIBC == true){gmXHuTPIBC = false;}
      if(liXkNTdFxP == true){liXkNTdFxP = false;}
      if(cIJOVKLFWr == true){cIJOVKLFWr = false;}
      if(FAUQEqhJYg == true){FAUQEqhJYg = false;}
      if(LdQXBSfnrb == true){LdQXBSfnrb = false;}
      if(QoriNYdJlj == true){QoriNYdJlj = false;}
      if(IJkIwkOHgW == true){IJkIwkOHgW = false;}
      if(lTgPDzrkMD == true){lTgPDzrkMD = false;}
      if(eWMubPIkDe == true){eWMubPIkDe = false;}
      if(MXXnYCrHaP == true){MXXnYCrHaP = false;}
      if(DYHVowIceO == true){DYHVowIceO = false;}
      if(gswhFkwpgg == true){gswhFkwpgg = false;}
      if(cdDBJNurpB == true){cdDBJNurpB = false;}
      if(GjJxzGOqFe == true){GjJxzGOqFe = false;}
      if(GfFnyTapin == true){GfFnyTapin = false;}
      if(FBZtmZUGMa == true){FBZtmZUGMa = false;}
      if(AQldHwjziy == true){AQldHwjziy = false;}
      if(VFjBZxXOCo == true){VFjBZxXOCo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MNKJDMYLVC
{ 
  void knEBUzokeZ()
  { 
      bool yFzJFXJMDp = false;
      bool EUwhkhIKeZ = false;
      bool VaZuQTKxXd = false;
      bool ratbzAQZls = false;
      bool sswaxMiPcq = false;
      bool rKRQzWfErl = false;
      bool NLnZxdaTij = false;
      bool QhXkfbeyxD = false;
      bool jtTglgIdcb = false;
      bool ULgAfhEWcT = false;
      bool lPgsZJlgkW = false;
      bool rhGoBSrHmP = false;
      bool UxpGPEBXEy = false;
      bool XIUcRUMZHb = false;
      bool tarFjlEryM = false;
      bool pxiNyoYadU = false;
      bool IObmzjNdWg = false;
      bool GRALVtEkWc = false;
      bool AIYqHZYHdb = false;
      bool ululTfyFFA = false;
      string DGfVdUPcgO;
      string xEGEywKhkD;
      string QVDHVmjuKe;
      string KpzfVnhYpM;
      string mFdLwylWNJ;
      string xeVLLhCXzq;
      string kTSBEGEZhJ;
      string NlrXQKqqLX;
      string BMgBhfUnUm;
      string lJYybSlcHo;
      string sLnDuLlxtA;
      string pLMboXlRTY;
      string kQFtXDMsuy;
      string XOiVdoYdFW;
      string sXsTwZxxJE;
      string fUIiEbGsfq;
      string yrBwcbbqxz;
      string yILweNXkxx;
      string GPjqRCFARC;
      string heyOYAVxpU;
      if(DGfVdUPcgO == sLnDuLlxtA){yFzJFXJMDp = true;}
      else if(sLnDuLlxtA == DGfVdUPcgO){lPgsZJlgkW = true;}
      if(xEGEywKhkD == pLMboXlRTY){EUwhkhIKeZ = true;}
      else if(pLMboXlRTY == xEGEywKhkD){rhGoBSrHmP = true;}
      if(QVDHVmjuKe == kQFtXDMsuy){VaZuQTKxXd = true;}
      else if(kQFtXDMsuy == QVDHVmjuKe){UxpGPEBXEy = true;}
      if(KpzfVnhYpM == XOiVdoYdFW){ratbzAQZls = true;}
      else if(XOiVdoYdFW == KpzfVnhYpM){XIUcRUMZHb = true;}
      if(mFdLwylWNJ == sXsTwZxxJE){sswaxMiPcq = true;}
      else if(sXsTwZxxJE == mFdLwylWNJ){tarFjlEryM = true;}
      if(xeVLLhCXzq == fUIiEbGsfq){rKRQzWfErl = true;}
      else if(fUIiEbGsfq == xeVLLhCXzq){pxiNyoYadU = true;}
      if(kTSBEGEZhJ == yrBwcbbqxz){NLnZxdaTij = true;}
      else if(yrBwcbbqxz == kTSBEGEZhJ){IObmzjNdWg = true;}
      if(NlrXQKqqLX == yILweNXkxx){QhXkfbeyxD = true;}
      if(BMgBhfUnUm == GPjqRCFARC){jtTglgIdcb = true;}
      if(lJYybSlcHo == heyOYAVxpU){ULgAfhEWcT = true;}
      while(yILweNXkxx == NlrXQKqqLX){GRALVtEkWc = true;}
      while(GPjqRCFARC == GPjqRCFARC){AIYqHZYHdb = true;}
      while(heyOYAVxpU == heyOYAVxpU){ululTfyFFA = true;}
      if(yFzJFXJMDp == true){yFzJFXJMDp = false;}
      if(EUwhkhIKeZ == true){EUwhkhIKeZ = false;}
      if(VaZuQTKxXd == true){VaZuQTKxXd = false;}
      if(ratbzAQZls == true){ratbzAQZls = false;}
      if(sswaxMiPcq == true){sswaxMiPcq = false;}
      if(rKRQzWfErl == true){rKRQzWfErl = false;}
      if(NLnZxdaTij == true){NLnZxdaTij = false;}
      if(QhXkfbeyxD == true){QhXkfbeyxD = false;}
      if(jtTglgIdcb == true){jtTglgIdcb = false;}
      if(ULgAfhEWcT == true){ULgAfhEWcT = false;}
      if(lPgsZJlgkW == true){lPgsZJlgkW = false;}
      if(rhGoBSrHmP == true){rhGoBSrHmP = false;}
      if(UxpGPEBXEy == true){UxpGPEBXEy = false;}
      if(XIUcRUMZHb == true){XIUcRUMZHb = false;}
      if(tarFjlEryM == true){tarFjlEryM = false;}
      if(pxiNyoYadU == true){pxiNyoYadU = false;}
      if(IObmzjNdWg == true){IObmzjNdWg = false;}
      if(GRALVtEkWc == true){GRALVtEkWc = false;}
      if(AIYqHZYHdb == true){AIYqHZYHdb = false;}
      if(ululTfyFFA == true){ululTfyFFA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OWPMPMQPEA
{ 
  void oNDAkPwKel()
  { 
      bool hLUNqldVRQ = false;
      bool dWRXTUQjNt = false;
      bool sepKOQSTPZ = false;
      bool WqDFXygmkX = false;
      bool ZdGGHRlYMp = false;
      bool nIoAqDUJzZ = false;
      bool SKFqsmTMAU = false;
      bool ndzpUOYiDw = false;
      bool etHYlOBCNU = false;
      bool dWmKFOXONU = false;
      bool yYXqCKJzeb = false;
      bool ekGCuXDgFe = false;
      bool kQyeqQpnjG = false;
      bool pJVhxFVQWa = false;
      bool FBXlprQkPg = false;
      bool LqJhjhZszk = false;
      bool UUOYkUPXEp = false;
      bool CmKdZisMoy = false;
      bool AFYDtzAmyP = false;
      bool ecQkGjmyAU = false;
      string jQGjCZCIkE;
      string QcEzwsUaug;
      string BqaibLEPUD;
      string unxiLxPeYO;
      string dcnIdZjPTs;
      string emhlGroXIp;
      string VlmhJbfUmz;
      string coBgCYCSFC;
      string anwbXHGKbT;
      string HrWhsqpGyB;
      string sPbUPokAih;
      string oWRFPgpLJB;
      string VVsTqkSTnL;
      string iwTDJLMaLh;
      string WFatdkZQSL;
      string BdwwfXVIde;
      string WRMXzsCOwE;
      string UPAaePtypG;
      string KBBbJEzXXk;
      string oEwfLEsDuj;
      if(jQGjCZCIkE == sPbUPokAih){hLUNqldVRQ = true;}
      else if(sPbUPokAih == jQGjCZCIkE){yYXqCKJzeb = true;}
      if(QcEzwsUaug == oWRFPgpLJB){dWRXTUQjNt = true;}
      else if(oWRFPgpLJB == QcEzwsUaug){ekGCuXDgFe = true;}
      if(BqaibLEPUD == VVsTqkSTnL){sepKOQSTPZ = true;}
      else if(VVsTqkSTnL == BqaibLEPUD){kQyeqQpnjG = true;}
      if(unxiLxPeYO == iwTDJLMaLh){WqDFXygmkX = true;}
      else if(iwTDJLMaLh == unxiLxPeYO){pJVhxFVQWa = true;}
      if(dcnIdZjPTs == WFatdkZQSL){ZdGGHRlYMp = true;}
      else if(WFatdkZQSL == dcnIdZjPTs){FBXlprQkPg = true;}
      if(emhlGroXIp == BdwwfXVIde){nIoAqDUJzZ = true;}
      else if(BdwwfXVIde == emhlGroXIp){LqJhjhZszk = true;}
      if(VlmhJbfUmz == WRMXzsCOwE){SKFqsmTMAU = true;}
      else if(WRMXzsCOwE == VlmhJbfUmz){UUOYkUPXEp = true;}
      if(coBgCYCSFC == UPAaePtypG){ndzpUOYiDw = true;}
      if(anwbXHGKbT == KBBbJEzXXk){etHYlOBCNU = true;}
      if(HrWhsqpGyB == oEwfLEsDuj){dWmKFOXONU = true;}
      while(UPAaePtypG == coBgCYCSFC){CmKdZisMoy = true;}
      while(KBBbJEzXXk == KBBbJEzXXk){AFYDtzAmyP = true;}
      while(oEwfLEsDuj == oEwfLEsDuj){ecQkGjmyAU = true;}
      if(hLUNqldVRQ == true){hLUNqldVRQ = false;}
      if(dWRXTUQjNt == true){dWRXTUQjNt = false;}
      if(sepKOQSTPZ == true){sepKOQSTPZ = false;}
      if(WqDFXygmkX == true){WqDFXygmkX = false;}
      if(ZdGGHRlYMp == true){ZdGGHRlYMp = false;}
      if(nIoAqDUJzZ == true){nIoAqDUJzZ = false;}
      if(SKFqsmTMAU == true){SKFqsmTMAU = false;}
      if(ndzpUOYiDw == true){ndzpUOYiDw = false;}
      if(etHYlOBCNU == true){etHYlOBCNU = false;}
      if(dWmKFOXONU == true){dWmKFOXONU = false;}
      if(yYXqCKJzeb == true){yYXqCKJzeb = false;}
      if(ekGCuXDgFe == true){ekGCuXDgFe = false;}
      if(kQyeqQpnjG == true){kQyeqQpnjG = false;}
      if(pJVhxFVQWa == true){pJVhxFVQWa = false;}
      if(FBXlprQkPg == true){FBXlprQkPg = false;}
      if(LqJhjhZszk == true){LqJhjhZszk = false;}
      if(UUOYkUPXEp == true){UUOYkUPXEp = false;}
      if(CmKdZisMoy == true){CmKdZisMoy = false;}
      if(AFYDtzAmyP == true){AFYDtzAmyP = false;}
      if(ecQkGjmyAU == true){ecQkGjmyAU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SSSQQUWMZS
{ 
  void pPGyueTxli()
  { 
      bool rWniDHmqUV = false;
      bool kTPGCWBhtK = false;
      bool SWONxgUkaz = false;
      bool YNhYaimpLs = false;
      bool OWnDwTnWYX = false;
      bool DnWcRVHakV = false;
      bool mBPUwWlpfT = false;
      bool jKVBwwhLRc = false;
      bool sxMCwjqEHE = false;
      bool aLFpGMGSGY = false;
      bool hxqFnrSUgV = false;
      bool mwJHcooNYM = false;
      bool ehiarXFfti = false;
      bool oOEruwFGND = false;
      bool RVXkRdCxMQ = false;
      bool XrttsANQMQ = false;
      bool mlaxfXrQLg = false;
      bool jhzVuJJyJO = false;
      bool UISdLxUKyu = false;
      bool hotxQeuZEs = false;
      string rFqUsnoEIk;
      string SgLggBMULo;
      string HltoVmoTut;
      string zgabeAraqD;
      string cHmySOwGyj;
      string tyKNNFLBzG;
      string zVJNllGjCg;
      string IldILzzuWG;
      string piULAAVNNY;
      string npbjEGyYLg;
      string IMERaHUBBN;
      string GPerPPBxNl;
      string PrVcrRKsEF;
      string BZipxdeDjr;
      string fUMARYLiUn;
      string FALZQQpaZI;
      string xaxuLgfntl;
      string kyNUFueVKS;
      string GaYPABNJYg;
      string nDdbtFWRnk;
      if(rFqUsnoEIk == IMERaHUBBN){rWniDHmqUV = true;}
      else if(IMERaHUBBN == rFqUsnoEIk){hxqFnrSUgV = true;}
      if(SgLggBMULo == GPerPPBxNl){kTPGCWBhtK = true;}
      else if(GPerPPBxNl == SgLggBMULo){mwJHcooNYM = true;}
      if(HltoVmoTut == PrVcrRKsEF){SWONxgUkaz = true;}
      else if(PrVcrRKsEF == HltoVmoTut){ehiarXFfti = true;}
      if(zgabeAraqD == BZipxdeDjr){YNhYaimpLs = true;}
      else if(BZipxdeDjr == zgabeAraqD){oOEruwFGND = true;}
      if(cHmySOwGyj == fUMARYLiUn){OWnDwTnWYX = true;}
      else if(fUMARYLiUn == cHmySOwGyj){RVXkRdCxMQ = true;}
      if(tyKNNFLBzG == FALZQQpaZI){DnWcRVHakV = true;}
      else if(FALZQQpaZI == tyKNNFLBzG){XrttsANQMQ = true;}
      if(zVJNllGjCg == xaxuLgfntl){mBPUwWlpfT = true;}
      else if(xaxuLgfntl == zVJNllGjCg){mlaxfXrQLg = true;}
      if(IldILzzuWG == kyNUFueVKS){jKVBwwhLRc = true;}
      if(piULAAVNNY == GaYPABNJYg){sxMCwjqEHE = true;}
      if(npbjEGyYLg == nDdbtFWRnk){aLFpGMGSGY = true;}
      while(kyNUFueVKS == IldILzzuWG){jhzVuJJyJO = true;}
      while(GaYPABNJYg == GaYPABNJYg){UISdLxUKyu = true;}
      while(nDdbtFWRnk == nDdbtFWRnk){hotxQeuZEs = true;}
      if(rWniDHmqUV == true){rWniDHmqUV = false;}
      if(kTPGCWBhtK == true){kTPGCWBhtK = false;}
      if(SWONxgUkaz == true){SWONxgUkaz = false;}
      if(YNhYaimpLs == true){YNhYaimpLs = false;}
      if(OWnDwTnWYX == true){OWnDwTnWYX = false;}
      if(DnWcRVHakV == true){DnWcRVHakV = false;}
      if(mBPUwWlpfT == true){mBPUwWlpfT = false;}
      if(jKVBwwhLRc == true){jKVBwwhLRc = false;}
      if(sxMCwjqEHE == true){sxMCwjqEHE = false;}
      if(aLFpGMGSGY == true){aLFpGMGSGY = false;}
      if(hxqFnrSUgV == true){hxqFnrSUgV = false;}
      if(mwJHcooNYM == true){mwJHcooNYM = false;}
      if(ehiarXFfti == true){ehiarXFfti = false;}
      if(oOEruwFGND == true){oOEruwFGND = false;}
      if(RVXkRdCxMQ == true){RVXkRdCxMQ = false;}
      if(XrttsANQMQ == true){XrttsANQMQ = false;}
      if(mlaxfXrQLg == true){mlaxfXrQLg = false;}
      if(jhzVuJJyJO == true){jhzVuJJyJO = false;}
      if(UISdLxUKyu == true){UISdLxUKyu = false;}
      if(hotxQeuZEs == true){hotxQeuZEs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BCTVPOBHIF
{ 
  void BIcdYkbdpE()
  { 
      bool BHrRaxjTQY = false;
      bool bdelMPBaYU = false;
      bool nLChyJcwGl = false;
      bool VXyPGmpsat = false;
      bool neVUbfUSbI = false;
      bool icAEqoFJSR = false;
      bool TnMeCEePeY = false;
      bool YHAefIphFR = false;
      bool oKTyDsWtXR = false;
      bool gfZJZzSpRX = false;
      bool BEmCiacVXh = false;
      bool zWEBbJTObI = false;
      bool gkrNqsgqEg = false;
      bool pzPhQkWROb = false;
      bool QBVoRKbYgQ = false;
      bool fDOIpPLYKp = false;
      bool tkQqrzGWkq = false;
      bool lCAzrfODwg = false;
      bool QIJmAmjGqA = false;
      bool YyGXhejexR = false;
      string EGEuHgcgfj;
      string iaiFOYTrml;
      string dYAPgmcpzz;
      string WbFmkfgKNa;
      string AidTPlkKlx;
      string TbcZhfFoNU;
      string DlhfNHVuUi;
      string qJdLkuugAu;
      string BRDBftzfFE;
      string ZrwsXNByGH;
      string QppDFKxQnm;
      string kYnRSlhfAS;
      string IooPIyMknW;
      string sxZreKaAuV;
      string whqRTADdZl;
      string fUymzaAauH;
      string xMcQsnyulI;
      string KhjdVayWuI;
      string uhLCqYzmsE;
      string JAFnXjEZYf;
      if(EGEuHgcgfj == QppDFKxQnm){BHrRaxjTQY = true;}
      else if(QppDFKxQnm == EGEuHgcgfj){BEmCiacVXh = true;}
      if(iaiFOYTrml == kYnRSlhfAS){bdelMPBaYU = true;}
      else if(kYnRSlhfAS == iaiFOYTrml){zWEBbJTObI = true;}
      if(dYAPgmcpzz == IooPIyMknW){nLChyJcwGl = true;}
      else if(IooPIyMknW == dYAPgmcpzz){gkrNqsgqEg = true;}
      if(WbFmkfgKNa == sxZreKaAuV){VXyPGmpsat = true;}
      else if(sxZreKaAuV == WbFmkfgKNa){pzPhQkWROb = true;}
      if(AidTPlkKlx == whqRTADdZl){neVUbfUSbI = true;}
      else if(whqRTADdZl == AidTPlkKlx){QBVoRKbYgQ = true;}
      if(TbcZhfFoNU == fUymzaAauH){icAEqoFJSR = true;}
      else if(fUymzaAauH == TbcZhfFoNU){fDOIpPLYKp = true;}
      if(DlhfNHVuUi == xMcQsnyulI){TnMeCEePeY = true;}
      else if(xMcQsnyulI == DlhfNHVuUi){tkQqrzGWkq = true;}
      if(qJdLkuugAu == KhjdVayWuI){YHAefIphFR = true;}
      if(BRDBftzfFE == uhLCqYzmsE){oKTyDsWtXR = true;}
      if(ZrwsXNByGH == JAFnXjEZYf){gfZJZzSpRX = true;}
      while(KhjdVayWuI == qJdLkuugAu){lCAzrfODwg = true;}
      while(uhLCqYzmsE == uhLCqYzmsE){QIJmAmjGqA = true;}
      while(JAFnXjEZYf == JAFnXjEZYf){YyGXhejexR = true;}
      if(BHrRaxjTQY == true){BHrRaxjTQY = false;}
      if(bdelMPBaYU == true){bdelMPBaYU = false;}
      if(nLChyJcwGl == true){nLChyJcwGl = false;}
      if(VXyPGmpsat == true){VXyPGmpsat = false;}
      if(neVUbfUSbI == true){neVUbfUSbI = false;}
      if(icAEqoFJSR == true){icAEqoFJSR = false;}
      if(TnMeCEePeY == true){TnMeCEePeY = false;}
      if(YHAefIphFR == true){YHAefIphFR = false;}
      if(oKTyDsWtXR == true){oKTyDsWtXR = false;}
      if(gfZJZzSpRX == true){gfZJZzSpRX = false;}
      if(BEmCiacVXh == true){BEmCiacVXh = false;}
      if(zWEBbJTObI == true){zWEBbJTObI = false;}
      if(gkrNqsgqEg == true){gkrNqsgqEg = false;}
      if(pzPhQkWROb == true){pzPhQkWROb = false;}
      if(QBVoRKbYgQ == true){QBVoRKbYgQ = false;}
      if(fDOIpPLYKp == true){fDOIpPLYKp = false;}
      if(tkQqrzGWkq == true){tkQqrzGWkq = false;}
      if(lCAzrfODwg == true){lCAzrfODwg = false;}
      if(QIJmAmjGqA == true){QIJmAmjGqA = false;}
      if(YyGXhejexR == true){YyGXhejexR = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NMLOXFXXRG
{ 
  void OkqsSIGfPE()
  { 
      bool QPJEGxksJL = false;
      bool pbaFQolVos = false;
      bool eusDFJKfFY = false;
      bool sTrzeXkNRW = false;
      bool HkZuGXmrry = false;
      bool FkBedVVFzx = false;
      bool mEnjSVjaas = false;
      bool EYESktUyxg = false;
      bool VRnHSISNUJ = false;
      bool BeNUjDwpbj = false;
      bool nLRztggisj = false;
      bool AXIRaUQgQK = false;
      bool DWFRqYKjih = false;
      bool xdVMsKAoMl = false;
      bool ajTkIcVusO = false;
      bool lPYRgRFAHU = false;
      bool WxAKlVBwYQ = false;
      bool UaTeiSCUIN = false;
      bool COpTuhFEeb = false;
      bool EzhrOdjIyl = false;
      string DKATkUKsse;
      string uFNMzbVEcC;
      string BGXxOZpLTN;
      string quKVqXmLTb;
      string NDjaPGMgTx;
      string APwZExmCFL;
      string dzZemAurVb;
      string oKjAoxVxhI;
      string MJqWNaykQs;
      string kjGIyNdEmO;
      string LTLrIoewjG;
      string aoTxyUHDRM;
      string CKabqEEbBa;
      string TRKTxsxzNY;
      string uBAKSuQJJd;
      string GamaMArjBE;
      string jDtJlJeAzF;
      string WIosQAKQIu;
      string muRXUsdjGw;
      string ANyInqHpXJ;
      if(DKATkUKsse == LTLrIoewjG){QPJEGxksJL = true;}
      else if(LTLrIoewjG == DKATkUKsse){nLRztggisj = true;}
      if(uFNMzbVEcC == aoTxyUHDRM){pbaFQolVos = true;}
      else if(aoTxyUHDRM == uFNMzbVEcC){AXIRaUQgQK = true;}
      if(BGXxOZpLTN == CKabqEEbBa){eusDFJKfFY = true;}
      else if(CKabqEEbBa == BGXxOZpLTN){DWFRqYKjih = true;}
      if(quKVqXmLTb == TRKTxsxzNY){sTrzeXkNRW = true;}
      else if(TRKTxsxzNY == quKVqXmLTb){xdVMsKAoMl = true;}
      if(NDjaPGMgTx == uBAKSuQJJd){HkZuGXmrry = true;}
      else if(uBAKSuQJJd == NDjaPGMgTx){ajTkIcVusO = true;}
      if(APwZExmCFL == GamaMArjBE){FkBedVVFzx = true;}
      else if(GamaMArjBE == APwZExmCFL){lPYRgRFAHU = true;}
      if(dzZemAurVb == jDtJlJeAzF){mEnjSVjaas = true;}
      else if(jDtJlJeAzF == dzZemAurVb){WxAKlVBwYQ = true;}
      if(oKjAoxVxhI == WIosQAKQIu){EYESktUyxg = true;}
      if(MJqWNaykQs == muRXUsdjGw){VRnHSISNUJ = true;}
      if(kjGIyNdEmO == ANyInqHpXJ){BeNUjDwpbj = true;}
      while(WIosQAKQIu == oKjAoxVxhI){UaTeiSCUIN = true;}
      while(muRXUsdjGw == muRXUsdjGw){COpTuhFEeb = true;}
      while(ANyInqHpXJ == ANyInqHpXJ){EzhrOdjIyl = true;}
      if(QPJEGxksJL == true){QPJEGxksJL = false;}
      if(pbaFQolVos == true){pbaFQolVos = false;}
      if(eusDFJKfFY == true){eusDFJKfFY = false;}
      if(sTrzeXkNRW == true){sTrzeXkNRW = false;}
      if(HkZuGXmrry == true){HkZuGXmrry = false;}
      if(FkBedVVFzx == true){FkBedVVFzx = false;}
      if(mEnjSVjaas == true){mEnjSVjaas = false;}
      if(EYESktUyxg == true){EYESktUyxg = false;}
      if(VRnHSISNUJ == true){VRnHSISNUJ = false;}
      if(BeNUjDwpbj == true){BeNUjDwpbj = false;}
      if(nLRztggisj == true){nLRztggisj = false;}
      if(AXIRaUQgQK == true){AXIRaUQgQK = false;}
      if(DWFRqYKjih == true){DWFRqYKjih = false;}
      if(xdVMsKAoMl == true){xdVMsKAoMl = false;}
      if(ajTkIcVusO == true){ajTkIcVusO = false;}
      if(lPYRgRFAHU == true){lPYRgRFAHU = false;}
      if(WxAKlVBwYQ == true){WxAKlVBwYQ = false;}
      if(UaTeiSCUIN == true){UaTeiSCUIN = false;}
      if(COpTuhFEeb == true){COpTuhFEeb = false;}
      if(EzhrOdjIyl == true){EzhrOdjIyl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JHNNVFJRJO
{ 
  void jURoLomILt()
  { 
      bool EnETpZPfAQ = false;
      bool tJTFpKHRak = false;
      bool mFcyWyVBoX = false;
      bool ODryzdkduw = false;
      bool RwpdjxBQhl = false;
      bool FqTDozKFxr = false;
      bool DOpbCpQUFM = false;
      bool YcWeTpPyOY = false;
      bool wFSCRWjoJA = false;
      bool SefCrDFNbt = false;
      bool gfKUNbUniy = false;
      bool jLqYuxunJf = false;
      bool FaLEstxTmP = false;
      bool kBWTqQuxop = false;
      bool BuFKQGmHGt = false;
      bool IRDwnFUFbS = false;
      bool eABTNhoWge = false;
      bool CxyTdKUAnl = false;
      bool GoNrHSyafl = false;
      bool uoVVLBtlxi = false;
      string DEsWKQtXBJ;
      string tPzTefyFqk;
      string lzUcNCFdAy;
      string oytJCFiriD;
      string GTEGUFWdlh;
      string dWlQJDjPJI;
      string KPXDTuUoQY;
      string wGVneSuNII;
      string fbrRTRLdzz;
      string VSiarpuBmY;
      string jgMMkWAykq;
      string IVHAciXFiC;
      string jqpRDcXuun;
      string WsaiXyPefw;
      string RMrIqzLhyJ;
      string SoRjnsqtJH;
      string zEDxTOuXcd;
      string SrZljXzgaD;
      string RoxkVhbpud;
      string HUVXxMJuob;
      if(DEsWKQtXBJ == jgMMkWAykq){EnETpZPfAQ = true;}
      else if(jgMMkWAykq == DEsWKQtXBJ){gfKUNbUniy = true;}
      if(tPzTefyFqk == IVHAciXFiC){tJTFpKHRak = true;}
      else if(IVHAciXFiC == tPzTefyFqk){jLqYuxunJf = true;}
      if(lzUcNCFdAy == jqpRDcXuun){mFcyWyVBoX = true;}
      else if(jqpRDcXuun == lzUcNCFdAy){FaLEstxTmP = true;}
      if(oytJCFiriD == WsaiXyPefw){ODryzdkduw = true;}
      else if(WsaiXyPefw == oytJCFiriD){kBWTqQuxop = true;}
      if(GTEGUFWdlh == RMrIqzLhyJ){RwpdjxBQhl = true;}
      else if(RMrIqzLhyJ == GTEGUFWdlh){BuFKQGmHGt = true;}
      if(dWlQJDjPJI == SoRjnsqtJH){FqTDozKFxr = true;}
      else if(SoRjnsqtJH == dWlQJDjPJI){IRDwnFUFbS = true;}
      if(KPXDTuUoQY == zEDxTOuXcd){DOpbCpQUFM = true;}
      else if(zEDxTOuXcd == KPXDTuUoQY){eABTNhoWge = true;}
      if(wGVneSuNII == SrZljXzgaD){YcWeTpPyOY = true;}
      if(fbrRTRLdzz == RoxkVhbpud){wFSCRWjoJA = true;}
      if(VSiarpuBmY == HUVXxMJuob){SefCrDFNbt = true;}
      while(SrZljXzgaD == wGVneSuNII){CxyTdKUAnl = true;}
      while(RoxkVhbpud == RoxkVhbpud){GoNrHSyafl = true;}
      while(HUVXxMJuob == HUVXxMJuob){uoVVLBtlxi = true;}
      if(EnETpZPfAQ == true){EnETpZPfAQ = false;}
      if(tJTFpKHRak == true){tJTFpKHRak = false;}
      if(mFcyWyVBoX == true){mFcyWyVBoX = false;}
      if(ODryzdkduw == true){ODryzdkduw = false;}
      if(RwpdjxBQhl == true){RwpdjxBQhl = false;}
      if(FqTDozKFxr == true){FqTDozKFxr = false;}
      if(DOpbCpQUFM == true){DOpbCpQUFM = false;}
      if(YcWeTpPyOY == true){YcWeTpPyOY = false;}
      if(wFSCRWjoJA == true){wFSCRWjoJA = false;}
      if(SefCrDFNbt == true){SefCrDFNbt = false;}
      if(gfKUNbUniy == true){gfKUNbUniy = false;}
      if(jLqYuxunJf == true){jLqYuxunJf = false;}
      if(FaLEstxTmP == true){FaLEstxTmP = false;}
      if(kBWTqQuxop == true){kBWTqQuxop = false;}
      if(BuFKQGmHGt == true){BuFKQGmHGt = false;}
      if(IRDwnFUFbS == true){IRDwnFUFbS = false;}
      if(eABTNhoWge == true){eABTNhoWge = false;}
      if(CxyTdKUAnl == true){CxyTdKUAnl = false;}
      if(GoNrHSyafl == true){GoNrHSyafl = false;}
      if(uoVVLBtlxi == true){uoVVLBtlxi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TKNYCKTRFY
{ 
  void gHAPqMLwlt()
  { 
      bool YYOTBJnfUu = false;
      bool fdwzicgChj = false;
      bool RcPxefPjfT = false;
      bool WgGkfFAFdU = false;
      bool WHupfFGLrW = false;
      bool LzzjwuIQoU = false;
      bool tEAAYyXKSc = false;
      bool cbXNNJhZRY = false;
      bool ZDqIIXgWok = false;
      bool wiPIaSGtRh = false;
      bool hurOsialVl = false;
      bool MizPtpMqrB = false;
      bool QAVhfsLOmu = false;
      bool DnHiqdLcOY = false;
      bool wuACXNjANj = false;
      bool FOBQRgZKhz = false;
      bool XMzaiYaPYQ = false;
      bool ejcTEUASKS = false;
      bool IKAConlqRJ = false;
      bool EAuOAjwYTy = false;
      string DmsYaDkYYz;
      string ATtATVsUpA;
      string NPHzwnLBRd;
      string YLmiLmdRYI;
      string KEeSQjFBIK;
      string GKeRNnesbl;
      string qiGuZnfKnf;
      string qbugpIjoiL;
      string iKrHWOgVUW;
      string IkolRayjrb;
      string EtMcSeOxbF;
      string EpJQDgpzXL;
      string HJpRXFmIPO;
      string JzwGGnxPnr;
      string zitPSRbXij;
      string jzLgUtEuEN;
      string rGhMuEsCJO;
      string iaKfMYOazF;
      string dOYedKmPlL;
      string jCKaArBhTN;
      if(DmsYaDkYYz == EtMcSeOxbF){YYOTBJnfUu = true;}
      else if(EtMcSeOxbF == DmsYaDkYYz){hurOsialVl = true;}
      if(ATtATVsUpA == EpJQDgpzXL){fdwzicgChj = true;}
      else if(EpJQDgpzXL == ATtATVsUpA){MizPtpMqrB = true;}
      if(NPHzwnLBRd == HJpRXFmIPO){RcPxefPjfT = true;}
      else if(HJpRXFmIPO == NPHzwnLBRd){QAVhfsLOmu = true;}
      if(YLmiLmdRYI == JzwGGnxPnr){WgGkfFAFdU = true;}
      else if(JzwGGnxPnr == YLmiLmdRYI){DnHiqdLcOY = true;}
      if(KEeSQjFBIK == zitPSRbXij){WHupfFGLrW = true;}
      else if(zitPSRbXij == KEeSQjFBIK){wuACXNjANj = true;}
      if(GKeRNnesbl == jzLgUtEuEN){LzzjwuIQoU = true;}
      else if(jzLgUtEuEN == GKeRNnesbl){FOBQRgZKhz = true;}
      if(qiGuZnfKnf == rGhMuEsCJO){tEAAYyXKSc = true;}
      else if(rGhMuEsCJO == qiGuZnfKnf){XMzaiYaPYQ = true;}
      if(qbugpIjoiL == iaKfMYOazF){cbXNNJhZRY = true;}
      if(iKrHWOgVUW == dOYedKmPlL){ZDqIIXgWok = true;}
      if(IkolRayjrb == jCKaArBhTN){wiPIaSGtRh = true;}
      while(iaKfMYOazF == qbugpIjoiL){ejcTEUASKS = true;}
      while(dOYedKmPlL == dOYedKmPlL){IKAConlqRJ = true;}
      while(jCKaArBhTN == jCKaArBhTN){EAuOAjwYTy = true;}
      if(YYOTBJnfUu == true){YYOTBJnfUu = false;}
      if(fdwzicgChj == true){fdwzicgChj = false;}
      if(RcPxefPjfT == true){RcPxefPjfT = false;}
      if(WgGkfFAFdU == true){WgGkfFAFdU = false;}
      if(WHupfFGLrW == true){WHupfFGLrW = false;}
      if(LzzjwuIQoU == true){LzzjwuIQoU = false;}
      if(tEAAYyXKSc == true){tEAAYyXKSc = false;}
      if(cbXNNJhZRY == true){cbXNNJhZRY = false;}
      if(ZDqIIXgWok == true){ZDqIIXgWok = false;}
      if(wiPIaSGtRh == true){wiPIaSGtRh = false;}
      if(hurOsialVl == true){hurOsialVl = false;}
      if(MizPtpMqrB == true){MizPtpMqrB = false;}
      if(QAVhfsLOmu == true){QAVhfsLOmu = false;}
      if(DnHiqdLcOY == true){DnHiqdLcOY = false;}
      if(wuACXNjANj == true){wuACXNjANj = false;}
      if(FOBQRgZKhz == true){FOBQRgZKhz = false;}
      if(XMzaiYaPYQ == true){XMzaiYaPYQ = false;}
      if(ejcTEUASKS == true){ejcTEUASKS = false;}
      if(IKAConlqRJ == true){IKAConlqRJ = false;}
      if(EAuOAjwYTy == true){EAuOAjwYTy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NFFOMDMWVA
{ 
  void jDJPkwyyIO()
  { 
      bool DHkUNNIMUw = false;
      bool jSpAJFbWSY = false;
      bool YTbgXrBJzV = false;
      bool ngCPMndfKD = false;
      bool rySoZQGXxp = false;
      bool VBtGlieEwU = false;
      bool LzMzHCPanV = false;
      bool buMOfZBlGH = false;
      bool ldMIzmmwdd = false;
      bool IYiorskTgX = false;
      bool BfiaOZneZL = false;
      bool gsyoiglDzU = false;
      bool PTxKNCLXVm = false;
      bool ogEhFFultY = false;
      bool yHVRwPatFK = false;
      bool HgTMIgiXNM = false;
      bool OQImWznPic = false;
      bool kAJZVHxeOy = false;
      bool aRRKeHNqZX = false;
      bool WWXhKhyhuw = false;
      string yHwssPmIjU;
      string ZPmMFYuzBH;
      string mFQAGUJrzr;
      string sBlnrkbuwP;
      string yKOIOxqHLS;
      string MKJzesqwmX;
      string GuIrbXczTU;
      string wzClJcpHlI;
      string dBfgehqmJy;
      string tfHXIDBVgp;
      string TrmqVMHXpC;
      string DiJetEObNs;
      string fppVaQrecF;
      string aTaunlOoLR;
      string aTwGuKVccE;
      string nYImJdBRQE;
      string UcujajHOqI;
      string pZQwSMsiaP;
      string uPBXRBKmlK;
      string jxXGwxXMfK;
      if(yHwssPmIjU == TrmqVMHXpC){DHkUNNIMUw = true;}
      else if(TrmqVMHXpC == yHwssPmIjU){BfiaOZneZL = true;}
      if(ZPmMFYuzBH == DiJetEObNs){jSpAJFbWSY = true;}
      else if(DiJetEObNs == ZPmMFYuzBH){gsyoiglDzU = true;}
      if(mFQAGUJrzr == fppVaQrecF){YTbgXrBJzV = true;}
      else if(fppVaQrecF == mFQAGUJrzr){PTxKNCLXVm = true;}
      if(sBlnrkbuwP == aTaunlOoLR){ngCPMndfKD = true;}
      else if(aTaunlOoLR == sBlnrkbuwP){ogEhFFultY = true;}
      if(yKOIOxqHLS == aTwGuKVccE){rySoZQGXxp = true;}
      else if(aTwGuKVccE == yKOIOxqHLS){yHVRwPatFK = true;}
      if(MKJzesqwmX == nYImJdBRQE){VBtGlieEwU = true;}
      else if(nYImJdBRQE == MKJzesqwmX){HgTMIgiXNM = true;}
      if(GuIrbXczTU == UcujajHOqI){LzMzHCPanV = true;}
      else if(UcujajHOqI == GuIrbXczTU){OQImWznPic = true;}
      if(wzClJcpHlI == pZQwSMsiaP){buMOfZBlGH = true;}
      if(dBfgehqmJy == uPBXRBKmlK){ldMIzmmwdd = true;}
      if(tfHXIDBVgp == jxXGwxXMfK){IYiorskTgX = true;}
      while(pZQwSMsiaP == wzClJcpHlI){kAJZVHxeOy = true;}
      while(uPBXRBKmlK == uPBXRBKmlK){aRRKeHNqZX = true;}
      while(jxXGwxXMfK == jxXGwxXMfK){WWXhKhyhuw = true;}
      if(DHkUNNIMUw == true){DHkUNNIMUw = false;}
      if(jSpAJFbWSY == true){jSpAJFbWSY = false;}
      if(YTbgXrBJzV == true){YTbgXrBJzV = false;}
      if(ngCPMndfKD == true){ngCPMndfKD = false;}
      if(rySoZQGXxp == true){rySoZQGXxp = false;}
      if(VBtGlieEwU == true){VBtGlieEwU = false;}
      if(LzMzHCPanV == true){LzMzHCPanV = false;}
      if(buMOfZBlGH == true){buMOfZBlGH = false;}
      if(ldMIzmmwdd == true){ldMIzmmwdd = false;}
      if(IYiorskTgX == true){IYiorskTgX = false;}
      if(BfiaOZneZL == true){BfiaOZneZL = false;}
      if(gsyoiglDzU == true){gsyoiglDzU = false;}
      if(PTxKNCLXVm == true){PTxKNCLXVm = false;}
      if(ogEhFFultY == true){ogEhFFultY = false;}
      if(yHVRwPatFK == true){yHVRwPatFK = false;}
      if(HgTMIgiXNM == true){HgTMIgiXNM = false;}
      if(OQImWznPic == true){OQImWznPic = false;}
      if(kAJZVHxeOy == true){kAJZVHxeOy = false;}
      if(aRRKeHNqZX == true){aRRKeHNqZX = false;}
      if(WWXhKhyhuw == true){WWXhKhyhuw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SEGPVOYQID
{ 
  void zZVrniFtAE()
  { 
      bool deoFgpWqCz = false;
      bool EnzQaVhkaB = false;
      bool DVUODFWVyV = false;
      bool jXCUcShDWJ = false;
      bool RUnfFIcEPs = false;
      bool aJYxOIKZBb = false;
      bool SAsgPfkenI = false;
      bool meOQiignIu = false;
      bool cRGEtZzEFU = false;
      bool IIBupQbHQZ = false;
      bool WxQGGwuHNf = false;
      bool bWQGyAsnlo = false;
      bool fgYTzedSaS = false;
      bool HZWiYeBzyy = false;
      bool drpZskquxo = false;
      bool WmUatoYwGb = false;
      bool RbfuYBzhgS = false;
      bool ztpnKmpzaO = false;
      bool miVkuTEuoi = false;
      bool GErWfCLuBq = false;
      string sYlklZXsgM;
      string BgxNkfCyLk;
      string AXuSPkHRtY;
      string WmAFgHKsre;
      string dNhFKFVlAY;
      string mNqRCUcTUA;
      string RFyHpNgrfc;
      string UJGgHmppHK;
      string OURUlIwiDL;
      string eqXqMXOZPw;
      string YdGWDdHKJO;
      string emLKkxndVg;
      string RxoTydkPTi;
      string YPZLkkOtQI;
      string BlZngqfNGP;
      string jjEhebolXZ;
      string sqpZoOMAYA;
      string hsNNefXAmH;
      string yWJEznLqfJ;
      string wbVqOJuzyR;
      if(sYlklZXsgM == YdGWDdHKJO){deoFgpWqCz = true;}
      else if(YdGWDdHKJO == sYlklZXsgM){WxQGGwuHNf = true;}
      if(BgxNkfCyLk == emLKkxndVg){EnzQaVhkaB = true;}
      else if(emLKkxndVg == BgxNkfCyLk){bWQGyAsnlo = true;}
      if(AXuSPkHRtY == RxoTydkPTi){DVUODFWVyV = true;}
      else if(RxoTydkPTi == AXuSPkHRtY){fgYTzedSaS = true;}
      if(WmAFgHKsre == YPZLkkOtQI){jXCUcShDWJ = true;}
      else if(YPZLkkOtQI == WmAFgHKsre){HZWiYeBzyy = true;}
      if(dNhFKFVlAY == BlZngqfNGP){RUnfFIcEPs = true;}
      else if(BlZngqfNGP == dNhFKFVlAY){drpZskquxo = true;}
      if(mNqRCUcTUA == jjEhebolXZ){aJYxOIKZBb = true;}
      else if(jjEhebolXZ == mNqRCUcTUA){WmUatoYwGb = true;}
      if(RFyHpNgrfc == sqpZoOMAYA){SAsgPfkenI = true;}
      else if(sqpZoOMAYA == RFyHpNgrfc){RbfuYBzhgS = true;}
      if(UJGgHmppHK == hsNNefXAmH){meOQiignIu = true;}
      if(OURUlIwiDL == yWJEznLqfJ){cRGEtZzEFU = true;}
      if(eqXqMXOZPw == wbVqOJuzyR){IIBupQbHQZ = true;}
      while(hsNNefXAmH == UJGgHmppHK){ztpnKmpzaO = true;}
      while(yWJEznLqfJ == yWJEznLqfJ){miVkuTEuoi = true;}
      while(wbVqOJuzyR == wbVqOJuzyR){GErWfCLuBq = true;}
      if(deoFgpWqCz == true){deoFgpWqCz = false;}
      if(EnzQaVhkaB == true){EnzQaVhkaB = false;}
      if(DVUODFWVyV == true){DVUODFWVyV = false;}
      if(jXCUcShDWJ == true){jXCUcShDWJ = false;}
      if(RUnfFIcEPs == true){RUnfFIcEPs = false;}
      if(aJYxOIKZBb == true){aJYxOIKZBb = false;}
      if(SAsgPfkenI == true){SAsgPfkenI = false;}
      if(meOQiignIu == true){meOQiignIu = false;}
      if(cRGEtZzEFU == true){cRGEtZzEFU = false;}
      if(IIBupQbHQZ == true){IIBupQbHQZ = false;}
      if(WxQGGwuHNf == true){WxQGGwuHNf = false;}
      if(bWQGyAsnlo == true){bWQGyAsnlo = false;}
      if(fgYTzedSaS == true){fgYTzedSaS = false;}
      if(HZWiYeBzyy == true){HZWiYeBzyy = false;}
      if(drpZskquxo == true){drpZskquxo = false;}
      if(WmUatoYwGb == true){WmUatoYwGb = false;}
      if(RbfuYBzhgS == true){RbfuYBzhgS = false;}
      if(ztpnKmpzaO == true){ztpnKmpzaO = false;}
      if(miVkuTEuoi == true){miVkuTEuoi = false;}
      if(GErWfCLuBq == true){GErWfCLuBq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WRDTHHTYLV
{ 
  void OdqjEaaSqp()
  { 
      bool jbKypRyEzZ = false;
      bool fpXpdxuTGt = false;
      bool uIlPMPNToT = false;
      bool fCPrXAqJxp = false;
      bool TyIjMBPeaW = false;
      bool pyaidbotka = false;
      bool EePesdZfRc = false;
      bool FkVPZxzBDK = false;
      bool zhsRchagNw = false;
      bool KwXWlohcph = false;
      bool aiNxREcKIg = false;
      bool bnqZaVkxCL = false;
      bool DsSVRINzdk = false;
      bool OVKUjQUuAX = false;
      bool EPYfUWFxwP = false;
      bool EkadehQqeI = false;
      bool qdgXSFcVpJ = false;
      bool BoxdGmTmAu = false;
      bool DQCNfcufoB = false;
      bool zcPqgahBMO = false;
      string zzbbUSLdlI;
      string zKZxJWOEfG;
      string CXJPFtYbdR;
      string LMIuRUQHKV;
      string pGMxizyMxM;
      string bIymAbpMWa;
      string tjATxlBefn;
      string FGtJPwKMfF;
      string uLeRoDOUto;
      string eEkibGGszw;
      string xTgIGVNoXD;
      string QdfQfhFidt;
      string XgjpJAVKgI;
      string pRMiIaeVKT;
      string oYwqYAYTDt;
      string BWOEYPipzF;
      string zDHONyQwIE;
      string DjYJFyjsRI;
      string bCHmHSDHKr;
      string qnVEUkRhcZ;
      if(zzbbUSLdlI == xTgIGVNoXD){jbKypRyEzZ = true;}
      else if(xTgIGVNoXD == zzbbUSLdlI){aiNxREcKIg = true;}
      if(zKZxJWOEfG == QdfQfhFidt){fpXpdxuTGt = true;}
      else if(QdfQfhFidt == zKZxJWOEfG){bnqZaVkxCL = true;}
      if(CXJPFtYbdR == XgjpJAVKgI){uIlPMPNToT = true;}
      else if(XgjpJAVKgI == CXJPFtYbdR){DsSVRINzdk = true;}
      if(LMIuRUQHKV == pRMiIaeVKT){fCPrXAqJxp = true;}
      else if(pRMiIaeVKT == LMIuRUQHKV){OVKUjQUuAX = true;}
      if(pGMxizyMxM == oYwqYAYTDt){TyIjMBPeaW = true;}
      else if(oYwqYAYTDt == pGMxizyMxM){EPYfUWFxwP = true;}
      if(bIymAbpMWa == BWOEYPipzF){pyaidbotka = true;}
      else if(BWOEYPipzF == bIymAbpMWa){EkadehQqeI = true;}
      if(tjATxlBefn == zDHONyQwIE){EePesdZfRc = true;}
      else if(zDHONyQwIE == tjATxlBefn){qdgXSFcVpJ = true;}
      if(FGtJPwKMfF == DjYJFyjsRI){FkVPZxzBDK = true;}
      if(uLeRoDOUto == bCHmHSDHKr){zhsRchagNw = true;}
      if(eEkibGGszw == qnVEUkRhcZ){KwXWlohcph = true;}
      while(DjYJFyjsRI == FGtJPwKMfF){BoxdGmTmAu = true;}
      while(bCHmHSDHKr == bCHmHSDHKr){DQCNfcufoB = true;}
      while(qnVEUkRhcZ == qnVEUkRhcZ){zcPqgahBMO = true;}
      if(jbKypRyEzZ == true){jbKypRyEzZ = false;}
      if(fpXpdxuTGt == true){fpXpdxuTGt = false;}
      if(uIlPMPNToT == true){uIlPMPNToT = false;}
      if(fCPrXAqJxp == true){fCPrXAqJxp = false;}
      if(TyIjMBPeaW == true){TyIjMBPeaW = false;}
      if(pyaidbotka == true){pyaidbotka = false;}
      if(EePesdZfRc == true){EePesdZfRc = false;}
      if(FkVPZxzBDK == true){FkVPZxzBDK = false;}
      if(zhsRchagNw == true){zhsRchagNw = false;}
      if(KwXWlohcph == true){KwXWlohcph = false;}
      if(aiNxREcKIg == true){aiNxREcKIg = false;}
      if(bnqZaVkxCL == true){bnqZaVkxCL = false;}
      if(DsSVRINzdk == true){DsSVRINzdk = false;}
      if(OVKUjQUuAX == true){OVKUjQUuAX = false;}
      if(EPYfUWFxwP == true){EPYfUWFxwP = false;}
      if(EkadehQqeI == true){EkadehQqeI = false;}
      if(qdgXSFcVpJ == true){qdgXSFcVpJ = false;}
      if(BoxdGmTmAu == true){BoxdGmTmAu = false;}
      if(DQCNfcufoB == true){DQCNfcufoB = false;}
      if(zcPqgahBMO == true){zcPqgahBMO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CNJUPJTQDO
{ 
  void iUQyQqWDmR()
  { 
      bool pjMZuuCZQG = false;
      bool nOgblmfVmB = false;
      bool CDMhBchmAc = false;
      bool lJnxHoBWPl = false;
      bool ZBJsQCJJrT = false;
      bool ByUIhmWdYS = false;
      bool MPZnTdaiUx = false;
      bool kJFAbAsTRy = false;
      bool SYtfbmofPU = false;
      bool RWxwYJbVYm = false;
      bool bZouiqrTgg = false;
      bool sDMZAnUsLf = false;
      bool ouTzfNLJaR = false;
      bool YXtdWsizxj = false;
      bool uUyiaOKWwk = false;
      bool oiJrxfFoPU = false;
      bool fisPHVIiDH = false;
      bool EVubkTwkXQ = false;
      bool DnMpcpNCfp = false;
      bool mticELbiaF = false;
      string JRjzAENfBR;
      string xwVACZPtkh;
      string pImZtaTYxG;
      string WeIUDggCPX;
      string mNwoolyGQZ;
      string ewUNEAdKFZ;
      string kohttSnAxV;
      string rMeNucMxjH;
      string bNHXbaJtPC;
      string CWxpmxeBuW;
      string yyItgRRHSb;
      string ngZxxXgJST;
      string IskuTVPoax;
      string yKCNJdEMqJ;
      string JBbmNUwArO;
      string PUiaNEMNaL;
      string CbgUddeMmS;
      string SUSIKGQxgy;
      string YOwLhcbazE;
      string RpleHgUHJj;
      if(JRjzAENfBR == yyItgRRHSb){pjMZuuCZQG = true;}
      else if(yyItgRRHSb == JRjzAENfBR){bZouiqrTgg = true;}
      if(xwVACZPtkh == ngZxxXgJST){nOgblmfVmB = true;}
      else if(ngZxxXgJST == xwVACZPtkh){sDMZAnUsLf = true;}
      if(pImZtaTYxG == IskuTVPoax){CDMhBchmAc = true;}
      else if(IskuTVPoax == pImZtaTYxG){ouTzfNLJaR = true;}
      if(WeIUDggCPX == yKCNJdEMqJ){lJnxHoBWPl = true;}
      else if(yKCNJdEMqJ == WeIUDggCPX){YXtdWsizxj = true;}
      if(mNwoolyGQZ == JBbmNUwArO){ZBJsQCJJrT = true;}
      else if(JBbmNUwArO == mNwoolyGQZ){uUyiaOKWwk = true;}
      if(ewUNEAdKFZ == PUiaNEMNaL){ByUIhmWdYS = true;}
      else if(PUiaNEMNaL == ewUNEAdKFZ){oiJrxfFoPU = true;}
      if(kohttSnAxV == CbgUddeMmS){MPZnTdaiUx = true;}
      else if(CbgUddeMmS == kohttSnAxV){fisPHVIiDH = true;}
      if(rMeNucMxjH == SUSIKGQxgy){kJFAbAsTRy = true;}
      if(bNHXbaJtPC == YOwLhcbazE){SYtfbmofPU = true;}
      if(CWxpmxeBuW == RpleHgUHJj){RWxwYJbVYm = true;}
      while(SUSIKGQxgy == rMeNucMxjH){EVubkTwkXQ = true;}
      while(YOwLhcbazE == YOwLhcbazE){DnMpcpNCfp = true;}
      while(RpleHgUHJj == RpleHgUHJj){mticELbiaF = true;}
      if(pjMZuuCZQG == true){pjMZuuCZQG = false;}
      if(nOgblmfVmB == true){nOgblmfVmB = false;}
      if(CDMhBchmAc == true){CDMhBchmAc = false;}
      if(lJnxHoBWPl == true){lJnxHoBWPl = false;}
      if(ZBJsQCJJrT == true){ZBJsQCJJrT = false;}
      if(ByUIhmWdYS == true){ByUIhmWdYS = false;}
      if(MPZnTdaiUx == true){MPZnTdaiUx = false;}
      if(kJFAbAsTRy == true){kJFAbAsTRy = false;}
      if(SYtfbmofPU == true){SYtfbmofPU = false;}
      if(RWxwYJbVYm == true){RWxwYJbVYm = false;}
      if(bZouiqrTgg == true){bZouiqrTgg = false;}
      if(sDMZAnUsLf == true){sDMZAnUsLf = false;}
      if(ouTzfNLJaR == true){ouTzfNLJaR = false;}
      if(YXtdWsizxj == true){YXtdWsizxj = false;}
      if(uUyiaOKWwk == true){uUyiaOKWwk = false;}
      if(oiJrxfFoPU == true){oiJrxfFoPU = false;}
      if(fisPHVIiDH == true){fisPHVIiDH = false;}
      if(EVubkTwkXQ == true){EVubkTwkXQ = false;}
      if(DnMpcpNCfp == true){DnMpcpNCfp = false;}
      if(mticELbiaF == true){mticELbiaF = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RGAUIYJTVO
{ 
  void XwgoddwQbP()
  { 
      bool LpQtVmVFrO = false;
      bool iSNejFOUCA = false;
      bool aawtCWLbxx = false;
      bool IAcANJZoad = false;
      bool xwDIPCRIfG = false;
      bool zqDYCjtWqz = false;
      bool btItGhoNQa = false;
      bool fYsXCdXmdz = false;
      bool EDupxQxDxc = false;
      bool OCxbCDnICg = false;
      bool NDqFaaxeVX = false;
      bool MHsqtYyRwo = false;
      bool kkQmLcaRVz = false;
      bool oOjxlBLpOW = false;
      bool SRhYQHJGgd = false;
      bool EWLNubmDsy = false;
      bool zfJVhxWwFs = false;
      bool MICYVTDqWE = false;
      bool TUjrtSDWBK = false;
      bool NIwQXmILpS = false;
      string TkIAblOZag;
      string ndYqOCYjpx;
      string jSmpJINXly;
      string wXAjECFgAW;
      string SuOMzzDOWq;
      string ojGCpAxSpT;
      string YqmcjQdnmL;
      string xqiTeoEapJ;
      string FyBtIMRxtD;
      string ElpgybHkbi;
      string VOaKtyAmXB;
      string wWbgOZorUX;
      string zftjHKBCVD;
      string hSzHnpWpAK;
      string LdZrGhKJSX;
      string lOQDjoUqDj;
      string xyRTTcWTrX;
      string CyrKLUGCcY;
      string zqixRSCuXy;
      string VfmQMNMFYc;
      if(TkIAblOZag == VOaKtyAmXB){LpQtVmVFrO = true;}
      else if(VOaKtyAmXB == TkIAblOZag){NDqFaaxeVX = true;}
      if(ndYqOCYjpx == wWbgOZorUX){iSNejFOUCA = true;}
      else if(wWbgOZorUX == ndYqOCYjpx){MHsqtYyRwo = true;}
      if(jSmpJINXly == zftjHKBCVD){aawtCWLbxx = true;}
      else if(zftjHKBCVD == jSmpJINXly){kkQmLcaRVz = true;}
      if(wXAjECFgAW == hSzHnpWpAK){IAcANJZoad = true;}
      else if(hSzHnpWpAK == wXAjECFgAW){oOjxlBLpOW = true;}
      if(SuOMzzDOWq == LdZrGhKJSX){xwDIPCRIfG = true;}
      else if(LdZrGhKJSX == SuOMzzDOWq){SRhYQHJGgd = true;}
      if(ojGCpAxSpT == lOQDjoUqDj){zqDYCjtWqz = true;}
      else if(lOQDjoUqDj == ojGCpAxSpT){EWLNubmDsy = true;}
      if(YqmcjQdnmL == xyRTTcWTrX){btItGhoNQa = true;}
      else if(xyRTTcWTrX == YqmcjQdnmL){zfJVhxWwFs = true;}
      if(xqiTeoEapJ == CyrKLUGCcY){fYsXCdXmdz = true;}
      if(FyBtIMRxtD == zqixRSCuXy){EDupxQxDxc = true;}
      if(ElpgybHkbi == VfmQMNMFYc){OCxbCDnICg = true;}
      while(CyrKLUGCcY == xqiTeoEapJ){MICYVTDqWE = true;}
      while(zqixRSCuXy == zqixRSCuXy){TUjrtSDWBK = true;}
      while(VfmQMNMFYc == VfmQMNMFYc){NIwQXmILpS = true;}
      if(LpQtVmVFrO == true){LpQtVmVFrO = false;}
      if(iSNejFOUCA == true){iSNejFOUCA = false;}
      if(aawtCWLbxx == true){aawtCWLbxx = false;}
      if(IAcANJZoad == true){IAcANJZoad = false;}
      if(xwDIPCRIfG == true){xwDIPCRIfG = false;}
      if(zqDYCjtWqz == true){zqDYCjtWqz = false;}
      if(btItGhoNQa == true){btItGhoNQa = false;}
      if(fYsXCdXmdz == true){fYsXCdXmdz = false;}
      if(EDupxQxDxc == true){EDupxQxDxc = false;}
      if(OCxbCDnICg == true){OCxbCDnICg = false;}
      if(NDqFaaxeVX == true){NDqFaaxeVX = false;}
      if(MHsqtYyRwo == true){MHsqtYyRwo = false;}
      if(kkQmLcaRVz == true){kkQmLcaRVz = false;}
      if(oOjxlBLpOW == true){oOjxlBLpOW = false;}
      if(SRhYQHJGgd == true){SRhYQHJGgd = false;}
      if(EWLNubmDsy == true){EWLNubmDsy = false;}
      if(zfJVhxWwFs == true){zfJVhxWwFs = false;}
      if(MICYVTDqWE == true){MICYVTDqWE = false;}
      if(TUjrtSDWBK == true){TUjrtSDWBK = false;}
      if(NIwQXmILpS == true){NIwQXmILpS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KSBBQMJPKW
{ 
  void WcaQZmHlhF()
  { 
      bool baBVWigbdG = false;
      bool clRHrBKnZO = false;
      bool IaWmzjMHiD = false;
      bool woAwRqmubc = false;
      bool DuTjbcjoTD = false;
      bool IIJPzADOWl = false;
      bool WZsaKmkUoA = false;
      bool OOLxKMHmKV = false;
      bool AXmftmfkwQ = false;
      bool mXYoGdCrYo = false;
      bool qOzTntxMFl = false;
      bool sPEAlylNfX = false;
      bool HAXXMqpWqg = false;
      bool KOOVdkXTqA = false;
      bool oSeKXWTYDH = false;
      bool qoLBgqqaGx = false;
      bool EaUXbdBhVA = false;
      bool ZSVfhnUNAo = false;
      bool WRSipVCMnm = false;
      bool txMtuOxHit = false;
      string IRcdJLJmdP;
      string LbxZoOhKwF;
      string wxZGVHKKHX;
      string bCOYVTOEfT;
      string zUHGmMzVRS;
      string CFWyKLrWIC;
      string AzeFnJMXEV;
      string AddbFapSSI;
      string HoroUVfPNK;
      string arudTzpBSc;
      string zbQuSYburj;
      string UZDAtKGpjR;
      string dxJtuHiAHF;
      string knzpIfYxWw;
      string tjWdoBotiI;
      string sAQBzCRWAK;
      string pammzfZNrN;
      string lJSnWsZfFs;
      string SnQyMjXypF;
      string OBwLFTxjeI;
      if(IRcdJLJmdP == zbQuSYburj){baBVWigbdG = true;}
      else if(zbQuSYburj == IRcdJLJmdP){qOzTntxMFl = true;}
      if(LbxZoOhKwF == UZDAtKGpjR){clRHrBKnZO = true;}
      else if(UZDAtKGpjR == LbxZoOhKwF){sPEAlylNfX = true;}
      if(wxZGVHKKHX == dxJtuHiAHF){IaWmzjMHiD = true;}
      else if(dxJtuHiAHF == wxZGVHKKHX){HAXXMqpWqg = true;}
      if(bCOYVTOEfT == knzpIfYxWw){woAwRqmubc = true;}
      else if(knzpIfYxWw == bCOYVTOEfT){KOOVdkXTqA = true;}
      if(zUHGmMzVRS == tjWdoBotiI){DuTjbcjoTD = true;}
      else if(tjWdoBotiI == zUHGmMzVRS){oSeKXWTYDH = true;}
      if(CFWyKLrWIC == sAQBzCRWAK){IIJPzADOWl = true;}
      else if(sAQBzCRWAK == CFWyKLrWIC){qoLBgqqaGx = true;}
      if(AzeFnJMXEV == pammzfZNrN){WZsaKmkUoA = true;}
      else if(pammzfZNrN == AzeFnJMXEV){EaUXbdBhVA = true;}
      if(AddbFapSSI == lJSnWsZfFs){OOLxKMHmKV = true;}
      if(HoroUVfPNK == SnQyMjXypF){AXmftmfkwQ = true;}
      if(arudTzpBSc == OBwLFTxjeI){mXYoGdCrYo = true;}
      while(lJSnWsZfFs == AddbFapSSI){ZSVfhnUNAo = true;}
      while(SnQyMjXypF == SnQyMjXypF){WRSipVCMnm = true;}
      while(OBwLFTxjeI == OBwLFTxjeI){txMtuOxHit = true;}
      if(baBVWigbdG == true){baBVWigbdG = false;}
      if(clRHrBKnZO == true){clRHrBKnZO = false;}
      if(IaWmzjMHiD == true){IaWmzjMHiD = false;}
      if(woAwRqmubc == true){woAwRqmubc = false;}
      if(DuTjbcjoTD == true){DuTjbcjoTD = false;}
      if(IIJPzADOWl == true){IIJPzADOWl = false;}
      if(WZsaKmkUoA == true){WZsaKmkUoA = false;}
      if(OOLxKMHmKV == true){OOLxKMHmKV = false;}
      if(AXmftmfkwQ == true){AXmftmfkwQ = false;}
      if(mXYoGdCrYo == true){mXYoGdCrYo = false;}
      if(qOzTntxMFl == true){qOzTntxMFl = false;}
      if(sPEAlylNfX == true){sPEAlylNfX = false;}
      if(HAXXMqpWqg == true){HAXXMqpWqg = false;}
      if(KOOVdkXTqA == true){KOOVdkXTqA = false;}
      if(oSeKXWTYDH == true){oSeKXWTYDH = false;}
      if(qoLBgqqaGx == true){qoLBgqqaGx = false;}
      if(EaUXbdBhVA == true){EaUXbdBhVA = false;}
      if(ZSVfhnUNAo == true){ZSVfhnUNAo = false;}
      if(WRSipVCMnm == true){WRSipVCMnm = false;}
      if(txMtuOxHit == true){txMtuOxHit = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QGKESFJYAG
{ 
  void irYNtahciV()
  { 
      bool BguptWSxSY = false;
      bool XqbRdYNKjO = false;
      bool trZIHaMuCC = false;
      bool hrZwtCOtks = false;
      bool JLieLDtjLg = false;
      bool dtgYwIGliT = false;
      bool JWtPpeReXa = false;
      bool KSZqFiJuwE = false;
      bool fcjVYnKAAk = false;
      bool wcKRimQSng = false;
      bool kPLkWKUpTN = false;
      bool zLoemgmYqC = false;
      bool TReldhIIHS = false;
      bool RqGPSQQyyN = false;
      bool SUgllkmfdC = false;
      bool ArakMTjYGo = false;
      bool CMiBuHEYLV = false;
      bool QnZcjzjrAs = false;
      bool tbdFRZPUDz = false;
      bool fqqJLGkepd = false;
      string pVGKzCWbZS;
      string CcyCOUcWoi;
      string VAExaxhVZB;
      string RNjBEUQgyt;
      string RwdujBxxAM;
      string dBHSxpQJiC;
      string TACXAorxyt;
      string DuYuRZWJMZ;
      string zkVMXQGRDO;
      string DHxuZQsGuu;
      string IzzFJhbLAX;
      string THwJCieozi;
      string hatiquydVB;
      string fPNRQAlPDp;
      string otQfuuLnsq;
      string kEGMMgFPNN;
      string iyozqDterC;
      string wOkJsnJNnu;
      string jKkePbIBEj;
      string tVCpGqFRnq;
      if(pVGKzCWbZS == IzzFJhbLAX){BguptWSxSY = true;}
      else if(IzzFJhbLAX == pVGKzCWbZS){kPLkWKUpTN = true;}
      if(CcyCOUcWoi == THwJCieozi){XqbRdYNKjO = true;}
      else if(THwJCieozi == CcyCOUcWoi){zLoemgmYqC = true;}
      if(VAExaxhVZB == hatiquydVB){trZIHaMuCC = true;}
      else if(hatiquydVB == VAExaxhVZB){TReldhIIHS = true;}
      if(RNjBEUQgyt == fPNRQAlPDp){hrZwtCOtks = true;}
      else if(fPNRQAlPDp == RNjBEUQgyt){RqGPSQQyyN = true;}
      if(RwdujBxxAM == otQfuuLnsq){JLieLDtjLg = true;}
      else if(otQfuuLnsq == RwdujBxxAM){SUgllkmfdC = true;}
      if(dBHSxpQJiC == kEGMMgFPNN){dtgYwIGliT = true;}
      else if(kEGMMgFPNN == dBHSxpQJiC){ArakMTjYGo = true;}
      if(TACXAorxyt == iyozqDterC){JWtPpeReXa = true;}
      else if(iyozqDterC == TACXAorxyt){CMiBuHEYLV = true;}
      if(DuYuRZWJMZ == wOkJsnJNnu){KSZqFiJuwE = true;}
      if(zkVMXQGRDO == jKkePbIBEj){fcjVYnKAAk = true;}
      if(DHxuZQsGuu == tVCpGqFRnq){wcKRimQSng = true;}
      while(wOkJsnJNnu == DuYuRZWJMZ){QnZcjzjrAs = true;}
      while(jKkePbIBEj == jKkePbIBEj){tbdFRZPUDz = true;}
      while(tVCpGqFRnq == tVCpGqFRnq){fqqJLGkepd = true;}
      if(BguptWSxSY == true){BguptWSxSY = false;}
      if(XqbRdYNKjO == true){XqbRdYNKjO = false;}
      if(trZIHaMuCC == true){trZIHaMuCC = false;}
      if(hrZwtCOtks == true){hrZwtCOtks = false;}
      if(JLieLDtjLg == true){JLieLDtjLg = false;}
      if(dtgYwIGliT == true){dtgYwIGliT = false;}
      if(JWtPpeReXa == true){JWtPpeReXa = false;}
      if(KSZqFiJuwE == true){KSZqFiJuwE = false;}
      if(fcjVYnKAAk == true){fcjVYnKAAk = false;}
      if(wcKRimQSng == true){wcKRimQSng = false;}
      if(kPLkWKUpTN == true){kPLkWKUpTN = false;}
      if(zLoemgmYqC == true){zLoemgmYqC = false;}
      if(TReldhIIHS == true){TReldhIIHS = false;}
      if(RqGPSQQyyN == true){RqGPSQQyyN = false;}
      if(SUgllkmfdC == true){SUgllkmfdC = false;}
      if(ArakMTjYGo == true){ArakMTjYGo = false;}
      if(CMiBuHEYLV == true){CMiBuHEYLV = false;}
      if(QnZcjzjrAs == true){QnZcjzjrAs = false;}
      if(tbdFRZPUDz == true){tbdFRZPUDz = false;}
      if(fqqJLGkepd == true){fqqJLGkepd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SPROADKNOK
{ 
  void LLQXxgWYOT()
  { 
      bool LGHOZldWeV = false;
      bool OPyVJNFdLS = false;
      bool KnTHBRuCPU = false;
      bool bIoMDaLNBC = false;
      bool XGuhorjIHD = false;
      bool LtWqbXlzMH = false;
      bool paGJsFZkRi = false;
      bool CrbJuDrlRI = false;
      bool gmuLJglxke = false;
      bool nLAtLwzlni = false;
      bool RzpYBTSNwX = false;
      bool eAgDZDftAM = false;
      bool TcAltpqhyy = false;
      bool EGxiphwVKo = false;
      bool NXAMAndwTH = false;
      bool QEkaPoyDeK = false;
      bool bjPxSXCyNq = false;
      bool GJjKyucmlG = false;
      bool MuNphzUVSB = false;
      bool jsqANdIDsb = false;
      string sUJorlQfJt;
      string QqeJBhbKGf;
      string mDUncszltK;
      string YCixmfxAEu;
      string eLiYOmbtwl;
      string soRBwpAVLz;
      string mdkyVOHgWU;
      string kaGKBaTuYp;
      string REpIRijhBx;
      string eaoFIAakDz;
      string etXaASiGAU;
      string VFxVghnbgl;
      string dYfZoKlABA;
      string AhqYKWgOHy;
      string zdhfaZdSYa;
      string LyEQVGOxkp;
      string ghEzUnNXLJ;
      string BZUhlAqzzP;
      string gSbanBcZDE;
      string HidRRRYhKl;
      if(sUJorlQfJt == etXaASiGAU){LGHOZldWeV = true;}
      else if(etXaASiGAU == sUJorlQfJt){RzpYBTSNwX = true;}
      if(QqeJBhbKGf == VFxVghnbgl){OPyVJNFdLS = true;}
      else if(VFxVghnbgl == QqeJBhbKGf){eAgDZDftAM = true;}
      if(mDUncszltK == dYfZoKlABA){KnTHBRuCPU = true;}
      else if(dYfZoKlABA == mDUncszltK){TcAltpqhyy = true;}
      if(YCixmfxAEu == AhqYKWgOHy){bIoMDaLNBC = true;}
      else if(AhqYKWgOHy == YCixmfxAEu){EGxiphwVKo = true;}
      if(eLiYOmbtwl == zdhfaZdSYa){XGuhorjIHD = true;}
      else if(zdhfaZdSYa == eLiYOmbtwl){NXAMAndwTH = true;}
      if(soRBwpAVLz == LyEQVGOxkp){LtWqbXlzMH = true;}
      else if(LyEQVGOxkp == soRBwpAVLz){QEkaPoyDeK = true;}
      if(mdkyVOHgWU == ghEzUnNXLJ){paGJsFZkRi = true;}
      else if(ghEzUnNXLJ == mdkyVOHgWU){bjPxSXCyNq = true;}
      if(kaGKBaTuYp == BZUhlAqzzP){CrbJuDrlRI = true;}
      if(REpIRijhBx == gSbanBcZDE){gmuLJglxke = true;}
      if(eaoFIAakDz == HidRRRYhKl){nLAtLwzlni = true;}
      while(BZUhlAqzzP == kaGKBaTuYp){GJjKyucmlG = true;}
      while(gSbanBcZDE == gSbanBcZDE){MuNphzUVSB = true;}
      while(HidRRRYhKl == HidRRRYhKl){jsqANdIDsb = true;}
      if(LGHOZldWeV == true){LGHOZldWeV = false;}
      if(OPyVJNFdLS == true){OPyVJNFdLS = false;}
      if(KnTHBRuCPU == true){KnTHBRuCPU = false;}
      if(bIoMDaLNBC == true){bIoMDaLNBC = false;}
      if(XGuhorjIHD == true){XGuhorjIHD = false;}
      if(LtWqbXlzMH == true){LtWqbXlzMH = false;}
      if(paGJsFZkRi == true){paGJsFZkRi = false;}
      if(CrbJuDrlRI == true){CrbJuDrlRI = false;}
      if(gmuLJglxke == true){gmuLJglxke = false;}
      if(nLAtLwzlni == true){nLAtLwzlni = false;}
      if(RzpYBTSNwX == true){RzpYBTSNwX = false;}
      if(eAgDZDftAM == true){eAgDZDftAM = false;}
      if(TcAltpqhyy == true){TcAltpqhyy = false;}
      if(EGxiphwVKo == true){EGxiphwVKo = false;}
      if(NXAMAndwTH == true){NXAMAndwTH = false;}
      if(QEkaPoyDeK == true){QEkaPoyDeK = false;}
      if(bjPxSXCyNq == true){bjPxSXCyNq = false;}
      if(GJjKyucmlG == true){GJjKyucmlG = false;}
      if(MuNphzUVSB == true){MuNphzUVSB = false;}
      if(jsqANdIDsb == true){jsqANdIDsb = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KMZGGHEEGJ
{ 
  void kBHlGAYeTH()
  { 
      bool sTAUsZZEHq = false;
      bool NzZMFGBOnB = false;
      bool TcYOWYSOmr = false;
      bool JUraJRgjmh = false;
      bool NCbQWeJBYh = false;
      bool rDKxNQukxT = false;
      bool HLROgrGzDn = false;
      bool YQbFmRAHrr = false;
      bool OQpFAGJuMs = false;
      bool WwmSaCkMjd = false;
      bool XKykIabEJw = false;
      bool PmrEEpVLOa = false;
      bool iLmJQcdIek = false;
      bool gnXNUDSExZ = false;
      bool eFkPdiDhOj = false;
      bool gYyJuLXnTe = false;
      bool wSZXPbOeiA = false;
      bool WQOnucZWfg = false;
      bool YuttEIoORI = false;
      bool fekRcLfMAB = false;
      string BHFueHqqTu;
      string DOfDBzkQkH;
      string iNHOVVLecP;
      string cbwSMwBoAZ;
      string bunygMhrcc;
      string RxnpjnApZJ;
      string uLNDRaSNqE;
      string ywQLenYJhJ;
      string nTBzyfnFiY;
      string wKPpdQgkfG;
      string sTBXdUfbKu;
      string OpIPUbJNEl;
      string mNdSImgZhO;
      string GmdaQTScUc;
      string zLtmJFBzVC;
      string GSftSfpiPq;
      string nGtwoaMhzB;
      string IfsblaSpIC;
      string kgEfInGjIj;
      string djNdnsRyyh;
      if(BHFueHqqTu == sTBXdUfbKu){sTAUsZZEHq = true;}
      else if(sTBXdUfbKu == BHFueHqqTu){XKykIabEJw = true;}
      if(DOfDBzkQkH == OpIPUbJNEl){NzZMFGBOnB = true;}
      else if(OpIPUbJNEl == DOfDBzkQkH){PmrEEpVLOa = true;}
      if(iNHOVVLecP == mNdSImgZhO){TcYOWYSOmr = true;}
      else if(mNdSImgZhO == iNHOVVLecP){iLmJQcdIek = true;}
      if(cbwSMwBoAZ == GmdaQTScUc){JUraJRgjmh = true;}
      else if(GmdaQTScUc == cbwSMwBoAZ){gnXNUDSExZ = true;}
      if(bunygMhrcc == zLtmJFBzVC){NCbQWeJBYh = true;}
      else if(zLtmJFBzVC == bunygMhrcc){eFkPdiDhOj = true;}
      if(RxnpjnApZJ == GSftSfpiPq){rDKxNQukxT = true;}
      else if(GSftSfpiPq == RxnpjnApZJ){gYyJuLXnTe = true;}
      if(uLNDRaSNqE == nGtwoaMhzB){HLROgrGzDn = true;}
      else if(nGtwoaMhzB == uLNDRaSNqE){wSZXPbOeiA = true;}
      if(ywQLenYJhJ == IfsblaSpIC){YQbFmRAHrr = true;}
      if(nTBzyfnFiY == kgEfInGjIj){OQpFAGJuMs = true;}
      if(wKPpdQgkfG == djNdnsRyyh){WwmSaCkMjd = true;}
      while(IfsblaSpIC == ywQLenYJhJ){WQOnucZWfg = true;}
      while(kgEfInGjIj == kgEfInGjIj){YuttEIoORI = true;}
      while(djNdnsRyyh == djNdnsRyyh){fekRcLfMAB = true;}
      if(sTAUsZZEHq == true){sTAUsZZEHq = false;}
      if(NzZMFGBOnB == true){NzZMFGBOnB = false;}
      if(TcYOWYSOmr == true){TcYOWYSOmr = false;}
      if(JUraJRgjmh == true){JUraJRgjmh = false;}
      if(NCbQWeJBYh == true){NCbQWeJBYh = false;}
      if(rDKxNQukxT == true){rDKxNQukxT = false;}
      if(HLROgrGzDn == true){HLROgrGzDn = false;}
      if(YQbFmRAHrr == true){YQbFmRAHrr = false;}
      if(OQpFAGJuMs == true){OQpFAGJuMs = false;}
      if(WwmSaCkMjd == true){WwmSaCkMjd = false;}
      if(XKykIabEJw == true){XKykIabEJw = false;}
      if(PmrEEpVLOa == true){PmrEEpVLOa = false;}
      if(iLmJQcdIek == true){iLmJQcdIek = false;}
      if(gnXNUDSExZ == true){gnXNUDSExZ = false;}
      if(eFkPdiDhOj == true){eFkPdiDhOj = false;}
      if(gYyJuLXnTe == true){gYyJuLXnTe = false;}
      if(wSZXPbOeiA == true){wSZXPbOeiA = false;}
      if(WQOnucZWfg == true){WQOnucZWfg = false;}
      if(YuttEIoORI == true){YuttEIoORI = false;}
      if(fekRcLfMAB == true){fekRcLfMAB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PLOHBRKEWF
{ 
  void EwKDyDcbmd()
  { 
      bool FaAcBGxqzn = false;
      bool YpFYZKPusb = false;
      bool qRdjSpAKDu = false;
      bool zSLnYrCDQr = false;
      bool xfyMkUCSqc = false;
      bool TTpJtmxQKK = false;
      bool FlnEoFeRzg = false;
      bool ILdAdKTRyy = false;
      bool mgTZbecBsl = false;
      bool ulsGlLrtXe = false;
      bool okTJUHoofj = false;
      bool TeRMWYSNbJ = false;
      bool ccYNkVSXNS = false;
      bool RZtTICNJET = false;
      bool pmVIRPUgiN = false;
      bool rdRNBMLFXz = false;
      bool oRbMRtkyDU = false;
      bool KJIITpHPuU = false;
      bool AlozZmHfRu = false;
      bool pHkFdXLIqX = false;
      string fxjbPhwwJu;
      string wofcpAabZA;
      string SYjyBtANZA;
      string UtpZdbeCEU;
      string SjnQxGAFQq;
      string kFkXZHMlaV;
      string hiWJZByGyN;
      string UYWEhhkTAt;
      string suwhOOJSAW;
      string OsailjNdpu;
      string HtGCWLifpq;
      string VskgbmlntK;
      string rfGazbxmRx;
      string UGcFlyntLC;
      string fccIBUHtGz;
      string yZieDbnfCt;
      string OwNrWOPftd;
      string yqbqeODxZd;
      string TDgSzuXyBK;
      string KIqRniPjll;
      if(fxjbPhwwJu == HtGCWLifpq){FaAcBGxqzn = true;}
      else if(HtGCWLifpq == fxjbPhwwJu){okTJUHoofj = true;}
      if(wofcpAabZA == VskgbmlntK){YpFYZKPusb = true;}
      else if(VskgbmlntK == wofcpAabZA){TeRMWYSNbJ = true;}
      if(SYjyBtANZA == rfGazbxmRx){qRdjSpAKDu = true;}
      else if(rfGazbxmRx == SYjyBtANZA){ccYNkVSXNS = true;}
      if(UtpZdbeCEU == UGcFlyntLC){zSLnYrCDQr = true;}
      else if(UGcFlyntLC == UtpZdbeCEU){RZtTICNJET = true;}
      if(SjnQxGAFQq == fccIBUHtGz){xfyMkUCSqc = true;}
      else if(fccIBUHtGz == SjnQxGAFQq){pmVIRPUgiN = true;}
      if(kFkXZHMlaV == yZieDbnfCt){TTpJtmxQKK = true;}
      else if(yZieDbnfCt == kFkXZHMlaV){rdRNBMLFXz = true;}
      if(hiWJZByGyN == OwNrWOPftd){FlnEoFeRzg = true;}
      else if(OwNrWOPftd == hiWJZByGyN){oRbMRtkyDU = true;}
      if(UYWEhhkTAt == yqbqeODxZd){ILdAdKTRyy = true;}
      if(suwhOOJSAW == TDgSzuXyBK){mgTZbecBsl = true;}
      if(OsailjNdpu == KIqRniPjll){ulsGlLrtXe = true;}
      while(yqbqeODxZd == UYWEhhkTAt){KJIITpHPuU = true;}
      while(TDgSzuXyBK == TDgSzuXyBK){AlozZmHfRu = true;}
      while(KIqRniPjll == KIqRniPjll){pHkFdXLIqX = true;}
      if(FaAcBGxqzn == true){FaAcBGxqzn = false;}
      if(YpFYZKPusb == true){YpFYZKPusb = false;}
      if(qRdjSpAKDu == true){qRdjSpAKDu = false;}
      if(zSLnYrCDQr == true){zSLnYrCDQr = false;}
      if(xfyMkUCSqc == true){xfyMkUCSqc = false;}
      if(TTpJtmxQKK == true){TTpJtmxQKK = false;}
      if(FlnEoFeRzg == true){FlnEoFeRzg = false;}
      if(ILdAdKTRyy == true){ILdAdKTRyy = false;}
      if(mgTZbecBsl == true){mgTZbecBsl = false;}
      if(ulsGlLrtXe == true){ulsGlLrtXe = false;}
      if(okTJUHoofj == true){okTJUHoofj = false;}
      if(TeRMWYSNbJ == true){TeRMWYSNbJ = false;}
      if(ccYNkVSXNS == true){ccYNkVSXNS = false;}
      if(RZtTICNJET == true){RZtTICNJET = false;}
      if(pmVIRPUgiN == true){pmVIRPUgiN = false;}
      if(rdRNBMLFXz == true){rdRNBMLFXz = false;}
      if(oRbMRtkyDU == true){oRbMRtkyDU = false;}
      if(KJIITpHPuU == true){KJIITpHPuU = false;}
      if(AlozZmHfRu == true){AlozZmHfRu = false;}
      if(pHkFdXLIqX == true){pHkFdXLIqX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CIFVWJVDBL
{ 
  void irppBEXVwO()
  { 
      bool HeuRunUTDX = false;
      bool KXXHLdyMRQ = false;
      bool atIKAbbMqj = false;
      bool KTKmptGLgh = false;
      bool xImpnEmNbV = false;
      bool DCHHfNWkQT = false;
      bool LIbTNqNjQR = false;
      bool WmejSSGRVw = false;
      bool oOjVfHLCHc = false;
      bool XVtNVofTcw = false;
      bool qCwKyQWXnN = false;
      bool GEPUexXGmF = false;
      bool fKcntBrQNd = false;
      bool lmoYFlRZWO = false;
      bool kmNtWSGYiY = false;
      bool QZfXQfZIQx = false;
      bool XFKJaSOpOr = false;
      bool eyiYUrwhRa = false;
      bool uThnNLHRIZ = false;
      bool LJIZpecDSD = false;
      string GkPhljBRiH;
      string EBrGEObhaT;
      string hxjwCsTJIn;
      string MUebGHDzBP;
      string BCczKjMVpX;
      string RhNQAIATfr;
      string PqIttjWQAw;
      string LbgLSfUkkR;
      string fyTjWeGNax;
      string HjTIzJshFa;
      string NwUsLoAByU;
      string nPhNMQXYXP;
      string dIoCoaIYLI;
      string llfKEicZXg;
      string dEPGIqCFui;
      string lUKowEGREk;
      string gboohCjWJc;
      string mxAqlYsitx;
      string YCccWRbpNz;
      string bLUmzCOZXG;
      if(GkPhljBRiH == NwUsLoAByU){HeuRunUTDX = true;}
      else if(NwUsLoAByU == GkPhljBRiH){qCwKyQWXnN = true;}
      if(EBrGEObhaT == nPhNMQXYXP){KXXHLdyMRQ = true;}
      else if(nPhNMQXYXP == EBrGEObhaT){GEPUexXGmF = true;}
      if(hxjwCsTJIn == dIoCoaIYLI){atIKAbbMqj = true;}
      else if(dIoCoaIYLI == hxjwCsTJIn){fKcntBrQNd = true;}
      if(MUebGHDzBP == llfKEicZXg){KTKmptGLgh = true;}
      else if(llfKEicZXg == MUebGHDzBP){lmoYFlRZWO = true;}
      if(BCczKjMVpX == dEPGIqCFui){xImpnEmNbV = true;}
      else if(dEPGIqCFui == BCczKjMVpX){kmNtWSGYiY = true;}
      if(RhNQAIATfr == lUKowEGREk){DCHHfNWkQT = true;}
      else if(lUKowEGREk == RhNQAIATfr){QZfXQfZIQx = true;}
      if(PqIttjWQAw == gboohCjWJc){LIbTNqNjQR = true;}
      else if(gboohCjWJc == PqIttjWQAw){XFKJaSOpOr = true;}
      if(LbgLSfUkkR == mxAqlYsitx){WmejSSGRVw = true;}
      if(fyTjWeGNax == YCccWRbpNz){oOjVfHLCHc = true;}
      if(HjTIzJshFa == bLUmzCOZXG){XVtNVofTcw = true;}
      while(mxAqlYsitx == LbgLSfUkkR){eyiYUrwhRa = true;}
      while(YCccWRbpNz == YCccWRbpNz){uThnNLHRIZ = true;}
      while(bLUmzCOZXG == bLUmzCOZXG){LJIZpecDSD = true;}
      if(HeuRunUTDX == true){HeuRunUTDX = false;}
      if(KXXHLdyMRQ == true){KXXHLdyMRQ = false;}
      if(atIKAbbMqj == true){atIKAbbMqj = false;}
      if(KTKmptGLgh == true){KTKmptGLgh = false;}
      if(xImpnEmNbV == true){xImpnEmNbV = false;}
      if(DCHHfNWkQT == true){DCHHfNWkQT = false;}
      if(LIbTNqNjQR == true){LIbTNqNjQR = false;}
      if(WmejSSGRVw == true){WmejSSGRVw = false;}
      if(oOjVfHLCHc == true){oOjVfHLCHc = false;}
      if(XVtNVofTcw == true){XVtNVofTcw = false;}
      if(qCwKyQWXnN == true){qCwKyQWXnN = false;}
      if(GEPUexXGmF == true){GEPUexXGmF = false;}
      if(fKcntBrQNd == true){fKcntBrQNd = false;}
      if(lmoYFlRZWO == true){lmoYFlRZWO = false;}
      if(kmNtWSGYiY == true){kmNtWSGYiY = false;}
      if(QZfXQfZIQx == true){QZfXQfZIQx = false;}
      if(XFKJaSOpOr == true){XFKJaSOpOr = false;}
      if(eyiYUrwhRa == true){eyiYUrwhRa = false;}
      if(uThnNLHRIZ == true){uThnNLHRIZ = false;}
      if(LJIZpecDSD == true){LJIZpecDSD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PLOZNWAWKN
{ 
  void uajgsPnJXf()
  { 
      bool bBufXmlzHl = false;
      bool QzMmsxFuEc = false;
      bool APFOQrdkhY = false;
      bool FhuxiFfPzL = false;
      bool PXJxXLMlDi = false;
      bool sYxnQMufOf = false;
      bool IBRighEODm = false;
      bool RowsNcFIqE = false;
      bool eqgyegjHgW = false;
      bool KzASCOFaQo = false;
      bool HyiPRDbEam = false;
      bool GlnKARTSEz = false;
      bool xwhgcRocpy = false;
      bool WerBxgIRga = false;
      bool EffsPXKpSk = false;
      bool tmWQrLPBlz = false;
      bool OyObQbtmMT = false;
      bool JClcuRPoRF = false;
      bool GyKFEjIaJk = false;
      bool QGegMFREcm = false;
      string NPglTUwnqI;
      string zoRofIgYey;
      string mfiaMVUZQS;
      string dTAMiUiQIJ;
      string oUIiFfamfW;
      string OWABRhfGYx;
      string AWXYefoEGK;
      string ZQQUIRWAUf;
      string uhtsDGSyuB;
      string obqLISxjDZ;
      string qVfUxyGerz;
      string KekrCScdUX;
      string MpIEVxYEMo;
      string UlxuoYBYwF;
      string AmFupKpyIo;
      string pSOeMAMazz;
      string YBSDeYhoDc;
      string RDgPXZjMOL;
      string GauWyBXRin;
      string LZZwgzUCQR;
      if(NPglTUwnqI == qVfUxyGerz){bBufXmlzHl = true;}
      else if(qVfUxyGerz == NPglTUwnqI){HyiPRDbEam = true;}
      if(zoRofIgYey == KekrCScdUX){QzMmsxFuEc = true;}
      else if(KekrCScdUX == zoRofIgYey){GlnKARTSEz = true;}
      if(mfiaMVUZQS == MpIEVxYEMo){APFOQrdkhY = true;}
      else if(MpIEVxYEMo == mfiaMVUZQS){xwhgcRocpy = true;}
      if(dTAMiUiQIJ == UlxuoYBYwF){FhuxiFfPzL = true;}
      else if(UlxuoYBYwF == dTAMiUiQIJ){WerBxgIRga = true;}
      if(oUIiFfamfW == AmFupKpyIo){PXJxXLMlDi = true;}
      else if(AmFupKpyIo == oUIiFfamfW){EffsPXKpSk = true;}
      if(OWABRhfGYx == pSOeMAMazz){sYxnQMufOf = true;}
      else if(pSOeMAMazz == OWABRhfGYx){tmWQrLPBlz = true;}
      if(AWXYefoEGK == YBSDeYhoDc){IBRighEODm = true;}
      else if(YBSDeYhoDc == AWXYefoEGK){OyObQbtmMT = true;}
      if(ZQQUIRWAUf == RDgPXZjMOL){RowsNcFIqE = true;}
      if(uhtsDGSyuB == GauWyBXRin){eqgyegjHgW = true;}
      if(obqLISxjDZ == LZZwgzUCQR){KzASCOFaQo = true;}
      while(RDgPXZjMOL == ZQQUIRWAUf){JClcuRPoRF = true;}
      while(GauWyBXRin == GauWyBXRin){GyKFEjIaJk = true;}
      while(LZZwgzUCQR == LZZwgzUCQR){QGegMFREcm = true;}
      if(bBufXmlzHl == true){bBufXmlzHl = false;}
      if(QzMmsxFuEc == true){QzMmsxFuEc = false;}
      if(APFOQrdkhY == true){APFOQrdkhY = false;}
      if(FhuxiFfPzL == true){FhuxiFfPzL = false;}
      if(PXJxXLMlDi == true){PXJxXLMlDi = false;}
      if(sYxnQMufOf == true){sYxnQMufOf = false;}
      if(IBRighEODm == true){IBRighEODm = false;}
      if(RowsNcFIqE == true){RowsNcFIqE = false;}
      if(eqgyegjHgW == true){eqgyegjHgW = false;}
      if(KzASCOFaQo == true){KzASCOFaQo = false;}
      if(HyiPRDbEam == true){HyiPRDbEam = false;}
      if(GlnKARTSEz == true){GlnKARTSEz = false;}
      if(xwhgcRocpy == true){xwhgcRocpy = false;}
      if(WerBxgIRga == true){WerBxgIRga = false;}
      if(EffsPXKpSk == true){EffsPXKpSk = false;}
      if(tmWQrLPBlz == true){tmWQrLPBlz = false;}
      if(OyObQbtmMT == true){OyObQbtmMT = false;}
      if(JClcuRPoRF == true){JClcuRPoRF = false;}
      if(GyKFEjIaJk == true){GyKFEjIaJk = false;}
      if(QGegMFREcm == true){QGegMFREcm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SYNCEQFEWX
{ 
  void ldOgomLrlp()
  { 
      bool bUrmbSxWcm = false;
      bool sSuJsubyrh = false;
      bool KxOHbxtaVq = false;
      bool TbEeYYPMOP = false;
      bool bHWIMYHnhj = false;
      bool lzSQsdFkLe = false;
      bool XVAkTROQkT = false;
      bool fPicGDaFtC = false;
      bool pFslFTImlt = false;
      bool TdBjqFGkou = false;
      bool KqDNEWPpJp = false;
      bool DzpIoScOfz = false;
      bool IadpABNzkA = false;
      bool woQzrzCfgi = false;
      bool ESduqTnrtY = false;
      bool crFoljMNNC = false;
      bool IQEJtNhpQo = false;
      bool Pxgxupxhyg = false;
      bool NVEMWDibNI = false;
      bool moSdZnNTBc = false;
      string NHjZEqQyUY;
      string iLSLjjONPQ;
      string wHNNgbRSIi;
      string EjeQtBPxQk;
      string LGuzFmhaVg;
      string ienYZGtVrb;
      string ZypkUyfBRi;
      string URweFpMNIs;
      string tqwaAYuFqn;
      string HtEoLMPJeH;
      string HfImMQiJTi;
      string sgMzhhCPku;
      string CZKdbZNHTr;
      string ZDiBlNhJNi;
      string pllHtZgFHc;
      string KAYiEgVeLl;
      string DboaIpxLMT;
      string ArCBFQmCiB;
      string sIzqhZeAwF;
      string rCyafNxLyE;
      if(NHjZEqQyUY == HfImMQiJTi){bUrmbSxWcm = true;}
      else if(HfImMQiJTi == NHjZEqQyUY){KqDNEWPpJp = true;}
      if(iLSLjjONPQ == sgMzhhCPku){sSuJsubyrh = true;}
      else if(sgMzhhCPku == iLSLjjONPQ){DzpIoScOfz = true;}
      if(wHNNgbRSIi == CZKdbZNHTr){KxOHbxtaVq = true;}
      else if(CZKdbZNHTr == wHNNgbRSIi){IadpABNzkA = true;}
      if(EjeQtBPxQk == ZDiBlNhJNi){TbEeYYPMOP = true;}
      else if(ZDiBlNhJNi == EjeQtBPxQk){woQzrzCfgi = true;}
      if(LGuzFmhaVg == pllHtZgFHc){bHWIMYHnhj = true;}
      else if(pllHtZgFHc == LGuzFmhaVg){ESduqTnrtY = true;}
      if(ienYZGtVrb == KAYiEgVeLl){lzSQsdFkLe = true;}
      else if(KAYiEgVeLl == ienYZGtVrb){crFoljMNNC = true;}
      if(ZypkUyfBRi == DboaIpxLMT){XVAkTROQkT = true;}
      else if(DboaIpxLMT == ZypkUyfBRi){IQEJtNhpQo = true;}
      if(URweFpMNIs == ArCBFQmCiB){fPicGDaFtC = true;}
      if(tqwaAYuFqn == sIzqhZeAwF){pFslFTImlt = true;}
      if(HtEoLMPJeH == rCyafNxLyE){TdBjqFGkou = true;}
      while(ArCBFQmCiB == URweFpMNIs){Pxgxupxhyg = true;}
      while(sIzqhZeAwF == sIzqhZeAwF){NVEMWDibNI = true;}
      while(rCyafNxLyE == rCyafNxLyE){moSdZnNTBc = true;}
      if(bUrmbSxWcm == true){bUrmbSxWcm = false;}
      if(sSuJsubyrh == true){sSuJsubyrh = false;}
      if(KxOHbxtaVq == true){KxOHbxtaVq = false;}
      if(TbEeYYPMOP == true){TbEeYYPMOP = false;}
      if(bHWIMYHnhj == true){bHWIMYHnhj = false;}
      if(lzSQsdFkLe == true){lzSQsdFkLe = false;}
      if(XVAkTROQkT == true){XVAkTROQkT = false;}
      if(fPicGDaFtC == true){fPicGDaFtC = false;}
      if(pFslFTImlt == true){pFslFTImlt = false;}
      if(TdBjqFGkou == true){TdBjqFGkou = false;}
      if(KqDNEWPpJp == true){KqDNEWPpJp = false;}
      if(DzpIoScOfz == true){DzpIoScOfz = false;}
      if(IadpABNzkA == true){IadpABNzkA = false;}
      if(woQzrzCfgi == true){woQzrzCfgi = false;}
      if(ESduqTnrtY == true){ESduqTnrtY = false;}
      if(crFoljMNNC == true){crFoljMNNC = false;}
      if(IQEJtNhpQo == true){IQEJtNhpQo = false;}
      if(Pxgxupxhyg == true){Pxgxupxhyg = false;}
      if(NVEMWDibNI == true){NVEMWDibNI = false;}
      if(moSdZnNTBc == true){moSdZnNTBc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PXFXEFUGGX
{ 
  void AppTJBxxYh()
  { 
      bool pTqwwHedwe = false;
      bool rUeXajeQuk = false;
      bool pGntxGryrP = false;
      bool ITikVUhEUH = false;
      bool jQUwafdiHl = false;
      bool uLoGwiewyp = false;
      bool OglgVXksCG = false;
      bool CawmAYlrSQ = false;
      bool bJpGTxecnl = false;
      bool okFerDenaC = false;
      bool ukgBiUOBFX = false;
      bool xmchNFpfMX = false;
      bool sIFPRQUQux = false;
      bool tyRPVqboWz = false;
      bool hdWQnBHXQl = false;
      bool MyaCJleQiS = false;
      bool zxFIczVNLW = false;
      bool cBJNoPknaP = false;
      bool IxAIjuWOGi = false;
      bool LKFEzjXOmo = false;
      string lWsQbubHed;
      string TLLxXNbixV;
      string fgGieTGfnP;
      string wBjUCGARxe;
      string wLDReVHtbB;
      string wNeGxEQJkH;
      string owSOgSbqtQ;
      string NJkSYUdnDI;
      string MMDmSiWcWy;
      string ppgpoArokM;
      string tPzLnmbLhc;
      string ZbGbucrfux;
      string wyUwaXcudX;
      string boDqOwMryf;
      string HKtsXOiEtM;
      string IMsmPuuDjP;
      string JfhKBkXPNy;
      string PAgOpkDaXM;
      string nYAQPPuQaS;
      string AJHiWabjrm;
      if(lWsQbubHed == tPzLnmbLhc){pTqwwHedwe = true;}
      else if(tPzLnmbLhc == lWsQbubHed){ukgBiUOBFX = true;}
      if(TLLxXNbixV == ZbGbucrfux){rUeXajeQuk = true;}
      else if(ZbGbucrfux == TLLxXNbixV){xmchNFpfMX = true;}
      if(fgGieTGfnP == wyUwaXcudX){pGntxGryrP = true;}
      else if(wyUwaXcudX == fgGieTGfnP){sIFPRQUQux = true;}
      if(wBjUCGARxe == boDqOwMryf){ITikVUhEUH = true;}
      else if(boDqOwMryf == wBjUCGARxe){tyRPVqboWz = true;}
      if(wLDReVHtbB == HKtsXOiEtM){jQUwafdiHl = true;}
      else if(HKtsXOiEtM == wLDReVHtbB){hdWQnBHXQl = true;}
      if(wNeGxEQJkH == IMsmPuuDjP){uLoGwiewyp = true;}
      else if(IMsmPuuDjP == wNeGxEQJkH){MyaCJleQiS = true;}
      if(owSOgSbqtQ == JfhKBkXPNy){OglgVXksCG = true;}
      else if(JfhKBkXPNy == owSOgSbqtQ){zxFIczVNLW = true;}
      if(NJkSYUdnDI == PAgOpkDaXM){CawmAYlrSQ = true;}
      if(MMDmSiWcWy == nYAQPPuQaS){bJpGTxecnl = true;}
      if(ppgpoArokM == AJHiWabjrm){okFerDenaC = true;}
      while(PAgOpkDaXM == NJkSYUdnDI){cBJNoPknaP = true;}
      while(nYAQPPuQaS == nYAQPPuQaS){IxAIjuWOGi = true;}
      while(AJHiWabjrm == AJHiWabjrm){LKFEzjXOmo = true;}
      if(pTqwwHedwe == true){pTqwwHedwe = false;}
      if(rUeXajeQuk == true){rUeXajeQuk = false;}
      if(pGntxGryrP == true){pGntxGryrP = false;}
      if(ITikVUhEUH == true){ITikVUhEUH = false;}
      if(jQUwafdiHl == true){jQUwafdiHl = false;}
      if(uLoGwiewyp == true){uLoGwiewyp = false;}
      if(OglgVXksCG == true){OglgVXksCG = false;}
      if(CawmAYlrSQ == true){CawmAYlrSQ = false;}
      if(bJpGTxecnl == true){bJpGTxecnl = false;}
      if(okFerDenaC == true){okFerDenaC = false;}
      if(ukgBiUOBFX == true){ukgBiUOBFX = false;}
      if(xmchNFpfMX == true){xmchNFpfMX = false;}
      if(sIFPRQUQux == true){sIFPRQUQux = false;}
      if(tyRPVqboWz == true){tyRPVqboWz = false;}
      if(hdWQnBHXQl == true){hdWQnBHXQl = false;}
      if(MyaCJleQiS == true){MyaCJleQiS = false;}
      if(zxFIczVNLW == true){zxFIczVNLW = false;}
      if(cBJNoPknaP == true){cBJNoPknaP = false;}
      if(IxAIjuWOGi == true){IxAIjuWOGi = false;}
      if(LKFEzjXOmo == true){LKFEzjXOmo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FVQZJHNKEQ
{ 
  void klhQjHjQSW()
  { 
      bool gpoAGbansI = false;
      bool HRbXzFtBXW = false;
      bool MPaFxnEwqY = false;
      bool kZoXodnQOU = false;
      bool OiUskyihRn = false;
      bool KQjouROiDR = false;
      bool WexUWgbDhV = false;
      bool RdlDIdugUz = false;
      bool FWdCNUpAbX = false;
      bool OEMxxikzWE = false;
      bool VeAwVgdfgR = false;
      bool WgOELABuGC = false;
      bool pbaJGNQoGY = false;
      bool ruKPcaWhfg = false;
      bool qJTKBgoIkw = false;
      bool hpsTNtDpoI = false;
      bool PnZIYaWyak = false;
      bool dSOYLfIQIY = false;
      bool XIDJpCBpCi = false;
      bool WfrXQCjKkZ = false;
      string cRwzsHGnEE;
      string LNToQLYgzR;
      string yLpqhRkSwO;
      string wjSbpEOlnx;
      string kfbecxmhoD;
      string lkzkKQbFRb;
      string WnqoIbtrzL;
      string wlrBfSiKOk;
      string eCdrbKkhQZ;
      string cJULaxnlWB;
      string CthcGzrxEr;
      string gRpgxTKzNx;
      string pMaZhmQfai;
      string ewmMhjTAkc;
      string SmdYrLSnOG;
      string loYYKpTItT;
      string iMVUMjdcyN;
      string zKNjeChBgW;
      string TcRUwpXOCr;
      string PLFRsFRILI;
      if(cRwzsHGnEE == CthcGzrxEr){gpoAGbansI = true;}
      else if(CthcGzrxEr == cRwzsHGnEE){VeAwVgdfgR = true;}
      if(LNToQLYgzR == gRpgxTKzNx){HRbXzFtBXW = true;}
      else if(gRpgxTKzNx == LNToQLYgzR){WgOELABuGC = true;}
      if(yLpqhRkSwO == pMaZhmQfai){MPaFxnEwqY = true;}
      else if(pMaZhmQfai == yLpqhRkSwO){pbaJGNQoGY = true;}
      if(wjSbpEOlnx == ewmMhjTAkc){kZoXodnQOU = true;}
      else if(ewmMhjTAkc == wjSbpEOlnx){ruKPcaWhfg = true;}
      if(kfbecxmhoD == SmdYrLSnOG){OiUskyihRn = true;}
      else if(SmdYrLSnOG == kfbecxmhoD){qJTKBgoIkw = true;}
      if(lkzkKQbFRb == loYYKpTItT){KQjouROiDR = true;}
      else if(loYYKpTItT == lkzkKQbFRb){hpsTNtDpoI = true;}
      if(WnqoIbtrzL == iMVUMjdcyN){WexUWgbDhV = true;}
      else if(iMVUMjdcyN == WnqoIbtrzL){PnZIYaWyak = true;}
      if(wlrBfSiKOk == zKNjeChBgW){RdlDIdugUz = true;}
      if(eCdrbKkhQZ == TcRUwpXOCr){FWdCNUpAbX = true;}
      if(cJULaxnlWB == PLFRsFRILI){OEMxxikzWE = true;}
      while(zKNjeChBgW == wlrBfSiKOk){dSOYLfIQIY = true;}
      while(TcRUwpXOCr == TcRUwpXOCr){XIDJpCBpCi = true;}
      while(PLFRsFRILI == PLFRsFRILI){WfrXQCjKkZ = true;}
      if(gpoAGbansI == true){gpoAGbansI = false;}
      if(HRbXzFtBXW == true){HRbXzFtBXW = false;}
      if(MPaFxnEwqY == true){MPaFxnEwqY = false;}
      if(kZoXodnQOU == true){kZoXodnQOU = false;}
      if(OiUskyihRn == true){OiUskyihRn = false;}
      if(KQjouROiDR == true){KQjouROiDR = false;}
      if(WexUWgbDhV == true){WexUWgbDhV = false;}
      if(RdlDIdugUz == true){RdlDIdugUz = false;}
      if(FWdCNUpAbX == true){FWdCNUpAbX = false;}
      if(OEMxxikzWE == true){OEMxxikzWE = false;}
      if(VeAwVgdfgR == true){VeAwVgdfgR = false;}
      if(WgOELABuGC == true){WgOELABuGC = false;}
      if(pbaJGNQoGY == true){pbaJGNQoGY = false;}
      if(ruKPcaWhfg == true){ruKPcaWhfg = false;}
      if(qJTKBgoIkw == true){qJTKBgoIkw = false;}
      if(hpsTNtDpoI == true){hpsTNtDpoI = false;}
      if(PnZIYaWyak == true){PnZIYaWyak = false;}
      if(dSOYLfIQIY == true){dSOYLfIQIY = false;}
      if(XIDJpCBpCi == true){XIDJpCBpCi = false;}
      if(WfrXQCjKkZ == true){WfrXQCjKkZ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DFMJKFJQRH
{ 
  void opDzEeeLEQ()
  { 
      bool SYJUwulOJF = false;
      bool wNWQeNzuaD = false;
      bool XEMdsuPEak = false;
      bool XBSAXCcqAY = false;
      bool nTKtnDxaFM = false;
      bool rTYMMOMfaA = false;
      bool ehmYhttWsc = false;
      bool kHuBOEtRop = false;
      bool yoyOrDcGnJ = false;
      bool pwPztGfkMt = false;
      bool JahcbjYWxH = false;
      bool TtqkCpiYfd = false;
      bool RnhjlnquHI = false;
      bool YIFugWGVlI = false;
      bool eFIfouFdxD = false;
      bool XGFhbUrypm = false;
      bool rcPoiLouEe = false;
      bool iMmXzLeCtq = false;
      bool hMyDjSDWSL = false;
      bool dIezPBpTwd = false;
      string CXFWYmfczs;
      string gfdVYYoIkr;
      string WDXVpObihi;
      string HcSZwUXFMS;
      string IHJCqJcLsU;
      string wATDxzzifJ;
      string KxuPlDNEZD;
      string BjwRiPsVeg;
      string kPoMmHJPbx;
      string VnYDILLtPm;
      string wYlpTLEBTh;
      string FdDzXuHice;
      string txcchMLrJX;
      string DzFrjMdzra;
      string CHutqqkYfN;
      string DcQkrywIHH;
      string ALrudGYLPK;
      string QIpBbmBhkp;
      string blMwUahTzk;
      string QUrkaqPAPk;
      if(CXFWYmfczs == wYlpTLEBTh){SYJUwulOJF = true;}
      else if(wYlpTLEBTh == CXFWYmfczs){JahcbjYWxH = true;}
      if(gfdVYYoIkr == FdDzXuHice){wNWQeNzuaD = true;}
      else if(FdDzXuHice == gfdVYYoIkr){TtqkCpiYfd = true;}
      if(WDXVpObihi == txcchMLrJX){XEMdsuPEak = true;}
      else if(txcchMLrJX == WDXVpObihi){RnhjlnquHI = true;}
      if(HcSZwUXFMS == DzFrjMdzra){XBSAXCcqAY = true;}
      else if(DzFrjMdzra == HcSZwUXFMS){YIFugWGVlI = true;}
      if(IHJCqJcLsU == CHutqqkYfN){nTKtnDxaFM = true;}
      else if(CHutqqkYfN == IHJCqJcLsU){eFIfouFdxD = true;}
      if(wATDxzzifJ == DcQkrywIHH){rTYMMOMfaA = true;}
      else if(DcQkrywIHH == wATDxzzifJ){XGFhbUrypm = true;}
      if(KxuPlDNEZD == ALrudGYLPK){ehmYhttWsc = true;}
      else if(ALrudGYLPK == KxuPlDNEZD){rcPoiLouEe = true;}
      if(BjwRiPsVeg == QIpBbmBhkp){kHuBOEtRop = true;}
      if(kPoMmHJPbx == blMwUahTzk){yoyOrDcGnJ = true;}
      if(VnYDILLtPm == QUrkaqPAPk){pwPztGfkMt = true;}
      while(QIpBbmBhkp == BjwRiPsVeg){iMmXzLeCtq = true;}
      while(blMwUahTzk == blMwUahTzk){hMyDjSDWSL = true;}
      while(QUrkaqPAPk == QUrkaqPAPk){dIezPBpTwd = true;}
      if(SYJUwulOJF == true){SYJUwulOJF = false;}
      if(wNWQeNzuaD == true){wNWQeNzuaD = false;}
      if(XEMdsuPEak == true){XEMdsuPEak = false;}
      if(XBSAXCcqAY == true){XBSAXCcqAY = false;}
      if(nTKtnDxaFM == true){nTKtnDxaFM = false;}
      if(rTYMMOMfaA == true){rTYMMOMfaA = false;}
      if(ehmYhttWsc == true){ehmYhttWsc = false;}
      if(kHuBOEtRop == true){kHuBOEtRop = false;}
      if(yoyOrDcGnJ == true){yoyOrDcGnJ = false;}
      if(pwPztGfkMt == true){pwPztGfkMt = false;}
      if(JahcbjYWxH == true){JahcbjYWxH = false;}
      if(TtqkCpiYfd == true){TtqkCpiYfd = false;}
      if(RnhjlnquHI == true){RnhjlnquHI = false;}
      if(YIFugWGVlI == true){YIFugWGVlI = false;}
      if(eFIfouFdxD == true){eFIfouFdxD = false;}
      if(XGFhbUrypm == true){XGFhbUrypm = false;}
      if(rcPoiLouEe == true){rcPoiLouEe = false;}
      if(iMmXzLeCtq == true){iMmXzLeCtq = false;}
      if(hMyDjSDWSL == true){hMyDjSDWSL = false;}
      if(dIezPBpTwd == true){dIezPBpTwd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RDNGIKBGIM
{ 
  void OTEAtarOgT()
  { 
      bool spJeVAnxRm = false;
      bool LYsPcGkpWX = false;
      bool tWtRMiGjhz = false;
      bool BYYUOeTKNz = false;
      bool ulaGgQdEDg = false;
      bool ucKNoWwqSl = false;
      bool tEOSllzXAy = false;
      bool mZcXbIumku = false;
      bool FBfBDXNLfr = false;
      bool hbraEqWIUt = false;
      bool RFEFtUAmkg = false;
      bool bIGWgrXNON = false;
      bool YnrIPKgBSk = false;
      bool klRqdzxZGi = false;
      bool XaZzQYVUtM = false;
      bool SJsHVcMwpi = false;
      bool mzUzuoqOsh = false;
      bool tyGfpxNsqp = false;
      bool oMHFSasKVA = false;
      bool lBiXwYGZDb = false;
      string jrDZdLMQei;
      string bmNzgDdecb;
      string KkIoVpCHsc;
      string mhaWqziryM;
      string CWRfQldUXR;
      string UfCYiyiODn;
      string IEWCutLqIh;
      string TlgPZmSbYj;
      string MgibHHfCmr;
      string PUnJDxzPkW;
      string alMpxizlyu;
      string fGpTbaUuum;
      string OJDPRzyXLb;
      string nDqCJZUsdY;
      string IsDrLIdUgc;
      string bETxHQixYx;
      string jbZWGcPnKC;
      string GbQWeUFKDI;
      string Udpiziistr;
      string pWUgnfTgki;
      if(jrDZdLMQei == alMpxizlyu){spJeVAnxRm = true;}
      else if(alMpxizlyu == jrDZdLMQei){RFEFtUAmkg = true;}
      if(bmNzgDdecb == fGpTbaUuum){LYsPcGkpWX = true;}
      else if(fGpTbaUuum == bmNzgDdecb){bIGWgrXNON = true;}
      if(KkIoVpCHsc == OJDPRzyXLb){tWtRMiGjhz = true;}
      else if(OJDPRzyXLb == KkIoVpCHsc){YnrIPKgBSk = true;}
      if(mhaWqziryM == nDqCJZUsdY){BYYUOeTKNz = true;}
      else if(nDqCJZUsdY == mhaWqziryM){klRqdzxZGi = true;}
      if(CWRfQldUXR == IsDrLIdUgc){ulaGgQdEDg = true;}
      else if(IsDrLIdUgc == CWRfQldUXR){XaZzQYVUtM = true;}
      if(UfCYiyiODn == bETxHQixYx){ucKNoWwqSl = true;}
      else if(bETxHQixYx == UfCYiyiODn){SJsHVcMwpi = true;}
      if(IEWCutLqIh == jbZWGcPnKC){tEOSllzXAy = true;}
      else if(jbZWGcPnKC == IEWCutLqIh){mzUzuoqOsh = true;}
      if(TlgPZmSbYj == GbQWeUFKDI){mZcXbIumku = true;}
      if(MgibHHfCmr == Udpiziistr){FBfBDXNLfr = true;}
      if(PUnJDxzPkW == pWUgnfTgki){hbraEqWIUt = true;}
      while(GbQWeUFKDI == TlgPZmSbYj){tyGfpxNsqp = true;}
      while(Udpiziistr == Udpiziistr){oMHFSasKVA = true;}
      while(pWUgnfTgki == pWUgnfTgki){lBiXwYGZDb = true;}
      if(spJeVAnxRm == true){spJeVAnxRm = false;}
      if(LYsPcGkpWX == true){LYsPcGkpWX = false;}
      if(tWtRMiGjhz == true){tWtRMiGjhz = false;}
      if(BYYUOeTKNz == true){BYYUOeTKNz = false;}
      if(ulaGgQdEDg == true){ulaGgQdEDg = false;}
      if(ucKNoWwqSl == true){ucKNoWwqSl = false;}
      if(tEOSllzXAy == true){tEOSllzXAy = false;}
      if(mZcXbIumku == true){mZcXbIumku = false;}
      if(FBfBDXNLfr == true){FBfBDXNLfr = false;}
      if(hbraEqWIUt == true){hbraEqWIUt = false;}
      if(RFEFtUAmkg == true){RFEFtUAmkg = false;}
      if(bIGWgrXNON == true){bIGWgrXNON = false;}
      if(YnrIPKgBSk == true){YnrIPKgBSk = false;}
      if(klRqdzxZGi == true){klRqdzxZGi = false;}
      if(XaZzQYVUtM == true){XaZzQYVUtM = false;}
      if(SJsHVcMwpi == true){SJsHVcMwpi = false;}
      if(mzUzuoqOsh == true){mzUzuoqOsh = false;}
      if(tyGfpxNsqp == true){tyGfpxNsqp = false;}
      if(oMHFSasKVA == true){oMHFSasKVA = false;}
      if(lBiXwYGZDb == true){lBiXwYGZDb = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CHOBNDKBFE
{ 
  void OBUBxhKOYN()
  { 
      bool MTRqSUrfrA = false;
      bool SeYqRVjJbs = false;
      bool blDcCoxBjI = false;
      bool gUrHCkqGPG = false;
      bool omOcXOcKYQ = false;
      bool OzNFjaQooc = false;
      bool XqwjTymQnf = false;
      bool ectVEoWZFn = false;
      bool LlcTsgHmXb = false;
      bool UBRlhyqdJZ = false;
      bool bLWsklFrBu = false;
      bool DpiARkNcqm = false;
      bool pzsMKcKHmF = false;
      bool xrQfMAPjhG = false;
      bool aPlbXeSRUw = false;
      bool jOmVibSJhH = false;
      bool ayEuxLYDnR = false;
      bool VdtjLZMQof = false;
      bool KsmpViEtwj = false;
      bool gFCZecKqeL = false;
      string OhVIpIbhGC;
      string szISjXbAzj;
      string yghPWaAlje;
      string JPIXLHKKOH;
      string PoSiLWXfDq;
      string oGcSDhAFlO;
      string IoybrdqkHc;
      string MRkrUJNyDz;
      string CUiroCloTc;
      string HEXzyZppph;
      string emXaeXUkxt;
      string TBCmLnGFmp;
      string qCQdUHslst;
      string SWHUTPwHNQ;
      string hBrZJFBkmK;
      string TkDAqcVtFW;
      string dirCawshSw;
      string qyrWHpKqUO;
      string FIyhahTHdp;
      string sCBplzJoyM;
      if(OhVIpIbhGC == emXaeXUkxt){MTRqSUrfrA = true;}
      else if(emXaeXUkxt == OhVIpIbhGC){bLWsklFrBu = true;}
      if(szISjXbAzj == TBCmLnGFmp){SeYqRVjJbs = true;}
      else if(TBCmLnGFmp == szISjXbAzj){DpiARkNcqm = true;}
      if(yghPWaAlje == qCQdUHslst){blDcCoxBjI = true;}
      else if(qCQdUHslst == yghPWaAlje){pzsMKcKHmF = true;}
      if(JPIXLHKKOH == SWHUTPwHNQ){gUrHCkqGPG = true;}
      else if(SWHUTPwHNQ == JPIXLHKKOH){xrQfMAPjhG = true;}
      if(PoSiLWXfDq == hBrZJFBkmK){omOcXOcKYQ = true;}
      else if(hBrZJFBkmK == PoSiLWXfDq){aPlbXeSRUw = true;}
      if(oGcSDhAFlO == TkDAqcVtFW){OzNFjaQooc = true;}
      else if(TkDAqcVtFW == oGcSDhAFlO){jOmVibSJhH = true;}
      if(IoybrdqkHc == dirCawshSw){XqwjTymQnf = true;}
      else if(dirCawshSw == IoybrdqkHc){ayEuxLYDnR = true;}
      if(MRkrUJNyDz == qyrWHpKqUO){ectVEoWZFn = true;}
      if(CUiroCloTc == FIyhahTHdp){LlcTsgHmXb = true;}
      if(HEXzyZppph == sCBplzJoyM){UBRlhyqdJZ = true;}
      while(qyrWHpKqUO == MRkrUJNyDz){VdtjLZMQof = true;}
      while(FIyhahTHdp == FIyhahTHdp){KsmpViEtwj = true;}
      while(sCBplzJoyM == sCBplzJoyM){gFCZecKqeL = true;}
      if(MTRqSUrfrA == true){MTRqSUrfrA = false;}
      if(SeYqRVjJbs == true){SeYqRVjJbs = false;}
      if(blDcCoxBjI == true){blDcCoxBjI = false;}
      if(gUrHCkqGPG == true){gUrHCkqGPG = false;}
      if(omOcXOcKYQ == true){omOcXOcKYQ = false;}
      if(OzNFjaQooc == true){OzNFjaQooc = false;}
      if(XqwjTymQnf == true){XqwjTymQnf = false;}
      if(ectVEoWZFn == true){ectVEoWZFn = false;}
      if(LlcTsgHmXb == true){LlcTsgHmXb = false;}
      if(UBRlhyqdJZ == true){UBRlhyqdJZ = false;}
      if(bLWsklFrBu == true){bLWsklFrBu = false;}
      if(DpiARkNcqm == true){DpiARkNcqm = false;}
      if(pzsMKcKHmF == true){pzsMKcKHmF = false;}
      if(xrQfMAPjhG == true){xrQfMAPjhG = false;}
      if(aPlbXeSRUw == true){aPlbXeSRUw = false;}
      if(jOmVibSJhH == true){jOmVibSJhH = false;}
      if(ayEuxLYDnR == true){ayEuxLYDnR = false;}
      if(VdtjLZMQof == true){VdtjLZMQof = false;}
      if(KsmpViEtwj == true){KsmpViEtwj = false;}
      if(gFCZecKqeL == true){gFCZecKqeL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZQBWNVVGFY
{ 
  void TlCdPEyKss()
  { 
      bool XgwVlztihM = false;
      bool ZNkuQnATgj = false;
      bool GzaeiSOXSn = false;
      bool JwdaMMofMY = false;
      bool VftGTmtEKz = false;
      bool VfuqUXFQlT = false;
      bool kVQtCdiKhE = false;
      bool zTGIOZETKu = false;
      bool XKNTRdAlFn = false;
      bool GXssmAPADp = false;
      bool EjVFzBYmct = false;
      bool SkFquaIwbY = false;
      bool cwEtyaZMUN = false;
      bool JEQUqnoHTQ = false;
      bool FNILUURCyM = false;
      bool rTXeMHeaHY = false;
      bool bMRPjOUkzG = false;
      bool lAcLTOmsYd = false;
      bool IWNHVpglIj = false;
      bool JNAtorusaG = false;
      string ndDwmuhOph;
      string IeNyaDDPLi;
      string PtlieFWtQZ;
      string QKCnVqcGtZ;
      string rbbqwgdQzD;
      string dgofztrURg;
      string XmxQRybYbi;
      string awVkpnRtly;
      string aGuNuiVWoa;
      string yIoMBFdLZy;
      string EFnQbgLDoL;
      string cxhTGQrjUh;
      string bFjAkSHIjk;
      string JFUnsVSekN;
      string XSgcyAZVGj;
      string MsprVyUmeJ;
      string ZCSEBWcsZj;
      string yNzGJJYGCj;
      string skCzXQPcSt;
      string BezanVzbbb;
      if(ndDwmuhOph == EFnQbgLDoL){XgwVlztihM = true;}
      else if(EFnQbgLDoL == ndDwmuhOph){EjVFzBYmct = true;}
      if(IeNyaDDPLi == cxhTGQrjUh){ZNkuQnATgj = true;}
      else if(cxhTGQrjUh == IeNyaDDPLi){SkFquaIwbY = true;}
      if(PtlieFWtQZ == bFjAkSHIjk){GzaeiSOXSn = true;}
      else if(bFjAkSHIjk == PtlieFWtQZ){cwEtyaZMUN = true;}
      if(QKCnVqcGtZ == JFUnsVSekN){JwdaMMofMY = true;}
      else if(JFUnsVSekN == QKCnVqcGtZ){JEQUqnoHTQ = true;}
      if(rbbqwgdQzD == XSgcyAZVGj){VftGTmtEKz = true;}
      else if(XSgcyAZVGj == rbbqwgdQzD){FNILUURCyM = true;}
      if(dgofztrURg == MsprVyUmeJ){VfuqUXFQlT = true;}
      else if(MsprVyUmeJ == dgofztrURg){rTXeMHeaHY = true;}
      if(XmxQRybYbi == ZCSEBWcsZj){kVQtCdiKhE = true;}
      else if(ZCSEBWcsZj == XmxQRybYbi){bMRPjOUkzG = true;}
      if(awVkpnRtly == yNzGJJYGCj){zTGIOZETKu = true;}
      if(aGuNuiVWoa == skCzXQPcSt){XKNTRdAlFn = true;}
      if(yIoMBFdLZy == BezanVzbbb){GXssmAPADp = true;}
      while(yNzGJJYGCj == awVkpnRtly){lAcLTOmsYd = true;}
      while(skCzXQPcSt == skCzXQPcSt){IWNHVpglIj = true;}
      while(BezanVzbbb == BezanVzbbb){JNAtorusaG = true;}
      if(XgwVlztihM == true){XgwVlztihM = false;}
      if(ZNkuQnATgj == true){ZNkuQnATgj = false;}
      if(GzaeiSOXSn == true){GzaeiSOXSn = false;}
      if(JwdaMMofMY == true){JwdaMMofMY = false;}
      if(VftGTmtEKz == true){VftGTmtEKz = false;}
      if(VfuqUXFQlT == true){VfuqUXFQlT = false;}
      if(kVQtCdiKhE == true){kVQtCdiKhE = false;}
      if(zTGIOZETKu == true){zTGIOZETKu = false;}
      if(XKNTRdAlFn == true){XKNTRdAlFn = false;}
      if(GXssmAPADp == true){GXssmAPADp = false;}
      if(EjVFzBYmct == true){EjVFzBYmct = false;}
      if(SkFquaIwbY == true){SkFquaIwbY = false;}
      if(cwEtyaZMUN == true){cwEtyaZMUN = false;}
      if(JEQUqnoHTQ == true){JEQUqnoHTQ = false;}
      if(FNILUURCyM == true){FNILUURCyM = false;}
      if(rTXeMHeaHY == true){rTXeMHeaHY = false;}
      if(bMRPjOUkzG == true){bMRPjOUkzG = false;}
      if(lAcLTOmsYd == true){lAcLTOmsYd = false;}
      if(IWNHVpglIj == true){IWNHVpglIj = false;}
      if(JNAtorusaG == true){JNAtorusaG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WXRAEAQPLC
{ 
  void mGTIeJnJfJ()
  { 
      bool PJUudVfDxy = false;
      bool NAUAjdgEaQ = false;
      bool tTMGBFuFIb = false;
      bool xpPexSXHVV = false;
      bool gWMSgunGwI = false;
      bool gEWhruwOtX = false;
      bool ZauUBWkpCQ = false;
      bool FYiFjWHtYG = false;
      bool QoIzurThlC = false;
      bool VJNxuGaqRJ = false;
      bool JyEeCHVRxV = false;
      bool mConHkRuZI = false;
      bool DDDzuwLkJc = false;
      bool uPbOwRjcwm = false;
      bool WCjQOSUZpK = false;
      bool ofQIGGtEjG = false;
      bool ujIUZXZZqo = false;
      bool BnyllASmkb = false;
      bool akuynPyTIo = false;
      bool WesprltOpq = false;
      string JXndkEwwYj;
      string YMNwtkLYNX;
      string XlXsYJniyU;
      string MyDdXThabS;
      string ItHzzRqiue;
      string XepznXztFa;
      string PMxDWaJiYa;
      string DkxtOcdYtG;
      string SFnTxhHDpA;
      string TmUACwwEbX;
      string IIKTamhukL;
      string saAzHKXdFo;
      string SGsXGPPwqR;
      string fYwVxKuUrK;
      string zMRdGsmVrr;
      string YSymECOlNL;
      string XkDTeTddBx;
      string OClupGnOQM;
      string bsDzrNZJQu;
      string uJHfmNJxzl;
      if(JXndkEwwYj == IIKTamhukL){PJUudVfDxy = true;}
      else if(IIKTamhukL == JXndkEwwYj){JyEeCHVRxV = true;}
      if(YMNwtkLYNX == saAzHKXdFo){NAUAjdgEaQ = true;}
      else if(saAzHKXdFo == YMNwtkLYNX){mConHkRuZI = true;}
      if(XlXsYJniyU == SGsXGPPwqR){tTMGBFuFIb = true;}
      else if(SGsXGPPwqR == XlXsYJniyU){DDDzuwLkJc = true;}
      if(MyDdXThabS == fYwVxKuUrK){xpPexSXHVV = true;}
      else if(fYwVxKuUrK == MyDdXThabS){uPbOwRjcwm = true;}
      if(ItHzzRqiue == zMRdGsmVrr){gWMSgunGwI = true;}
      else if(zMRdGsmVrr == ItHzzRqiue){WCjQOSUZpK = true;}
      if(XepznXztFa == YSymECOlNL){gEWhruwOtX = true;}
      else if(YSymECOlNL == XepznXztFa){ofQIGGtEjG = true;}
      if(PMxDWaJiYa == XkDTeTddBx){ZauUBWkpCQ = true;}
      else if(XkDTeTddBx == PMxDWaJiYa){ujIUZXZZqo = true;}
      if(DkxtOcdYtG == OClupGnOQM){FYiFjWHtYG = true;}
      if(SFnTxhHDpA == bsDzrNZJQu){QoIzurThlC = true;}
      if(TmUACwwEbX == uJHfmNJxzl){VJNxuGaqRJ = true;}
      while(OClupGnOQM == DkxtOcdYtG){BnyllASmkb = true;}
      while(bsDzrNZJQu == bsDzrNZJQu){akuynPyTIo = true;}
      while(uJHfmNJxzl == uJHfmNJxzl){WesprltOpq = true;}
      if(PJUudVfDxy == true){PJUudVfDxy = false;}
      if(NAUAjdgEaQ == true){NAUAjdgEaQ = false;}
      if(tTMGBFuFIb == true){tTMGBFuFIb = false;}
      if(xpPexSXHVV == true){xpPexSXHVV = false;}
      if(gWMSgunGwI == true){gWMSgunGwI = false;}
      if(gEWhruwOtX == true){gEWhruwOtX = false;}
      if(ZauUBWkpCQ == true){ZauUBWkpCQ = false;}
      if(FYiFjWHtYG == true){FYiFjWHtYG = false;}
      if(QoIzurThlC == true){QoIzurThlC = false;}
      if(VJNxuGaqRJ == true){VJNxuGaqRJ = false;}
      if(JyEeCHVRxV == true){JyEeCHVRxV = false;}
      if(mConHkRuZI == true){mConHkRuZI = false;}
      if(DDDzuwLkJc == true){DDDzuwLkJc = false;}
      if(uPbOwRjcwm == true){uPbOwRjcwm = false;}
      if(WCjQOSUZpK == true){WCjQOSUZpK = false;}
      if(ofQIGGtEjG == true){ofQIGGtEjG = false;}
      if(ujIUZXZZqo == true){ujIUZXZZqo = false;}
      if(BnyllASmkb == true){BnyllASmkb = false;}
      if(akuynPyTIo == true){akuynPyTIo = false;}
      if(WesprltOpq == true){WesprltOpq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NQKNCSWNSM
{ 
  void OATrBtRowr()
  { 
      bool qspgYWsLkt = false;
      bool XyRAIgNpSg = false;
      bool ocbpuxCbCD = false;
      bool wAiaXnjrew = false;
      bool bAJKzQOQjw = false;
      bool qgIDoNQAkb = false;
      bool URIgyWPpbU = false;
      bool wUKVEoTQpd = false;
      bool hkqGMOrfkJ = false;
      bool WjGDbKqbMb = false;
      bool BYnPzWVlzA = false;
      bool tAJHsiZCUj = false;
      bool eiERxMNHts = false;
      bool yrEObJwaGo = false;
      bool rGgkPWazKr = false;
      bool koGbxRTVSm = false;
      bool IOqFBCwtei = false;
      bool lzueqYCTCZ = false;
      bool lNwCIJkbRK = false;
      bool pZITgcOaiM = false;
      string PdOyYKEMDF;
      string nPCbhpwupU;
      string QAenDTGcWT;
      string GbMTBaDgqr;
      string SLptPrJmBl;
      string zJCArOyZtE;
      string DbpiyMCiaH;
      string lPEyCFJkXL;
      string JSajTxYMal;
      string cclloqUGuD;
      string oaMtzjEEym;
      string EMCitjAFPB;
      string npHneIcRIK;
      string jDoUViFgjj;
      string rTOmgPaDrE;
      string qOqtOzSYPq;
      string GmumCBXhML;
      string eTVfITbTGP;
      string hZZETVLtFW;
      string PgQscFXake;
      if(PdOyYKEMDF == oaMtzjEEym){qspgYWsLkt = true;}
      else if(oaMtzjEEym == PdOyYKEMDF){BYnPzWVlzA = true;}
      if(nPCbhpwupU == EMCitjAFPB){XyRAIgNpSg = true;}
      else if(EMCitjAFPB == nPCbhpwupU){tAJHsiZCUj = true;}
      if(QAenDTGcWT == npHneIcRIK){ocbpuxCbCD = true;}
      else if(npHneIcRIK == QAenDTGcWT){eiERxMNHts = true;}
      if(GbMTBaDgqr == jDoUViFgjj){wAiaXnjrew = true;}
      else if(jDoUViFgjj == GbMTBaDgqr){yrEObJwaGo = true;}
      if(SLptPrJmBl == rTOmgPaDrE){bAJKzQOQjw = true;}
      else if(rTOmgPaDrE == SLptPrJmBl){rGgkPWazKr = true;}
      if(zJCArOyZtE == qOqtOzSYPq){qgIDoNQAkb = true;}
      else if(qOqtOzSYPq == zJCArOyZtE){koGbxRTVSm = true;}
      if(DbpiyMCiaH == GmumCBXhML){URIgyWPpbU = true;}
      else if(GmumCBXhML == DbpiyMCiaH){IOqFBCwtei = true;}
      if(lPEyCFJkXL == eTVfITbTGP){wUKVEoTQpd = true;}
      if(JSajTxYMal == hZZETVLtFW){hkqGMOrfkJ = true;}
      if(cclloqUGuD == PgQscFXake){WjGDbKqbMb = true;}
      while(eTVfITbTGP == lPEyCFJkXL){lzueqYCTCZ = true;}
      while(hZZETVLtFW == hZZETVLtFW){lNwCIJkbRK = true;}
      while(PgQscFXake == PgQscFXake){pZITgcOaiM = true;}
      if(qspgYWsLkt == true){qspgYWsLkt = false;}
      if(XyRAIgNpSg == true){XyRAIgNpSg = false;}
      if(ocbpuxCbCD == true){ocbpuxCbCD = false;}
      if(wAiaXnjrew == true){wAiaXnjrew = false;}
      if(bAJKzQOQjw == true){bAJKzQOQjw = false;}
      if(qgIDoNQAkb == true){qgIDoNQAkb = false;}
      if(URIgyWPpbU == true){URIgyWPpbU = false;}
      if(wUKVEoTQpd == true){wUKVEoTQpd = false;}
      if(hkqGMOrfkJ == true){hkqGMOrfkJ = false;}
      if(WjGDbKqbMb == true){WjGDbKqbMb = false;}
      if(BYnPzWVlzA == true){BYnPzWVlzA = false;}
      if(tAJHsiZCUj == true){tAJHsiZCUj = false;}
      if(eiERxMNHts == true){eiERxMNHts = false;}
      if(yrEObJwaGo == true){yrEObJwaGo = false;}
      if(rGgkPWazKr == true){rGgkPWazKr = false;}
      if(koGbxRTVSm == true){koGbxRTVSm = false;}
      if(IOqFBCwtei == true){IOqFBCwtei = false;}
      if(lzueqYCTCZ == true){lzueqYCTCZ = false;}
      if(lNwCIJkbRK == true){lNwCIJkbRK = false;}
      if(pZITgcOaiM == true){pZITgcOaiM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IUOHNKPIMV
{ 
  void mlYzqARGfT()
  { 
      bool GyEUZLdHpV = false;
      bool bWOjhjESzS = false;
      bool LZVcRnltRK = false;
      bool GlcxSncVkJ = false;
      bool mzbEuwlwLB = false;
      bool VwIJBgcbCk = false;
      bool YipeOotQRt = false;
      bool GhgpeLyQhV = false;
      bool CoiPoKcSwl = false;
      bool LBTzjEJwoF = false;
      bool BUeRtnPSdo = false;
      bool hLsfijAZpE = false;
      bool MbUeiFNgGa = false;
      bool CBbbUgKRUA = false;
      bool tKoGDzHmQA = false;
      bool EjGjsZAzio = false;
      bool ZhMClgiRLB = false;
      bool YiEoKClNai = false;
      bool xZQFWSuqhu = false;
      bool gKifaMylUW = false;
      string fpooLOwdSK;
      string YbFnCtZypL;
      string tNdxFlKNKD;
      string pKFGhwCjNj;
      string XoeBWQuVnt;
      string tcMIYRMqjY;
      string EiTOTOVzGG;
      string rokGnEhbao;
      string qoYQINyxgP;
      string iKAnenVQhH;
      string tuOhBFrFTT;
      string jkPuhPJDFG;
      string PXSpXgnxaz;
      string iRsStwZqOD;
      string UsdMfytFGP;
      string dRKbrWcYJU;
      string RudUjSqwAk;
      string oSnCkVtFKZ;
      string lhqLUQGRaX;
      string TAgtkZOIcM;
      if(fpooLOwdSK == tuOhBFrFTT){GyEUZLdHpV = true;}
      else if(tuOhBFrFTT == fpooLOwdSK){BUeRtnPSdo = true;}
      if(YbFnCtZypL == jkPuhPJDFG){bWOjhjESzS = true;}
      else if(jkPuhPJDFG == YbFnCtZypL){hLsfijAZpE = true;}
      if(tNdxFlKNKD == PXSpXgnxaz){LZVcRnltRK = true;}
      else if(PXSpXgnxaz == tNdxFlKNKD){MbUeiFNgGa = true;}
      if(pKFGhwCjNj == iRsStwZqOD){GlcxSncVkJ = true;}
      else if(iRsStwZqOD == pKFGhwCjNj){CBbbUgKRUA = true;}
      if(XoeBWQuVnt == UsdMfytFGP){mzbEuwlwLB = true;}
      else if(UsdMfytFGP == XoeBWQuVnt){tKoGDzHmQA = true;}
      if(tcMIYRMqjY == dRKbrWcYJU){VwIJBgcbCk = true;}
      else if(dRKbrWcYJU == tcMIYRMqjY){EjGjsZAzio = true;}
      if(EiTOTOVzGG == RudUjSqwAk){YipeOotQRt = true;}
      else if(RudUjSqwAk == EiTOTOVzGG){ZhMClgiRLB = true;}
      if(rokGnEhbao == oSnCkVtFKZ){GhgpeLyQhV = true;}
      if(qoYQINyxgP == lhqLUQGRaX){CoiPoKcSwl = true;}
      if(iKAnenVQhH == TAgtkZOIcM){LBTzjEJwoF = true;}
      while(oSnCkVtFKZ == rokGnEhbao){YiEoKClNai = true;}
      while(lhqLUQGRaX == lhqLUQGRaX){xZQFWSuqhu = true;}
      while(TAgtkZOIcM == TAgtkZOIcM){gKifaMylUW = true;}
      if(GyEUZLdHpV == true){GyEUZLdHpV = false;}
      if(bWOjhjESzS == true){bWOjhjESzS = false;}
      if(LZVcRnltRK == true){LZVcRnltRK = false;}
      if(GlcxSncVkJ == true){GlcxSncVkJ = false;}
      if(mzbEuwlwLB == true){mzbEuwlwLB = false;}
      if(VwIJBgcbCk == true){VwIJBgcbCk = false;}
      if(YipeOotQRt == true){YipeOotQRt = false;}
      if(GhgpeLyQhV == true){GhgpeLyQhV = false;}
      if(CoiPoKcSwl == true){CoiPoKcSwl = false;}
      if(LBTzjEJwoF == true){LBTzjEJwoF = false;}
      if(BUeRtnPSdo == true){BUeRtnPSdo = false;}
      if(hLsfijAZpE == true){hLsfijAZpE = false;}
      if(MbUeiFNgGa == true){MbUeiFNgGa = false;}
      if(CBbbUgKRUA == true){CBbbUgKRUA = false;}
      if(tKoGDzHmQA == true){tKoGDzHmQA = false;}
      if(EjGjsZAzio == true){EjGjsZAzio = false;}
      if(ZhMClgiRLB == true){ZhMClgiRLB = false;}
      if(YiEoKClNai == true){YiEoKClNai = false;}
      if(xZQFWSuqhu == true){xZQFWSuqhu = false;}
      if(gKifaMylUW == true){gKifaMylUW = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XIBZPFBAKF
{ 
  void wHALIUakhW()
  { 
      bool UiaupVWXuA = false;
      bool GsHAZPBVmh = false;
      bool ytoBYRHhjp = false;
      bool SSlrlhplzy = false;
      bool qAraKzLcAP = false;
      bool PigIuZtZWY = false;
      bool OomFgmHPNq = false;
      bool YOWnqxCNpY = false;
      bool GdgTZikdwL = false;
      bool nQPZjCNmsW = false;
      bool ruPJqBMsRx = false;
      bool XfixSjHTex = false;
      bool CdSBCHzwDo = false;
      bool jOTTgKOXcY = false;
      bool WrLynHgSHA = false;
      bool ttIwUObNGi = false;
      bool BPocNJMPPI = false;
      bool VjpGZJRUNt = false;
      bool agoDtpGXyS = false;
      bool hAQsWHCikd = false;
      string fhCraRjMQu;
      string APMKwiAeWD;
      string xXETwmJKOh;
      string FOsQIHBRkg;
      string AAxBCeizHp;
      string bZTjmIdium;
      string HYxholWuCo;
      string hitpCwZDRR;
      string EJDYFfuIdz;
      string dRWxYbDOLP;
      string HnxSHsechp;
      string hZRcixVYbI;
      string pNQDKQLwQa;
      string sLqBKQMkQM;
      string ARanQDztOa;
      string GemDYsWsHq;
      string giLGxWiQSS;
      string kjHPHYmclH;
      string QRhoTOXnbQ;
      string sScZeRFTqt;
      if(fhCraRjMQu == HnxSHsechp){UiaupVWXuA = true;}
      else if(HnxSHsechp == fhCraRjMQu){ruPJqBMsRx = true;}
      if(APMKwiAeWD == hZRcixVYbI){GsHAZPBVmh = true;}
      else if(hZRcixVYbI == APMKwiAeWD){XfixSjHTex = true;}
      if(xXETwmJKOh == pNQDKQLwQa){ytoBYRHhjp = true;}
      else if(pNQDKQLwQa == xXETwmJKOh){CdSBCHzwDo = true;}
      if(FOsQIHBRkg == sLqBKQMkQM){SSlrlhplzy = true;}
      else if(sLqBKQMkQM == FOsQIHBRkg){jOTTgKOXcY = true;}
      if(AAxBCeizHp == ARanQDztOa){qAraKzLcAP = true;}
      else if(ARanQDztOa == AAxBCeizHp){WrLynHgSHA = true;}
      if(bZTjmIdium == GemDYsWsHq){PigIuZtZWY = true;}
      else if(GemDYsWsHq == bZTjmIdium){ttIwUObNGi = true;}
      if(HYxholWuCo == giLGxWiQSS){OomFgmHPNq = true;}
      else if(giLGxWiQSS == HYxholWuCo){BPocNJMPPI = true;}
      if(hitpCwZDRR == kjHPHYmclH){YOWnqxCNpY = true;}
      if(EJDYFfuIdz == QRhoTOXnbQ){GdgTZikdwL = true;}
      if(dRWxYbDOLP == sScZeRFTqt){nQPZjCNmsW = true;}
      while(kjHPHYmclH == hitpCwZDRR){VjpGZJRUNt = true;}
      while(QRhoTOXnbQ == QRhoTOXnbQ){agoDtpGXyS = true;}
      while(sScZeRFTqt == sScZeRFTqt){hAQsWHCikd = true;}
      if(UiaupVWXuA == true){UiaupVWXuA = false;}
      if(GsHAZPBVmh == true){GsHAZPBVmh = false;}
      if(ytoBYRHhjp == true){ytoBYRHhjp = false;}
      if(SSlrlhplzy == true){SSlrlhplzy = false;}
      if(qAraKzLcAP == true){qAraKzLcAP = false;}
      if(PigIuZtZWY == true){PigIuZtZWY = false;}
      if(OomFgmHPNq == true){OomFgmHPNq = false;}
      if(YOWnqxCNpY == true){YOWnqxCNpY = false;}
      if(GdgTZikdwL == true){GdgTZikdwL = false;}
      if(nQPZjCNmsW == true){nQPZjCNmsW = false;}
      if(ruPJqBMsRx == true){ruPJqBMsRx = false;}
      if(XfixSjHTex == true){XfixSjHTex = false;}
      if(CdSBCHzwDo == true){CdSBCHzwDo = false;}
      if(jOTTgKOXcY == true){jOTTgKOXcY = false;}
      if(WrLynHgSHA == true){WrLynHgSHA = false;}
      if(ttIwUObNGi == true){ttIwUObNGi = false;}
      if(BPocNJMPPI == true){BPocNJMPPI = false;}
      if(VjpGZJRUNt == true){VjpGZJRUNt = false;}
      if(agoDtpGXyS == true){agoDtpGXyS = false;}
      if(hAQsWHCikd == true){hAQsWHCikd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QWUKKVLKUV
{ 
  void TQOVhNDlKI()
  { 
      bool VzpyiAmDIO = false;
      bool AEIzgdJWgZ = false;
      bool SpGkzzdaPf = false;
      bool zGyRCIFIkL = false;
      bool exjgmHAPOI = false;
      bool tunpUesEyj = false;
      bool VLmbPtFpxX = false;
      bool YBlVtVGYyr = false;
      bool zOMHlDSIwW = false;
      bool PAJBoSKHRs = false;
      bool BJSpjrJRuz = false;
      bool YUGFkHzepL = false;
      bool axtaHXqkfb = false;
      bool KyGFqpwVXs = false;
      bool sANwxusDPg = false;
      bool TeRGRDOriM = false;
      bool BYkToZNBOu = false;
      bool LcHxRajycM = false;
      bool dmUzIfgAmm = false;
      bool aDmKrUOtHX = false;
      string TiCaWtGYVP;
      string rDicUotldg;
      string CQjlDIRlfE;
      string HAzljUxcQx;
      string xJmDAVmUzs;
      string EDkjLDyPWN;
      string bUsQHqEwsz;
      string rRUgjrcfkh;
      string VxmlADOOag;
      string WgFCUbQWwI;
      string RpazBmiEAS;
      string ZSeordgHQE;
      string coVMbLLRLH;
      string bGjbPfWmkN;
      string UKNHGECPKq;
      string uJGqfbTNXp;
      string aQrsqShCYL;
      string HtVDiRaOFX;
      string IwHUMRArwn;
      string MIPumDSUPn;
      if(TiCaWtGYVP == RpazBmiEAS){VzpyiAmDIO = true;}
      else if(RpazBmiEAS == TiCaWtGYVP){BJSpjrJRuz = true;}
      if(rDicUotldg == ZSeordgHQE){AEIzgdJWgZ = true;}
      else if(ZSeordgHQE == rDicUotldg){YUGFkHzepL = true;}
      if(CQjlDIRlfE == coVMbLLRLH){SpGkzzdaPf = true;}
      else if(coVMbLLRLH == CQjlDIRlfE){axtaHXqkfb = true;}
      if(HAzljUxcQx == bGjbPfWmkN){zGyRCIFIkL = true;}
      else if(bGjbPfWmkN == HAzljUxcQx){KyGFqpwVXs = true;}
      if(xJmDAVmUzs == UKNHGECPKq){exjgmHAPOI = true;}
      else if(UKNHGECPKq == xJmDAVmUzs){sANwxusDPg = true;}
      if(EDkjLDyPWN == uJGqfbTNXp){tunpUesEyj = true;}
      else if(uJGqfbTNXp == EDkjLDyPWN){TeRGRDOriM = true;}
      if(bUsQHqEwsz == aQrsqShCYL){VLmbPtFpxX = true;}
      else if(aQrsqShCYL == bUsQHqEwsz){BYkToZNBOu = true;}
      if(rRUgjrcfkh == HtVDiRaOFX){YBlVtVGYyr = true;}
      if(VxmlADOOag == IwHUMRArwn){zOMHlDSIwW = true;}
      if(WgFCUbQWwI == MIPumDSUPn){PAJBoSKHRs = true;}
      while(HtVDiRaOFX == rRUgjrcfkh){LcHxRajycM = true;}
      while(IwHUMRArwn == IwHUMRArwn){dmUzIfgAmm = true;}
      while(MIPumDSUPn == MIPumDSUPn){aDmKrUOtHX = true;}
      if(VzpyiAmDIO == true){VzpyiAmDIO = false;}
      if(AEIzgdJWgZ == true){AEIzgdJWgZ = false;}
      if(SpGkzzdaPf == true){SpGkzzdaPf = false;}
      if(zGyRCIFIkL == true){zGyRCIFIkL = false;}
      if(exjgmHAPOI == true){exjgmHAPOI = false;}
      if(tunpUesEyj == true){tunpUesEyj = false;}
      if(VLmbPtFpxX == true){VLmbPtFpxX = false;}
      if(YBlVtVGYyr == true){YBlVtVGYyr = false;}
      if(zOMHlDSIwW == true){zOMHlDSIwW = false;}
      if(PAJBoSKHRs == true){PAJBoSKHRs = false;}
      if(BJSpjrJRuz == true){BJSpjrJRuz = false;}
      if(YUGFkHzepL == true){YUGFkHzepL = false;}
      if(axtaHXqkfb == true){axtaHXqkfb = false;}
      if(KyGFqpwVXs == true){KyGFqpwVXs = false;}
      if(sANwxusDPg == true){sANwxusDPg = false;}
      if(TeRGRDOriM == true){TeRGRDOriM = false;}
      if(BYkToZNBOu == true){BYkToZNBOu = false;}
      if(LcHxRajycM == true){LcHxRajycM = false;}
      if(dmUzIfgAmm == true){dmUzIfgAmm = false;}
      if(aDmKrUOtHX == true){aDmKrUOtHX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JEGHREOLTE
{ 
  void jFgxeWkfiI()
  { 
      bool aTCZDJTlAT = false;
      bool LzRKLrXJDx = false;
      bool EKbOTSrUpW = false;
      bool xXsgsfGhBE = false;
      bool EsOwQQERjW = false;
      bool QlDKZPqmGk = false;
      bool QzucVmFjQf = false;
      bool koIyrLuexD = false;
      bool NajTGKUiHc = false;
      bool nTaPmFxlad = false;
      bool KTAEkadERO = false;
      bool UMoDWpObWK = false;
      bool SVcnqRNrKm = false;
      bool NJETrtafhe = false;
      bool gQQrIbdXrx = false;
      bool qVOPKopMMS = false;
      bool MkseAKHULc = false;
      bool OhsqcCFjeP = false;
      bool npANwrunIC = false;
      bool DOSPSwWlqx = false;
      string XdIVHtNsbn;
      string tZmCzttfGj;
      string YjHqsLBfrk;
      string pQoBHZAiEa;
      string lICoueQyZP;
      string QYsHtnVRbu;
      string rgApxckktZ;
      string sLafxQhSuO;
      string uDGcURXhGT;
      string DhHJhEHLqR;
      string teDAVExMJd;
      string FCVMzoaWUe;
      string EDdZVLDRyr;
      string IVbdUQzGGP;
      string TKXcOBLtJs;
      string djqLNqAddK;
      string BfORUjarri;
      string oUQELseBLT;
      string zIHFNXePQc;
      string PVbuOnwGHe;
      if(XdIVHtNsbn == teDAVExMJd){aTCZDJTlAT = true;}
      else if(teDAVExMJd == XdIVHtNsbn){KTAEkadERO = true;}
      if(tZmCzttfGj == FCVMzoaWUe){LzRKLrXJDx = true;}
      else if(FCVMzoaWUe == tZmCzttfGj){UMoDWpObWK = true;}
      if(YjHqsLBfrk == EDdZVLDRyr){EKbOTSrUpW = true;}
      else if(EDdZVLDRyr == YjHqsLBfrk){SVcnqRNrKm = true;}
      if(pQoBHZAiEa == IVbdUQzGGP){xXsgsfGhBE = true;}
      else if(IVbdUQzGGP == pQoBHZAiEa){NJETrtafhe = true;}
      if(lICoueQyZP == TKXcOBLtJs){EsOwQQERjW = true;}
      else if(TKXcOBLtJs == lICoueQyZP){gQQrIbdXrx = true;}
      if(QYsHtnVRbu == djqLNqAddK){QlDKZPqmGk = true;}
      else if(djqLNqAddK == QYsHtnVRbu){qVOPKopMMS = true;}
      if(rgApxckktZ == BfORUjarri){QzucVmFjQf = true;}
      else if(BfORUjarri == rgApxckktZ){MkseAKHULc = true;}
      if(sLafxQhSuO == oUQELseBLT){koIyrLuexD = true;}
      if(uDGcURXhGT == zIHFNXePQc){NajTGKUiHc = true;}
      if(DhHJhEHLqR == PVbuOnwGHe){nTaPmFxlad = true;}
      while(oUQELseBLT == sLafxQhSuO){OhsqcCFjeP = true;}
      while(zIHFNXePQc == zIHFNXePQc){npANwrunIC = true;}
      while(PVbuOnwGHe == PVbuOnwGHe){DOSPSwWlqx = true;}
      if(aTCZDJTlAT == true){aTCZDJTlAT = false;}
      if(LzRKLrXJDx == true){LzRKLrXJDx = false;}
      if(EKbOTSrUpW == true){EKbOTSrUpW = false;}
      if(xXsgsfGhBE == true){xXsgsfGhBE = false;}
      if(EsOwQQERjW == true){EsOwQQERjW = false;}
      if(QlDKZPqmGk == true){QlDKZPqmGk = false;}
      if(QzucVmFjQf == true){QzucVmFjQf = false;}
      if(koIyrLuexD == true){koIyrLuexD = false;}
      if(NajTGKUiHc == true){NajTGKUiHc = false;}
      if(nTaPmFxlad == true){nTaPmFxlad = false;}
      if(KTAEkadERO == true){KTAEkadERO = false;}
      if(UMoDWpObWK == true){UMoDWpObWK = false;}
      if(SVcnqRNrKm == true){SVcnqRNrKm = false;}
      if(NJETrtafhe == true){NJETrtafhe = false;}
      if(gQQrIbdXrx == true){gQQrIbdXrx = false;}
      if(qVOPKopMMS == true){qVOPKopMMS = false;}
      if(MkseAKHULc == true){MkseAKHULc = false;}
      if(OhsqcCFjeP == true){OhsqcCFjeP = false;}
      if(npANwrunIC == true){npANwrunIC = false;}
      if(DOSPSwWlqx == true){DOSPSwWlqx = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BBMEMBQHKE
{ 
  void BuVSMlZRJg()
  { 
      bool ofzwxiHkus = false;
      bool sTBaSEucMh = false;
      bool IjwLhJcAon = false;
      bool HWTkDSxeOP = false;
      bool jMGxVPiUDE = false;
      bool mAbdkEjnOn = false;
      bool FGwHlWAQXA = false;
      bool GQNxywczoR = false;
      bool aHwkAGtozy = false;
      bool hJPaJcQzKz = false;
      bool ORxDxVlefn = false;
      bool bjywMxJLGe = false;
      bool bLQYRxBmmU = false;
      bool fdnoCIKsme = false;
      bool TOJLWyKSNM = false;
      bool SFrfWDBXlz = false;
      bool UzrKIxuIsE = false;
      bool mQXbKNCFBS = false;
      bool lhYZBalISz = false;
      bool HlHnKUVyCZ = false;
      string FbPRettowa;
      string fVtKLmDXBB;
      string kzECiZGxFF;
      string gPqqsHpGTr;
      string uYenywWcrg;
      string WPCYHGXMiE;
      string nwipsQeCRD;
      string wwDIGLUPDl;
      string DcFYiRMSil;
      string icXODzooth;
      string RTLnkZLtRw;
      string lHJwBLpKGb;
      string DMTeUSYoEG;
      string VeUVUjQQnJ;
      string FSAmAbTaNr;
      string IdhSdFAfTR;
      string BWoKZGmmbQ;
      string tZWxWkLfer;
      string yxaMyqxCqP;
      string ikwVIicoMt;
      if(FbPRettowa == RTLnkZLtRw){ofzwxiHkus = true;}
      else if(RTLnkZLtRw == FbPRettowa){ORxDxVlefn = true;}
      if(fVtKLmDXBB == lHJwBLpKGb){sTBaSEucMh = true;}
      else if(lHJwBLpKGb == fVtKLmDXBB){bjywMxJLGe = true;}
      if(kzECiZGxFF == DMTeUSYoEG){IjwLhJcAon = true;}
      else if(DMTeUSYoEG == kzECiZGxFF){bLQYRxBmmU = true;}
      if(gPqqsHpGTr == VeUVUjQQnJ){HWTkDSxeOP = true;}
      else if(VeUVUjQQnJ == gPqqsHpGTr){fdnoCIKsme = true;}
      if(uYenywWcrg == FSAmAbTaNr){jMGxVPiUDE = true;}
      else if(FSAmAbTaNr == uYenywWcrg){TOJLWyKSNM = true;}
      if(WPCYHGXMiE == IdhSdFAfTR){mAbdkEjnOn = true;}
      else if(IdhSdFAfTR == WPCYHGXMiE){SFrfWDBXlz = true;}
      if(nwipsQeCRD == BWoKZGmmbQ){FGwHlWAQXA = true;}
      else if(BWoKZGmmbQ == nwipsQeCRD){UzrKIxuIsE = true;}
      if(wwDIGLUPDl == tZWxWkLfer){GQNxywczoR = true;}
      if(DcFYiRMSil == yxaMyqxCqP){aHwkAGtozy = true;}
      if(icXODzooth == ikwVIicoMt){hJPaJcQzKz = true;}
      while(tZWxWkLfer == wwDIGLUPDl){mQXbKNCFBS = true;}
      while(yxaMyqxCqP == yxaMyqxCqP){lhYZBalISz = true;}
      while(ikwVIicoMt == ikwVIicoMt){HlHnKUVyCZ = true;}
      if(ofzwxiHkus == true){ofzwxiHkus = false;}
      if(sTBaSEucMh == true){sTBaSEucMh = false;}
      if(IjwLhJcAon == true){IjwLhJcAon = false;}
      if(HWTkDSxeOP == true){HWTkDSxeOP = false;}
      if(jMGxVPiUDE == true){jMGxVPiUDE = false;}
      if(mAbdkEjnOn == true){mAbdkEjnOn = false;}
      if(FGwHlWAQXA == true){FGwHlWAQXA = false;}
      if(GQNxywczoR == true){GQNxywczoR = false;}
      if(aHwkAGtozy == true){aHwkAGtozy = false;}
      if(hJPaJcQzKz == true){hJPaJcQzKz = false;}
      if(ORxDxVlefn == true){ORxDxVlefn = false;}
      if(bjywMxJLGe == true){bjywMxJLGe = false;}
      if(bLQYRxBmmU == true){bLQYRxBmmU = false;}
      if(fdnoCIKsme == true){fdnoCIKsme = false;}
      if(TOJLWyKSNM == true){TOJLWyKSNM = false;}
      if(SFrfWDBXlz == true){SFrfWDBXlz = false;}
      if(UzrKIxuIsE == true){UzrKIxuIsE = false;}
      if(mQXbKNCFBS == true){mQXbKNCFBS = false;}
      if(lhYZBalISz == true){lhYZBalISz = false;}
      if(HlHnKUVyCZ == true){HlHnKUVyCZ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LLLTDWMJCI
{ 
  void knIQKAUtUs()
  { 
      bool LXctAGLbjD = false;
      bool bDkqaxZRVj = false;
      bool uqPdCsXagC = false;
      bool ZXQdxiHKdV = false;
      bool jPWcGonHnj = false;
      bool TZAWYUIyqH = false;
      bool hkuhdDmGsr = false;
      bool pPnAAeUmWN = false;
      bool EQlwhAXsEb = false;
      bool ZISVmyTnyD = false;
      bool BhNutEkzWt = false;
      bool AVbdCRyzzd = false;
      bool esBQRhwRNB = false;
      bool KfnSLWCpCd = false;
      bool OXWKbEfoTV = false;
      bool AUoUXIfxcL = false;
      bool HTlUiUgIRf = false;
      bool OHwVPHWqoM = false;
      bool BHyqAoqStD = false;
      bool qFMpVtkBaH = false;
      string HiTkxrpQSl;
      string DcPmnsCeHs;
      string hfHqYYTBRO;
      string VQkmEXwTeQ;
      string nqaRNfQWiH;
      string pYjKUbbcLY;
      string XnzDQlqchP;
      string whApFqFocs;
      string IxRJYwgaRc;
      string wTHGszrWBa;
      string nmnTKIYsEd;
      string VlzSQaVoeT;
      string FQLFTbZtxG;
      string yinCFUIyyQ;
      string XnVmGJUopS;
      string dCqgVWbiAi;
      string hdwyfmywMu;
      string RWqOwRuXyJ;
      string HjEdburHgi;
      string rYyPhsqWHT;
      if(HiTkxrpQSl == nmnTKIYsEd){LXctAGLbjD = true;}
      else if(nmnTKIYsEd == HiTkxrpQSl){BhNutEkzWt = true;}
      if(DcPmnsCeHs == VlzSQaVoeT){bDkqaxZRVj = true;}
      else if(VlzSQaVoeT == DcPmnsCeHs){AVbdCRyzzd = true;}
      if(hfHqYYTBRO == FQLFTbZtxG){uqPdCsXagC = true;}
      else if(FQLFTbZtxG == hfHqYYTBRO){esBQRhwRNB = true;}
      if(VQkmEXwTeQ == yinCFUIyyQ){ZXQdxiHKdV = true;}
      else if(yinCFUIyyQ == VQkmEXwTeQ){KfnSLWCpCd = true;}
      if(nqaRNfQWiH == XnVmGJUopS){jPWcGonHnj = true;}
      else if(XnVmGJUopS == nqaRNfQWiH){OXWKbEfoTV = true;}
      if(pYjKUbbcLY == dCqgVWbiAi){TZAWYUIyqH = true;}
      else if(dCqgVWbiAi == pYjKUbbcLY){AUoUXIfxcL = true;}
      if(XnzDQlqchP == hdwyfmywMu){hkuhdDmGsr = true;}
      else if(hdwyfmywMu == XnzDQlqchP){HTlUiUgIRf = true;}
      if(whApFqFocs == RWqOwRuXyJ){pPnAAeUmWN = true;}
      if(IxRJYwgaRc == HjEdburHgi){EQlwhAXsEb = true;}
      if(wTHGszrWBa == rYyPhsqWHT){ZISVmyTnyD = true;}
      while(RWqOwRuXyJ == whApFqFocs){OHwVPHWqoM = true;}
      while(HjEdburHgi == HjEdburHgi){BHyqAoqStD = true;}
      while(rYyPhsqWHT == rYyPhsqWHT){qFMpVtkBaH = true;}
      if(LXctAGLbjD == true){LXctAGLbjD = false;}
      if(bDkqaxZRVj == true){bDkqaxZRVj = false;}
      if(uqPdCsXagC == true){uqPdCsXagC = false;}
      if(ZXQdxiHKdV == true){ZXQdxiHKdV = false;}
      if(jPWcGonHnj == true){jPWcGonHnj = false;}
      if(TZAWYUIyqH == true){TZAWYUIyqH = false;}
      if(hkuhdDmGsr == true){hkuhdDmGsr = false;}
      if(pPnAAeUmWN == true){pPnAAeUmWN = false;}
      if(EQlwhAXsEb == true){EQlwhAXsEb = false;}
      if(ZISVmyTnyD == true){ZISVmyTnyD = false;}
      if(BhNutEkzWt == true){BhNutEkzWt = false;}
      if(AVbdCRyzzd == true){AVbdCRyzzd = false;}
      if(esBQRhwRNB == true){esBQRhwRNB = false;}
      if(KfnSLWCpCd == true){KfnSLWCpCd = false;}
      if(OXWKbEfoTV == true){OXWKbEfoTV = false;}
      if(AUoUXIfxcL == true){AUoUXIfxcL = false;}
      if(HTlUiUgIRf == true){HTlUiUgIRf = false;}
      if(OHwVPHWqoM == true){OHwVPHWqoM = false;}
      if(BHyqAoqStD == true){BHyqAoqStD = false;}
      if(qFMpVtkBaH == true){qFMpVtkBaH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HNJSQDOATE
{ 
  void hdWXcQATSX()
  { 
      bool BXJhwcRDZX = false;
      bool nbXQTNTRVW = false;
      bool RXTyQDwtPM = false;
      bool gytUOIxSPk = false;
      bool cZrtFudMZk = false;
      bool KIOkycELQr = false;
      bool gWnNpphCBL = false;
      bool QBCksqfYrV = false;
      bool ZiUdCYiIVl = false;
      bool DkCWDFldDP = false;
      bool NaLOUJGdqG = false;
      bool weSuSxCJLR = false;
      bool GYfOHezehn = false;
      bool FCVbHyXGJr = false;
      bool RFROoYRGEG = false;
      bool DbSlIouwGo = false;
      bool JauDBtJpnT = false;
      bool YuzSzBGRZi = false;
      bool MraajcijSS = false;
      bool DbzTiqJONt = false;
      string WLcLWIBiXz;
      string htWARIrjeT;
      string EeozCcaSNt;
      string PVTDxpXWDz;
      string TLftGEXJSb;
      string yXYzKrVcIs;
      string giVdiLaSDb;
      string kVffzryusx;
      string FRAluqxUrF;
      string TtNKRjpBKt;
      string dEGpqbqrzS;
      string sajLnPiihr;
      string NFaxIpIVNP;
      string BxjySAKJtS;
      string AFVYMrJBVB;
      string BGSSfYcIgA;
      string TUdjRuplrn;
      string FYoOrIjBkQ;
      string fGALNtdRRt;
      string WBctzeakHQ;
      if(WLcLWIBiXz == dEGpqbqrzS){BXJhwcRDZX = true;}
      else if(dEGpqbqrzS == WLcLWIBiXz){NaLOUJGdqG = true;}
      if(htWARIrjeT == sajLnPiihr){nbXQTNTRVW = true;}
      else if(sajLnPiihr == htWARIrjeT){weSuSxCJLR = true;}
      if(EeozCcaSNt == NFaxIpIVNP){RXTyQDwtPM = true;}
      else if(NFaxIpIVNP == EeozCcaSNt){GYfOHezehn = true;}
      if(PVTDxpXWDz == BxjySAKJtS){gytUOIxSPk = true;}
      else if(BxjySAKJtS == PVTDxpXWDz){FCVbHyXGJr = true;}
      if(TLftGEXJSb == AFVYMrJBVB){cZrtFudMZk = true;}
      else if(AFVYMrJBVB == TLftGEXJSb){RFROoYRGEG = true;}
      if(yXYzKrVcIs == BGSSfYcIgA){KIOkycELQr = true;}
      else if(BGSSfYcIgA == yXYzKrVcIs){DbSlIouwGo = true;}
      if(giVdiLaSDb == TUdjRuplrn){gWnNpphCBL = true;}
      else if(TUdjRuplrn == giVdiLaSDb){JauDBtJpnT = true;}
      if(kVffzryusx == FYoOrIjBkQ){QBCksqfYrV = true;}
      if(FRAluqxUrF == fGALNtdRRt){ZiUdCYiIVl = true;}
      if(TtNKRjpBKt == WBctzeakHQ){DkCWDFldDP = true;}
      while(FYoOrIjBkQ == kVffzryusx){YuzSzBGRZi = true;}
      while(fGALNtdRRt == fGALNtdRRt){MraajcijSS = true;}
      while(WBctzeakHQ == WBctzeakHQ){DbzTiqJONt = true;}
      if(BXJhwcRDZX == true){BXJhwcRDZX = false;}
      if(nbXQTNTRVW == true){nbXQTNTRVW = false;}
      if(RXTyQDwtPM == true){RXTyQDwtPM = false;}
      if(gytUOIxSPk == true){gytUOIxSPk = false;}
      if(cZrtFudMZk == true){cZrtFudMZk = false;}
      if(KIOkycELQr == true){KIOkycELQr = false;}
      if(gWnNpphCBL == true){gWnNpphCBL = false;}
      if(QBCksqfYrV == true){QBCksqfYrV = false;}
      if(ZiUdCYiIVl == true){ZiUdCYiIVl = false;}
      if(DkCWDFldDP == true){DkCWDFldDP = false;}
      if(NaLOUJGdqG == true){NaLOUJGdqG = false;}
      if(weSuSxCJLR == true){weSuSxCJLR = false;}
      if(GYfOHezehn == true){GYfOHezehn = false;}
      if(FCVbHyXGJr == true){FCVbHyXGJr = false;}
      if(RFROoYRGEG == true){RFROoYRGEG = false;}
      if(DbSlIouwGo == true){DbSlIouwGo = false;}
      if(JauDBtJpnT == true){JauDBtJpnT = false;}
      if(YuzSzBGRZi == true){YuzSzBGRZi = false;}
      if(MraajcijSS == true){MraajcijSS = false;}
      if(DbzTiqJONt == true){DbzTiqJONt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NBEPWZIKUS
{ 
  void ktuTsWfZPA()
  { 
      bool HbNSnfRyHB = false;
      bool RXbZYRTWxY = false;
      bool RZmKYVhQNQ = false;
      bool QAgsGbqtSl = false;
      bool uSeUHrtQWC = false;
      bool DgUbVhnDsS = false;
      bool xBjpjRphSx = false;
      bool ZZDKdytRDg = false;
      bool aQsKNjeEXY = false;
      bool mnBAoshoko = false;
      bool BMyYcddJyW = false;
      bool nWzLrgHYbA = false;
      bool mlJsKlXDVj = false;
      bool PEgNeNSThj = false;
      bool FVGBkQnoIH = false;
      bool lIMjCPCEOq = false;
      bool FZnNHiolfd = false;
      bool OmqrtReIIt = false;
      bool xitNyGrwKP = false;
      bool LNFqdegrsB = false;
      string nmmkdTdUjy;
      string UDAtkzfTIr;
      string ICRJwSGYpr;
      string xMjKnlTheQ;
      string NxyqlDSQVy;
      string ZpJQqkVcnL;
      string NFerBITpFy;
      string fynqdLEZXs;
      string zpCrSGCTTC;
      string PpsTDHSGpd;
      string jcKWRaBSle;
      string guEtLWDmZt;
      string FzWCioEUnW;
      string syitJWeeZq;
      string EignVTlyHf;
      string EegwXjGInc;
      string WgePliMJko;
      string rYNhoVRhgO;
      string DFMHRNRmWZ;
      string PWpwTXERBm;
      if(nmmkdTdUjy == jcKWRaBSle){HbNSnfRyHB = true;}
      else if(jcKWRaBSle == nmmkdTdUjy){BMyYcddJyW = true;}
      if(UDAtkzfTIr == guEtLWDmZt){RXbZYRTWxY = true;}
      else if(guEtLWDmZt == UDAtkzfTIr){nWzLrgHYbA = true;}
      if(ICRJwSGYpr == FzWCioEUnW){RZmKYVhQNQ = true;}
      else if(FzWCioEUnW == ICRJwSGYpr){mlJsKlXDVj = true;}
      if(xMjKnlTheQ == syitJWeeZq){QAgsGbqtSl = true;}
      else if(syitJWeeZq == xMjKnlTheQ){PEgNeNSThj = true;}
      if(NxyqlDSQVy == EignVTlyHf){uSeUHrtQWC = true;}
      else if(EignVTlyHf == NxyqlDSQVy){FVGBkQnoIH = true;}
      if(ZpJQqkVcnL == EegwXjGInc){DgUbVhnDsS = true;}
      else if(EegwXjGInc == ZpJQqkVcnL){lIMjCPCEOq = true;}
      if(NFerBITpFy == WgePliMJko){xBjpjRphSx = true;}
      else if(WgePliMJko == NFerBITpFy){FZnNHiolfd = true;}
      if(fynqdLEZXs == rYNhoVRhgO){ZZDKdytRDg = true;}
      if(zpCrSGCTTC == DFMHRNRmWZ){aQsKNjeEXY = true;}
      if(PpsTDHSGpd == PWpwTXERBm){mnBAoshoko = true;}
      while(rYNhoVRhgO == fynqdLEZXs){OmqrtReIIt = true;}
      while(DFMHRNRmWZ == DFMHRNRmWZ){xitNyGrwKP = true;}
      while(PWpwTXERBm == PWpwTXERBm){LNFqdegrsB = true;}
      if(HbNSnfRyHB == true){HbNSnfRyHB = false;}
      if(RXbZYRTWxY == true){RXbZYRTWxY = false;}
      if(RZmKYVhQNQ == true){RZmKYVhQNQ = false;}
      if(QAgsGbqtSl == true){QAgsGbqtSl = false;}
      if(uSeUHrtQWC == true){uSeUHrtQWC = false;}
      if(DgUbVhnDsS == true){DgUbVhnDsS = false;}
      if(xBjpjRphSx == true){xBjpjRphSx = false;}
      if(ZZDKdytRDg == true){ZZDKdytRDg = false;}
      if(aQsKNjeEXY == true){aQsKNjeEXY = false;}
      if(mnBAoshoko == true){mnBAoshoko = false;}
      if(BMyYcddJyW == true){BMyYcddJyW = false;}
      if(nWzLrgHYbA == true){nWzLrgHYbA = false;}
      if(mlJsKlXDVj == true){mlJsKlXDVj = false;}
      if(PEgNeNSThj == true){PEgNeNSThj = false;}
      if(FVGBkQnoIH == true){FVGBkQnoIH = false;}
      if(lIMjCPCEOq == true){lIMjCPCEOq = false;}
      if(FZnNHiolfd == true){FZnNHiolfd = false;}
      if(OmqrtReIIt == true){OmqrtReIIt = false;}
      if(xitNyGrwKP == true){xitNyGrwKP = false;}
      if(LNFqdegrsB == true){LNFqdegrsB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SDABFYQGXY
{ 
  void VqBmbLZfRs()
  { 
      bool zjjJQcLgld = false;
      bool tHfMOHYRPZ = false;
      bool qPumLhWIta = false;
      bool EyztOKHMio = false;
      bool gziQEORGCJ = false;
      bool IlxDkioxyh = false;
      bool GCmKYuHkxE = false;
      bool zMharqfdhw = false;
      bool QQGjSSAwJf = false;
      bool UKNBieMYrI = false;
      bool GfIxJwEWKs = false;
      bool CLOCRouCqG = false;
      bool tKbRjoygqN = false;
      bool eHlZpkHJZW = false;
      bool OdlPcJwLjs = false;
      bool iJglNuDpNU = false;
      bool yWewOuYnLf = false;
      bool glcUqQYMoZ = false;
      bool slxyeUZALt = false;
      bool gRwEXqzoBh = false;
      string VMFFlucGUX;
      string moSXVmncPr;
      string JZLhzjiaAN;
      string uXTExlUQho;
      string ErcLghVEIw;
      string jFnGNwBXGC;
      string LCFGcOUOai;
      string RWtZmQwzAQ;
      string NeSEdojNqB;
      string kxKfcCmViR;
      string MHcYdMKNSw;
      string zBXongiBcm;
      string eGWZyUMOwk;
      string dZQhEGBsaQ;
      string dVutBjZUqw;
      string SlsumFjDaJ;
      string zwRhRYkokg;
      string fpZLJJLcXN;
      string tqWARrgUty;
      string VxSQRhENcc;
      if(VMFFlucGUX == MHcYdMKNSw){zjjJQcLgld = true;}
      else if(MHcYdMKNSw == VMFFlucGUX){GfIxJwEWKs = true;}
      if(moSXVmncPr == zBXongiBcm){tHfMOHYRPZ = true;}
      else if(zBXongiBcm == moSXVmncPr){CLOCRouCqG = true;}
      if(JZLhzjiaAN == eGWZyUMOwk){qPumLhWIta = true;}
      else if(eGWZyUMOwk == JZLhzjiaAN){tKbRjoygqN = true;}
      if(uXTExlUQho == dZQhEGBsaQ){EyztOKHMio = true;}
      else if(dZQhEGBsaQ == uXTExlUQho){eHlZpkHJZW = true;}
      if(ErcLghVEIw == dVutBjZUqw){gziQEORGCJ = true;}
      else if(dVutBjZUqw == ErcLghVEIw){OdlPcJwLjs = true;}
      if(jFnGNwBXGC == SlsumFjDaJ){IlxDkioxyh = true;}
      else if(SlsumFjDaJ == jFnGNwBXGC){iJglNuDpNU = true;}
      if(LCFGcOUOai == zwRhRYkokg){GCmKYuHkxE = true;}
      else if(zwRhRYkokg == LCFGcOUOai){yWewOuYnLf = true;}
      if(RWtZmQwzAQ == fpZLJJLcXN){zMharqfdhw = true;}
      if(NeSEdojNqB == tqWARrgUty){QQGjSSAwJf = true;}
      if(kxKfcCmViR == VxSQRhENcc){UKNBieMYrI = true;}
      while(fpZLJJLcXN == RWtZmQwzAQ){glcUqQYMoZ = true;}
      while(tqWARrgUty == tqWARrgUty){slxyeUZALt = true;}
      while(VxSQRhENcc == VxSQRhENcc){gRwEXqzoBh = true;}
      if(zjjJQcLgld == true){zjjJQcLgld = false;}
      if(tHfMOHYRPZ == true){tHfMOHYRPZ = false;}
      if(qPumLhWIta == true){qPumLhWIta = false;}
      if(EyztOKHMio == true){EyztOKHMio = false;}
      if(gziQEORGCJ == true){gziQEORGCJ = false;}
      if(IlxDkioxyh == true){IlxDkioxyh = false;}
      if(GCmKYuHkxE == true){GCmKYuHkxE = false;}
      if(zMharqfdhw == true){zMharqfdhw = false;}
      if(QQGjSSAwJf == true){QQGjSSAwJf = false;}
      if(UKNBieMYrI == true){UKNBieMYrI = false;}
      if(GfIxJwEWKs == true){GfIxJwEWKs = false;}
      if(CLOCRouCqG == true){CLOCRouCqG = false;}
      if(tKbRjoygqN == true){tKbRjoygqN = false;}
      if(eHlZpkHJZW == true){eHlZpkHJZW = false;}
      if(OdlPcJwLjs == true){OdlPcJwLjs = false;}
      if(iJglNuDpNU == true){iJglNuDpNU = false;}
      if(yWewOuYnLf == true){yWewOuYnLf = false;}
      if(glcUqQYMoZ == true){glcUqQYMoZ = false;}
      if(slxyeUZALt == true){slxyeUZALt = false;}
      if(gRwEXqzoBh == true){gRwEXqzoBh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BTKFFHSHZG
{ 
  void YMGZkBzsGb()
  { 
      bool QIEKGrfwjx = false;
      bool UCeRXyQhrF = false;
      bool MLepBBhGEe = false;
      bool gmyOjtqSeQ = false;
      bool uqGPEHsOZA = false;
      bool ceuUYEcNIF = false;
      bool KZqEPhDQhc = false;
      bool HxCaKUFPls = false;
      bool HTfEBCqdpe = false;
      bool QcKZGUABAW = false;
      bool EcfwsfcUKE = false;
      bool yNbOxqtmDN = false;
      bool pZaCMdmcUo = false;
      bool UEPOoemiKU = false;
      bool YWTlfCAucO = false;
      bool aNxlfdBcjO = false;
      bool szzUityMYR = false;
      bool DLbhnTFoWK = false;
      bool cqzGOHGciK = false;
      bool cJywbZXzCl = false;
      string OBUrhByOlw;
      string ljFNVbymty;
      string QmiuWmzDjj;
      string AnKVBkYbQO;
      string OqAPTmrcUh;
      string fRSCTaSZVt;
      string BueBuyZiLj;
      string DVWfjYmSai;
      string uVhCDiUICZ;
      string RzGVMTMyUV;
      string NIRjdKumSt;
      string nAylzeMbxM;
      string cQgkSDPASQ;
      string MnyFVlEyXs;
      string nhpHkRBTWg;
      string GjAIrwbfRZ;
      string ttTBhABrPM;
      string aNDEYuaYAA;
      string CudlJbnpMZ;
      string KYfxzsNnZk;
      if(OBUrhByOlw == NIRjdKumSt){QIEKGrfwjx = true;}
      else if(NIRjdKumSt == OBUrhByOlw){EcfwsfcUKE = true;}
      if(ljFNVbymty == nAylzeMbxM){UCeRXyQhrF = true;}
      else if(nAylzeMbxM == ljFNVbymty){yNbOxqtmDN = true;}
      if(QmiuWmzDjj == cQgkSDPASQ){MLepBBhGEe = true;}
      else if(cQgkSDPASQ == QmiuWmzDjj){pZaCMdmcUo = true;}
      if(AnKVBkYbQO == MnyFVlEyXs){gmyOjtqSeQ = true;}
      else if(MnyFVlEyXs == AnKVBkYbQO){UEPOoemiKU = true;}
      if(OqAPTmrcUh == nhpHkRBTWg){uqGPEHsOZA = true;}
      else if(nhpHkRBTWg == OqAPTmrcUh){YWTlfCAucO = true;}
      if(fRSCTaSZVt == GjAIrwbfRZ){ceuUYEcNIF = true;}
      else if(GjAIrwbfRZ == fRSCTaSZVt){aNxlfdBcjO = true;}
      if(BueBuyZiLj == ttTBhABrPM){KZqEPhDQhc = true;}
      else if(ttTBhABrPM == BueBuyZiLj){szzUityMYR = true;}
      if(DVWfjYmSai == aNDEYuaYAA){HxCaKUFPls = true;}
      if(uVhCDiUICZ == CudlJbnpMZ){HTfEBCqdpe = true;}
      if(RzGVMTMyUV == KYfxzsNnZk){QcKZGUABAW = true;}
      while(aNDEYuaYAA == DVWfjYmSai){DLbhnTFoWK = true;}
      while(CudlJbnpMZ == CudlJbnpMZ){cqzGOHGciK = true;}
      while(KYfxzsNnZk == KYfxzsNnZk){cJywbZXzCl = true;}
      if(QIEKGrfwjx == true){QIEKGrfwjx = false;}
      if(UCeRXyQhrF == true){UCeRXyQhrF = false;}
      if(MLepBBhGEe == true){MLepBBhGEe = false;}
      if(gmyOjtqSeQ == true){gmyOjtqSeQ = false;}
      if(uqGPEHsOZA == true){uqGPEHsOZA = false;}
      if(ceuUYEcNIF == true){ceuUYEcNIF = false;}
      if(KZqEPhDQhc == true){KZqEPhDQhc = false;}
      if(HxCaKUFPls == true){HxCaKUFPls = false;}
      if(HTfEBCqdpe == true){HTfEBCqdpe = false;}
      if(QcKZGUABAW == true){QcKZGUABAW = false;}
      if(EcfwsfcUKE == true){EcfwsfcUKE = false;}
      if(yNbOxqtmDN == true){yNbOxqtmDN = false;}
      if(pZaCMdmcUo == true){pZaCMdmcUo = false;}
      if(UEPOoemiKU == true){UEPOoemiKU = false;}
      if(YWTlfCAucO == true){YWTlfCAucO = false;}
      if(aNxlfdBcjO == true){aNxlfdBcjO = false;}
      if(szzUityMYR == true){szzUityMYR = false;}
      if(DLbhnTFoWK == true){DLbhnTFoWK = false;}
      if(cqzGOHGciK == true){cqzGOHGciK = false;}
      if(cJywbZXzCl == true){cJywbZXzCl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OBAIIILPHO
{ 
  void pZNEdXBVgy()
  { 
      bool CxkigkhKJb = false;
      bool UoUFANGTVd = false;
      bool uWHEyJoCoL = false;
      bool zHCiwKngPh = false;
      bool NAzpqPLCmC = false;
      bool IKjuffogjX = false;
      bool ueVycMmlJp = false;
      bool ZYDnPuWRNi = false;
      bool ymOlDnXGDP = false;
      bool UGwrlhdBMe = false;
      bool bYrxbiHuej = false;
      bool IXSBZIQiuo = false;
      bool nfisUScLBg = false;
      bool QVEiHYtQKT = false;
      bool fbbSHxmuRd = false;
      bool RuGXpupzbI = false;
      bool FusxTmTjsA = false;
      bool NzYaIMEPlX = false;
      bool ehsXPVOfrL = false;
      bool itnBOkhdzT = false;
      string dOxXDudAwU;
      string VPgzGdzOFr;
      string QxCrZSKWaV;
      string zOUQtSbKyK;
      string zkyPmmGbYp;
      string DogiYfLExk;
      string kHYaUHuQFn;
      string CnPusfymFl;
      string LhxkRKhkqN;
      string GWMMZrLXtf;
      string rpWknOYlps;
      string gYzKZCJmIK;
      string ZJYqgebDUB;
      string TasYtVLMxD;
      string kuuyghXCmX;
      string hXPjGiQeEI;
      string mVQoYGfZQD;
      string pxpEZdOaJy;
      string HYPxSuKJSL;
      string FgHnPOeGYa;
      if(dOxXDudAwU == rpWknOYlps){CxkigkhKJb = true;}
      else if(rpWknOYlps == dOxXDudAwU){bYrxbiHuej = true;}
      if(VPgzGdzOFr == gYzKZCJmIK){UoUFANGTVd = true;}
      else if(gYzKZCJmIK == VPgzGdzOFr){IXSBZIQiuo = true;}
      if(QxCrZSKWaV == ZJYqgebDUB){uWHEyJoCoL = true;}
      else if(ZJYqgebDUB == QxCrZSKWaV){nfisUScLBg = true;}
      if(zOUQtSbKyK == TasYtVLMxD){zHCiwKngPh = true;}
      else if(TasYtVLMxD == zOUQtSbKyK){QVEiHYtQKT = true;}
      if(zkyPmmGbYp == kuuyghXCmX){NAzpqPLCmC = true;}
      else if(kuuyghXCmX == zkyPmmGbYp){fbbSHxmuRd = true;}
      if(DogiYfLExk == hXPjGiQeEI){IKjuffogjX = true;}
      else if(hXPjGiQeEI == DogiYfLExk){RuGXpupzbI = true;}
      if(kHYaUHuQFn == mVQoYGfZQD){ueVycMmlJp = true;}
      else if(mVQoYGfZQD == kHYaUHuQFn){FusxTmTjsA = true;}
      if(CnPusfymFl == pxpEZdOaJy){ZYDnPuWRNi = true;}
      if(LhxkRKhkqN == HYPxSuKJSL){ymOlDnXGDP = true;}
      if(GWMMZrLXtf == FgHnPOeGYa){UGwrlhdBMe = true;}
      while(pxpEZdOaJy == CnPusfymFl){NzYaIMEPlX = true;}
      while(HYPxSuKJSL == HYPxSuKJSL){ehsXPVOfrL = true;}
      while(FgHnPOeGYa == FgHnPOeGYa){itnBOkhdzT = true;}
      if(CxkigkhKJb == true){CxkigkhKJb = false;}
      if(UoUFANGTVd == true){UoUFANGTVd = false;}
      if(uWHEyJoCoL == true){uWHEyJoCoL = false;}
      if(zHCiwKngPh == true){zHCiwKngPh = false;}
      if(NAzpqPLCmC == true){NAzpqPLCmC = false;}
      if(IKjuffogjX == true){IKjuffogjX = false;}
      if(ueVycMmlJp == true){ueVycMmlJp = false;}
      if(ZYDnPuWRNi == true){ZYDnPuWRNi = false;}
      if(ymOlDnXGDP == true){ymOlDnXGDP = false;}
      if(UGwrlhdBMe == true){UGwrlhdBMe = false;}
      if(bYrxbiHuej == true){bYrxbiHuej = false;}
      if(IXSBZIQiuo == true){IXSBZIQiuo = false;}
      if(nfisUScLBg == true){nfisUScLBg = false;}
      if(QVEiHYtQKT == true){QVEiHYtQKT = false;}
      if(fbbSHxmuRd == true){fbbSHxmuRd = false;}
      if(RuGXpupzbI == true){RuGXpupzbI = false;}
      if(FusxTmTjsA == true){FusxTmTjsA = false;}
      if(NzYaIMEPlX == true){NzYaIMEPlX = false;}
      if(ehsXPVOfrL == true){ehsXPVOfrL = false;}
      if(itnBOkhdzT == true){itnBOkhdzT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XDUPSIETZH
{ 
  void AGMqVHoTeO()
  { 
      bool rGfFnVjPhX = false;
      bool HzdePudgnP = false;
      bool fQmSIbApSk = false;
      bool uDfzteADbK = false;
      bool lONrpESmkc = false;
      bool ENnfWbSMwM = false;
      bool UtxxcDSejj = false;
      bool uZEcdRCtbE = false;
      bool JNEoggPAcS = false;
      bool LmHOpqRQPm = false;
      bool lmCAhdIKBE = false;
      bool YXkrDHoHQu = false;
      bool nRCRXHSpml = false;
      bool AldoRVKEHC = false;
      bool YEyXsnCOLi = false;
      bool BJAUyTkkba = false;
      bool LSXPEmsuag = false;
      bool xkANVjTdXj = false;
      bool EDdDjplJqx = false;
      bool kYbfqFgiSG = false;
      string fIdHpgOUsN;
      string pMxffmsImB;
      string YrGVwfDRfr;
      string norUBUzNVs;
      string nHCQwoObWi;
      string kCVDBnBHLJ;
      string dxoAZBUcdL;
      string memsMQooYH;
      string QatwHkToSN;
      string SrBLPqSBqu;
      string TlxKuFPbpj;
      string yMhErCCrso;
      string heKQsudbdU;
      string xmydmecjBB;
      string VryhOGJxSN;
      string naIBNnagRA;
      string bmxVlLNDQM;
      string YSprTogbCm;
      string kfJQgWewQy;
      string EwRAYhtcuD;
      if(fIdHpgOUsN == TlxKuFPbpj){rGfFnVjPhX = true;}
      else if(TlxKuFPbpj == fIdHpgOUsN){lmCAhdIKBE = true;}
      if(pMxffmsImB == yMhErCCrso){HzdePudgnP = true;}
      else if(yMhErCCrso == pMxffmsImB){YXkrDHoHQu = true;}
      if(YrGVwfDRfr == heKQsudbdU){fQmSIbApSk = true;}
      else if(heKQsudbdU == YrGVwfDRfr){nRCRXHSpml = true;}
      if(norUBUzNVs == xmydmecjBB){uDfzteADbK = true;}
      else if(xmydmecjBB == norUBUzNVs){AldoRVKEHC = true;}
      if(nHCQwoObWi == VryhOGJxSN){lONrpESmkc = true;}
      else if(VryhOGJxSN == nHCQwoObWi){YEyXsnCOLi = true;}
      if(kCVDBnBHLJ == naIBNnagRA){ENnfWbSMwM = true;}
      else if(naIBNnagRA == kCVDBnBHLJ){BJAUyTkkba = true;}
      if(dxoAZBUcdL == bmxVlLNDQM){UtxxcDSejj = true;}
      else if(bmxVlLNDQM == dxoAZBUcdL){LSXPEmsuag = true;}
      if(memsMQooYH == YSprTogbCm){uZEcdRCtbE = true;}
      if(QatwHkToSN == kfJQgWewQy){JNEoggPAcS = true;}
      if(SrBLPqSBqu == EwRAYhtcuD){LmHOpqRQPm = true;}
      while(YSprTogbCm == memsMQooYH){xkANVjTdXj = true;}
      while(kfJQgWewQy == kfJQgWewQy){EDdDjplJqx = true;}
      while(EwRAYhtcuD == EwRAYhtcuD){kYbfqFgiSG = true;}
      if(rGfFnVjPhX == true){rGfFnVjPhX = false;}
      if(HzdePudgnP == true){HzdePudgnP = false;}
      if(fQmSIbApSk == true){fQmSIbApSk = false;}
      if(uDfzteADbK == true){uDfzteADbK = false;}
      if(lONrpESmkc == true){lONrpESmkc = false;}
      if(ENnfWbSMwM == true){ENnfWbSMwM = false;}
      if(UtxxcDSejj == true){UtxxcDSejj = false;}
      if(uZEcdRCtbE == true){uZEcdRCtbE = false;}
      if(JNEoggPAcS == true){JNEoggPAcS = false;}
      if(LmHOpqRQPm == true){LmHOpqRQPm = false;}
      if(lmCAhdIKBE == true){lmCAhdIKBE = false;}
      if(YXkrDHoHQu == true){YXkrDHoHQu = false;}
      if(nRCRXHSpml == true){nRCRXHSpml = false;}
      if(AldoRVKEHC == true){AldoRVKEHC = false;}
      if(YEyXsnCOLi == true){YEyXsnCOLi = false;}
      if(BJAUyTkkba == true){BJAUyTkkba = false;}
      if(LSXPEmsuag == true){LSXPEmsuag = false;}
      if(xkANVjTdXj == true){xkANVjTdXj = false;}
      if(EDdDjplJqx == true){EDdDjplJqx = false;}
      if(kYbfqFgiSG == true){kYbfqFgiSG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XBJMUWKWSV
{ 
  void DpVmGjFBey()
  { 
      bool pqBzOmwDzn = false;
      bool hcoKoAFDON = false;
      bool sJbzicHkpk = false;
      bool ZeeXGxaASr = false;
      bool ypFndIwjrs = false;
      bool hSjctgPCQk = false;
      bool eLNNNDyRDr = false;
      bool ZEkVODDnUZ = false;
      bool fWrwCGuBdZ = false;
      bool hXjhNmbqZa = false;
      bool KegATdhNOq = false;
      bool cahpEHSRLH = false;
      bool IwmdKBTCIb = false;
      bool aLOtPBprbY = false;
      bool YhllwHptcF = false;
      bool TKfSRNqDoR = false;
      bool JeElPRLmaD = false;
      bool QZomhaKAgC = false;
      bool LZZDHQewbC = false;
      bool JmCKOsczWe = false;
      string XDaCWmhCwV;
      string EOmjChYXDd;
      string OXsFNOGRrx;
      string jslxyFilfM;
      string MGtaBJKbFO;
      string GRmfcAJUnI;
      string fQJykKbplx;
      string ymqqQIdSDE;
      string EdLcpSFGUy;
      string uNxXfuwuoq;
      string gTpIPSZwOU;
      string VqRjECFfHR;
      string FUVnBkpTKU;
      string LhcMXjnCMX;
      string sIrzbEwNTc;
      string pPnXkLyBHe;
      string cFksInOELT;
      string ODTjYxcVMw;
      string ocAgeutEbM;
      string kluhtmWyqn;
      if(XDaCWmhCwV == gTpIPSZwOU){pqBzOmwDzn = true;}
      else if(gTpIPSZwOU == XDaCWmhCwV){KegATdhNOq = true;}
      if(EOmjChYXDd == VqRjECFfHR){hcoKoAFDON = true;}
      else if(VqRjECFfHR == EOmjChYXDd){cahpEHSRLH = true;}
      if(OXsFNOGRrx == FUVnBkpTKU){sJbzicHkpk = true;}
      else if(FUVnBkpTKU == OXsFNOGRrx){IwmdKBTCIb = true;}
      if(jslxyFilfM == LhcMXjnCMX){ZeeXGxaASr = true;}
      else if(LhcMXjnCMX == jslxyFilfM){aLOtPBprbY = true;}
      if(MGtaBJKbFO == sIrzbEwNTc){ypFndIwjrs = true;}
      else if(sIrzbEwNTc == MGtaBJKbFO){YhllwHptcF = true;}
      if(GRmfcAJUnI == pPnXkLyBHe){hSjctgPCQk = true;}
      else if(pPnXkLyBHe == GRmfcAJUnI){TKfSRNqDoR = true;}
      if(fQJykKbplx == cFksInOELT){eLNNNDyRDr = true;}
      else if(cFksInOELT == fQJykKbplx){JeElPRLmaD = true;}
      if(ymqqQIdSDE == ODTjYxcVMw){ZEkVODDnUZ = true;}
      if(EdLcpSFGUy == ocAgeutEbM){fWrwCGuBdZ = true;}
      if(uNxXfuwuoq == kluhtmWyqn){hXjhNmbqZa = true;}
      while(ODTjYxcVMw == ymqqQIdSDE){QZomhaKAgC = true;}
      while(ocAgeutEbM == ocAgeutEbM){LZZDHQewbC = true;}
      while(kluhtmWyqn == kluhtmWyqn){JmCKOsczWe = true;}
      if(pqBzOmwDzn == true){pqBzOmwDzn = false;}
      if(hcoKoAFDON == true){hcoKoAFDON = false;}
      if(sJbzicHkpk == true){sJbzicHkpk = false;}
      if(ZeeXGxaASr == true){ZeeXGxaASr = false;}
      if(ypFndIwjrs == true){ypFndIwjrs = false;}
      if(hSjctgPCQk == true){hSjctgPCQk = false;}
      if(eLNNNDyRDr == true){eLNNNDyRDr = false;}
      if(ZEkVODDnUZ == true){ZEkVODDnUZ = false;}
      if(fWrwCGuBdZ == true){fWrwCGuBdZ = false;}
      if(hXjhNmbqZa == true){hXjhNmbqZa = false;}
      if(KegATdhNOq == true){KegATdhNOq = false;}
      if(cahpEHSRLH == true){cahpEHSRLH = false;}
      if(IwmdKBTCIb == true){IwmdKBTCIb = false;}
      if(aLOtPBprbY == true){aLOtPBprbY = false;}
      if(YhllwHptcF == true){YhllwHptcF = false;}
      if(TKfSRNqDoR == true){TKfSRNqDoR = false;}
      if(JeElPRLmaD == true){JeElPRLmaD = false;}
      if(QZomhaKAgC == true){QZomhaKAgC = false;}
      if(LZZDHQewbC == true){LZZDHQewbC = false;}
      if(JmCKOsczWe == true){JmCKOsczWe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UZFSPQJQKN
{ 
  void BjJjfodoVE()
  { 
      bool VDwERgXbQz = false;
      bool HZtbhjocWD = false;
      bool BHlyKxgunz = false;
      bool DPCIsDAIsr = false;
      bool VAUSydTyGe = false;
      bool EARZAOQytU = false;
      bool zbmRwrWkid = false;
      bool jgiehBmTzf = false;
      bool cioIuCxbHe = false;
      bool mBOYrsMDWG = false;
      bool HhObtXQlrz = false;
      bool bxyNqQmdWs = false;
      bool tucEqiRKmn = false;
      bool niwsYtTMOW = false;
      bool kKjIDpDAgb = false;
      bool GhgcLLEKkL = false;
      bool GGzDKrAEow = false;
      bool wZPzHjKUAA = false;
      bool fQcKgFCShM = false;
      bool oIbCQeKgxK = false;
      string mTHBOiMHJN;
      string bVYQiimKJI;
      string fGcRJShwOe;
      string ncjqCxEQmK;
      string eLNbhnuOHh;
      string CHenRfqoru;
      string APreYzlgOk;
      string dWXzdTrAiU;
      string XTofcgywYV;
      string GOYqJPSiDi;
      string CNzWDeieKy;
      string nqGWXfZLCq;
      string BpHsePyfoN;
      string AJxASJUSbx;
      string xTkagPIBSd;
      string oHgLgMFGqn;
      string gDowxesFEO;
      string LHiEthEfLP;
      string GwNLYsclSG;
      string DOanPtdodC;
      if(mTHBOiMHJN == CNzWDeieKy){VDwERgXbQz = true;}
      else if(CNzWDeieKy == mTHBOiMHJN){HhObtXQlrz = true;}
      if(bVYQiimKJI == nqGWXfZLCq){HZtbhjocWD = true;}
      else if(nqGWXfZLCq == bVYQiimKJI){bxyNqQmdWs = true;}
      if(fGcRJShwOe == BpHsePyfoN){BHlyKxgunz = true;}
      else if(BpHsePyfoN == fGcRJShwOe){tucEqiRKmn = true;}
      if(ncjqCxEQmK == AJxASJUSbx){DPCIsDAIsr = true;}
      else if(AJxASJUSbx == ncjqCxEQmK){niwsYtTMOW = true;}
      if(eLNbhnuOHh == xTkagPIBSd){VAUSydTyGe = true;}
      else if(xTkagPIBSd == eLNbhnuOHh){kKjIDpDAgb = true;}
      if(CHenRfqoru == oHgLgMFGqn){EARZAOQytU = true;}
      else if(oHgLgMFGqn == CHenRfqoru){GhgcLLEKkL = true;}
      if(APreYzlgOk == gDowxesFEO){zbmRwrWkid = true;}
      else if(gDowxesFEO == APreYzlgOk){GGzDKrAEow = true;}
      if(dWXzdTrAiU == LHiEthEfLP){jgiehBmTzf = true;}
      if(XTofcgywYV == GwNLYsclSG){cioIuCxbHe = true;}
      if(GOYqJPSiDi == DOanPtdodC){mBOYrsMDWG = true;}
      while(LHiEthEfLP == dWXzdTrAiU){wZPzHjKUAA = true;}
      while(GwNLYsclSG == GwNLYsclSG){fQcKgFCShM = true;}
      while(DOanPtdodC == DOanPtdodC){oIbCQeKgxK = true;}
      if(VDwERgXbQz == true){VDwERgXbQz = false;}
      if(HZtbhjocWD == true){HZtbhjocWD = false;}
      if(BHlyKxgunz == true){BHlyKxgunz = false;}
      if(DPCIsDAIsr == true){DPCIsDAIsr = false;}
      if(VAUSydTyGe == true){VAUSydTyGe = false;}
      if(EARZAOQytU == true){EARZAOQytU = false;}
      if(zbmRwrWkid == true){zbmRwrWkid = false;}
      if(jgiehBmTzf == true){jgiehBmTzf = false;}
      if(cioIuCxbHe == true){cioIuCxbHe = false;}
      if(mBOYrsMDWG == true){mBOYrsMDWG = false;}
      if(HhObtXQlrz == true){HhObtXQlrz = false;}
      if(bxyNqQmdWs == true){bxyNqQmdWs = false;}
      if(tucEqiRKmn == true){tucEqiRKmn = false;}
      if(niwsYtTMOW == true){niwsYtTMOW = false;}
      if(kKjIDpDAgb == true){kKjIDpDAgb = false;}
      if(GhgcLLEKkL == true){GhgcLLEKkL = false;}
      if(GGzDKrAEow == true){GGzDKrAEow = false;}
      if(wZPzHjKUAA == true){wZPzHjKUAA = false;}
      if(fQcKgFCShM == true){fQcKgFCShM = false;}
      if(oIbCQeKgxK == true){oIbCQeKgxK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NCVFBRXFAE
{ 
  void GKOPfeSMLr()
  { 
      bool NDlJJmIKWL = false;
      bool megprnBjZm = false;
      bool gUoxsbZaoJ = false;
      bool EYFdQYoWij = false;
      bool TQFuGkPyJU = false;
      bool DjDWysNnPg = false;
      bool NgUMHOZDYS = false;
      bool ydApTPPsqI = false;
      bool LbRUFUUheX = false;
      bool KBHCWlwrnr = false;
      bool KMYpWYiNDi = false;
      bool dUxGUlPAHm = false;
      bool tqYBIxWySg = false;
      bool SWFyxzDayg = false;
      bool kGduspDAgk = false;
      bool WwkBxTmWhZ = false;
      bool CjuVPGOyIV = false;
      bool inBaBnNcHR = false;
      bool sWRhKWlEBd = false;
      bool LfrYdBLIfm = false;
      string aaSorbgrAS;
      string DzaSpmGqFO;
      string CQOfFbooag;
      string rdUgpfYrDh;
      string QCzNgzeOUJ;
      string kVbVcJLljQ;
      string mqRSZYdtbK;
      string NOmWyzQBzG;
      string wZTRuWCzDf;
      string UmImuYkgnH;
      string UklKuWiEWd;
      string rJmdTMWzXC;
      string WepwoaGAaY;
      string NjNqkKmJVY;
      string nxbZmKzeSh;
      string BlIkjpupUT;
      string PicxPHwwrJ;
      string YNcyOYCPIe;
      string bbgrXbabGt;
      string PxnBIFUdKe;
      if(aaSorbgrAS == UklKuWiEWd){NDlJJmIKWL = true;}
      else if(UklKuWiEWd == aaSorbgrAS){KMYpWYiNDi = true;}
      if(DzaSpmGqFO == rJmdTMWzXC){megprnBjZm = true;}
      else if(rJmdTMWzXC == DzaSpmGqFO){dUxGUlPAHm = true;}
      if(CQOfFbooag == WepwoaGAaY){gUoxsbZaoJ = true;}
      else if(WepwoaGAaY == CQOfFbooag){tqYBIxWySg = true;}
      if(rdUgpfYrDh == NjNqkKmJVY){EYFdQYoWij = true;}
      else if(NjNqkKmJVY == rdUgpfYrDh){SWFyxzDayg = true;}
      if(QCzNgzeOUJ == nxbZmKzeSh){TQFuGkPyJU = true;}
      else if(nxbZmKzeSh == QCzNgzeOUJ){kGduspDAgk = true;}
      if(kVbVcJLljQ == BlIkjpupUT){DjDWysNnPg = true;}
      else if(BlIkjpupUT == kVbVcJLljQ){WwkBxTmWhZ = true;}
      if(mqRSZYdtbK == PicxPHwwrJ){NgUMHOZDYS = true;}
      else if(PicxPHwwrJ == mqRSZYdtbK){CjuVPGOyIV = true;}
      if(NOmWyzQBzG == YNcyOYCPIe){ydApTPPsqI = true;}
      if(wZTRuWCzDf == bbgrXbabGt){LbRUFUUheX = true;}
      if(UmImuYkgnH == PxnBIFUdKe){KBHCWlwrnr = true;}
      while(YNcyOYCPIe == NOmWyzQBzG){inBaBnNcHR = true;}
      while(bbgrXbabGt == bbgrXbabGt){sWRhKWlEBd = true;}
      while(PxnBIFUdKe == PxnBIFUdKe){LfrYdBLIfm = true;}
      if(NDlJJmIKWL == true){NDlJJmIKWL = false;}
      if(megprnBjZm == true){megprnBjZm = false;}
      if(gUoxsbZaoJ == true){gUoxsbZaoJ = false;}
      if(EYFdQYoWij == true){EYFdQYoWij = false;}
      if(TQFuGkPyJU == true){TQFuGkPyJU = false;}
      if(DjDWysNnPg == true){DjDWysNnPg = false;}
      if(NgUMHOZDYS == true){NgUMHOZDYS = false;}
      if(ydApTPPsqI == true){ydApTPPsqI = false;}
      if(LbRUFUUheX == true){LbRUFUUheX = false;}
      if(KBHCWlwrnr == true){KBHCWlwrnr = false;}
      if(KMYpWYiNDi == true){KMYpWYiNDi = false;}
      if(dUxGUlPAHm == true){dUxGUlPAHm = false;}
      if(tqYBIxWySg == true){tqYBIxWySg = false;}
      if(SWFyxzDayg == true){SWFyxzDayg = false;}
      if(kGduspDAgk == true){kGduspDAgk = false;}
      if(WwkBxTmWhZ == true){WwkBxTmWhZ = false;}
      if(CjuVPGOyIV == true){CjuVPGOyIV = false;}
      if(inBaBnNcHR == true){inBaBnNcHR = false;}
      if(sWRhKWlEBd == true){sWRhKWlEBd = false;}
      if(LfrYdBLIfm == true){LfrYdBLIfm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MFCLKCBCXJ
{ 
  void eMCFXwwyrL()
  { 
      bool kYKhTqCSiu = false;
      bool nNqrjRBTkk = false;
      bool kmcQAhxrPW = false;
      bool BKCjawasqQ = false;
      bool IypQcBETyG = false;
      bool SRFtcyusgk = false;
      bool sJCMtdKHQm = false;
      bool qIbUclxMnz = false;
      bool ddtIDiEIGc = false;
      bool BgLFWyudVy = false;
      bool qxCKPKpkRp = false;
      bool BiFhHOOsIK = false;
      bool YBAnlJwVxd = false;
      bool pSpoHfbjss = false;
      bool sTqQbRbdxH = false;
      bool iCULYowOYF = false;
      bool NTnmAhsPZa = false;
      bool mezPRFKjMT = false;
      bool hRXotnxdPS = false;
      bool jNKneYeDQs = false;
      string ScsQmoNofa;
      string czBLwuXLlM;
      string NwHcounKyE;
      string rSYdKdOKPO;
      string dhzuwDdCfV;
      string twMlQVVQbq;
      string BDpaiJZXAR;
      string gMwXifwLgk;
      string QldbJwGmMn;
      string IxiXNZqYMl;
      string cGBNogAoCk;
      string ZKjpZOYNQm;
      string DlKMJidXJI;
      string nXytHKkPmb;
      string wQkbOxNrxy;
      string AcHZhpunEF;
      string DbeswBLohP;
      string dJanpmBhxM;
      string VkaLIKQYLR;
      string OwLhYTkqeN;
      if(ScsQmoNofa == cGBNogAoCk){kYKhTqCSiu = true;}
      else if(cGBNogAoCk == ScsQmoNofa){qxCKPKpkRp = true;}
      if(czBLwuXLlM == ZKjpZOYNQm){nNqrjRBTkk = true;}
      else if(ZKjpZOYNQm == czBLwuXLlM){BiFhHOOsIK = true;}
      if(NwHcounKyE == DlKMJidXJI){kmcQAhxrPW = true;}
      else if(DlKMJidXJI == NwHcounKyE){YBAnlJwVxd = true;}
      if(rSYdKdOKPO == nXytHKkPmb){BKCjawasqQ = true;}
      else if(nXytHKkPmb == rSYdKdOKPO){pSpoHfbjss = true;}
      if(dhzuwDdCfV == wQkbOxNrxy){IypQcBETyG = true;}
      else if(wQkbOxNrxy == dhzuwDdCfV){sTqQbRbdxH = true;}
      if(twMlQVVQbq == AcHZhpunEF){SRFtcyusgk = true;}
      else if(AcHZhpunEF == twMlQVVQbq){iCULYowOYF = true;}
      if(BDpaiJZXAR == DbeswBLohP){sJCMtdKHQm = true;}
      else if(DbeswBLohP == BDpaiJZXAR){NTnmAhsPZa = true;}
      if(gMwXifwLgk == dJanpmBhxM){qIbUclxMnz = true;}
      if(QldbJwGmMn == VkaLIKQYLR){ddtIDiEIGc = true;}
      if(IxiXNZqYMl == OwLhYTkqeN){BgLFWyudVy = true;}
      while(dJanpmBhxM == gMwXifwLgk){mezPRFKjMT = true;}
      while(VkaLIKQYLR == VkaLIKQYLR){hRXotnxdPS = true;}
      while(OwLhYTkqeN == OwLhYTkqeN){jNKneYeDQs = true;}
      if(kYKhTqCSiu == true){kYKhTqCSiu = false;}
      if(nNqrjRBTkk == true){nNqrjRBTkk = false;}
      if(kmcQAhxrPW == true){kmcQAhxrPW = false;}
      if(BKCjawasqQ == true){BKCjawasqQ = false;}
      if(IypQcBETyG == true){IypQcBETyG = false;}
      if(SRFtcyusgk == true){SRFtcyusgk = false;}
      if(sJCMtdKHQm == true){sJCMtdKHQm = false;}
      if(qIbUclxMnz == true){qIbUclxMnz = false;}
      if(ddtIDiEIGc == true){ddtIDiEIGc = false;}
      if(BgLFWyudVy == true){BgLFWyudVy = false;}
      if(qxCKPKpkRp == true){qxCKPKpkRp = false;}
      if(BiFhHOOsIK == true){BiFhHOOsIK = false;}
      if(YBAnlJwVxd == true){YBAnlJwVxd = false;}
      if(pSpoHfbjss == true){pSpoHfbjss = false;}
      if(sTqQbRbdxH == true){sTqQbRbdxH = false;}
      if(iCULYowOYF == true){iCULYowOYF = false;}
      if(NTnmAhsPZa == true){NTnmAhsPZa = false;}
      if(mezPRFKjMT == true){mezPRFKjMT = false;}
      if(hRXotnxdPS == true){hRXotnxdPS = false;}
      if(jNKneYeDQs == true){jNKneYeDQs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZGMEAWIGQV
{ 
  void rLOdqKNjxd()
  { 
      bool jMwSOKUuYN = false;
      bool blUnyZuVeI = false;
      bool UrOEAcDcpA = false;
      bool rOHYdOzppa = false;
      bool KnUuXXBGQA = false;
      bool yCxQnryqSk = false;
      bool HqFXUMdwKj = false;
      bool ZFfDygXSgc = false;
      bool bxMoBoQqkl = false;
      bool QIWETTiwZK = false;
      bool sEufasVsRG = false;
      bool TeJpJKdPVf = false;
      bool umLYTRhbje = false;
      bool xasWiWCgOI = false;
      bool SWYdRkzguJ = false;
      bool oSZtOGsVMJ = false;
      bool LtJmJsklHl = false;
      bool HxBNMYNgHh = false;
      bool RAgZoeByIb = false;
      bool cfUExYNlUY = false;
      string nfoUkoHZUX;
      string NTDBqwkNBm;
      string pQMsZAnbRw;
      string pAYNnmpSFq;
      string lDwqJBFNqq;
      string jAFPTWTuuj;
      string BfHXgMuoNE;
      string GSIKBmUVjY;
      string tZjZMpuItQ;
      string TuWUpFuXJw;
      string FPAmkQruNf;
      string CxKWpauhLq;
      string RluwjLunTw;
      string rwLmSeZxou;
      string xAtNOVjAeP;
      string JNLSeTTDCZ;
      string UEaRnFlWEL;
      string AyrQsRqhhN;
      string auJElejsIz;
      string KWtOZsYcZJ;
      if(nfoUkoHZUX == FPAmkQruNf){jMwSOKUuYN = true;}
      else if(FPAmkQruNf == nfoUkoHZUX){sEufasVsRG = true;}
      if(NTDBqwkNBm == CxKWpauhLq){blUnyZuVeI = true;}
      else if(CxKWpauhLq == NTDBqwkNBm){TeJpJKdPVf = true;}
      if(pQMsZAnbRw == RluwjLunTw){UrOEAcDcpA = true;}
      else if(RluwjLunTw == pQMsZAnbRw){umLYTRhbje = true;}
      if(pAYNnmpSFq == rwLmSeZxou){rOHYdOzppa = true;}
      else if(rwLmSeZxou == pAYNnmpSFq){xasWiWCgOI = true;}
      if(lDwqJBFNqq == xAtNOVjAeP){KnUuXXBGQA = true;}
      else if(xAtNOVjAeP == lDwqJBFNqq){SWYdRkzguJ = true;}
      if(jAFPTWTuuj == JNLSeTTDCZ){yCxQnryqSk = true;}
      else if(JNLSeTTDCZ == jAFPTWTuuj){oSZtOGsVMJ = true;}
      if(BfHXgMuoNE == UEaRnFlWEL){HqFXUMdwKj = true;}
      else if(UEaRnFlWEL == BfHXgMuoNE){LtJmJsklHl = true;}
      if(GSIKBmUVjY == AyrQsRqhhN){ZFfDygXSgc = true;}
      if(tZjZMpuItQ == auJElejsIz){bxMoBoQqkl = true;}
      if(TuWUpFuXJw == KWtOZsYcZJ){QIWETTiwZK = true;}
      while(AyrQsRqhhN == GSIKBmUVjY){HxBNMYNgHh = true;}
      while(auJElejsIz == auJElejsIz){RAgZoeByIb = true;}
      while(KWtOZsYcZJ == KWtOZsYcZJ){cfUExYNlUY = true;}
      if(jMwSOKUuYN == true){jMwSOKUuYN = false;}
      if(blUnyZuVeI == true){blUnyZuVeI = false;}
      if(UrOEAcDcpA == true){UrOEAcDcpA = false;}
      if(rOHYdOzppa == true){rOHYdOzppa = false;}
      if(KnUuXXBGQA == true){KnUuXXBGQA = false;}
      if(yCxQnryqSk == true){yCxQnryqSk = false;}
      if(HqFXUMdwKj == true){HqFXUMdwKj = false;}
      if(ZFfDygXSgc == true){ZFfDygXSgc = false;}
      if(bxMoBoQqkl == true){bxMoBoQqkl = false;}
      if(QIWETTiwZK == true){QIWETTiwZK = false;}
      if(sEufasVsRG == true){sEufasVsRG = false;}
      if(TeJpJKdPVf == true){TeJpJKdPVf = false;}
      if(umLYTRhbje == true){umLYTRhbje = false;}
      if(xasWiWCgOI == true){xasWiWCgOI = false;}
      if(SWYdRkzguJ == true){SWYdRkzguJ = false;}
      if(oSZtOGsVMJ == true){oSZtOGsVMJ = false;}
      if(LtJmJsklHl == true){LtJmJsklHl = false;}
      if(HxBNMYNgHh == true){HxBNMYNgHh = false;}
      if(RAgZoeByIb == true){RAgZoeByIb = false;}
      if(cfUExYNlUY == true){cfUExYNlUY = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SDOFTVBKGI
{ 
  void uAgKMjdjZn()
  { 
      bool TpjkIutoHq = false;
      bool ELkKRNcWUe = false;
      bool wRTcoNZXtE = false;
      bool MhJrVZBAzb = false;
      bool lpqogNWYXZ = false;
      bool bmKirCWHwt = false;
      bool UWtCdWBsor = false;
      bool McXrsFZdua = false;
      bool pBzUcZWazS = false;
      bool CtDEHOuoLK = false;
      bool LcieNyPRom = false;
      bool crLUDTWZYE = false;
      bool zatWoRYSkd = false;
      bool JkSjONRUGe = false;
      bool JFkMpKhiAm = false;
      bool rVPGLkiMou = false;
      bool ZxgLcqZaFp = false;
      bool BCStttohfe = false;
      bool ioSxlnVLYd = false;
      bool eFeLEMXZjz = false;
      string WlgAImrpLO;
      string xWfUCzkoYo;
      string TruOfwAitN;
      string VfsGaIFjhM;
      string KnslrcGJVh;
      string mEbAfepSTM;
      string mIwxsQcYss;
      string sBGLefzJWC;
      string ppNLZQEsaw;
      string RmPVZBrusa;
      string AgPbnlyPVA;
      string egwQWwujWb;
      string LjyxaGkjkd;
      string nDQuCIakWn;
      string SnWZQkqyzd;
      string FHXQOQiSBh;
      string ANdrLbQAON;
      string VKMiJXMXgp;
      string PlkHXgfzgk;
      string YIjonEKeNo;
      if(WlgAImrpLO == AgPbnlyPVA){TpjkIutoHq = true;}
      else if(AgPbnlyPVA == WlgAImrpLO){LcieNyPRom = true;}
      if(xWfUCzkoYo == egwQWwujWb){ELkKRNcWUe = true;}
      else if(egwQWwujWb == xWfUCzkoYo){crLUDTWZYE = true;}
      if(TruOfwAitN == LjyxaGkjkd){wRTcoNZXtE = true;}
      else if(LjyxaGkjkd == TruOfwAitN){zatWoRYSkd = true;}
      if(VfsGaIFjhM == nDQuCIakWn){MhJrVZBAzb = true;}
      else if(nDQuCIakWn == VfsGaIFjhM){JkSjONRUGe = true;}
      if(KnslrcGJVh == SnWZQkqyzd){lpqogNWYXZ = true;}
      else if(SnWZQkqyzd == KnslrcGJVh){JFkMpKhiAm = true;}
      if(mEbAfepSTM == FHXQOQiSBh){bmKirCWHwt = true;}
      else if(FHXQOQiSBh == mEbAfepSTM){rVPGLkiMou = true;}
      if(mIwxsQcYss == ANdrLbQAON){UWtCdWBsor = true;}
      else if(ANdrLbQAON == mIwxsQcYss){ZxgLcqZaFp = true;}
      if(sBGLefzJWC == VKMiJXMXgp){McXrsFZdua = true;}
      if(ppNLZQEsaw == PlkHXgfzgk){pBzUcZWazS = true;}
      if(RmPVZBrusa == YIjonEKeNo){CtDEHOuoLK = true;}
      while(VKMiJXMXgp == sBGLefzJWC){BCStttohfe = true;}
      while(PlkHXgfzgk == PlkHXgfzgk){ioSxlnVLYd = true;}
      while(YIjonEKeNo == YIjonEKeNo){eFeLEMXZjz = true;}
      if(TpjkIutoHq == true){TpjkIutoHq = false;}
      if(ELkKRNcWUe == true){ELkKRNcWUe = false;}
      if(wRTcoNZXtE == true){wRTcoNZXtE = false;}
      if(MhJrVZBAzb == true){MhJrVZBAzb = false;}
      if(lpqogNWYXZ == true){lpqogNWYXZ = false;}
      if(bmKirCWHwt == true){bmKirCWHwt = false;}
      if(UWtCdWBsor == true){UWtCdWBsor = false;}
      if(McXrsFZdua == true){McXrsFZdua = false;}
      if(pBzUcZWazS == true){pBzUcZWazS = false;}
      if(CtDEHOuoLK == true){CtDEHOuoLK = false;}
      if(LcieNyPRom == true){LcieNyPRom = false;}
      if(crLUDTWZYE == true){crLUDTWZYE = false;}
      if(zatWoRYSkd == true){zatWoRYSkd = false;}
      if(JkSjONRUGe == true){JkSjONRUGe = false;}
      if(JFkMpKhiAm == true){JFkMpKhiAm = false;}
      if(rVPGLkiMou == true){rVPGLkiMou = false;}
      if(ZxgLcqZaFp == true){ZxgLcqZaFp = false;}
      if(BCStttohfe == true){BCStttohfe = false;}
      if(ioSxlnVLYd == true){ioSxlnVLYd = false;}
      if(eFeLEMXZjz == true){eFeLEMXZjz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QUBPIJLJXL
{ 
  void LlaChWphxF()
  { 
      bool AZEkMcVhYh = false;
      bool sFRbJXhRlR = false;
      bool rDuNUHNayN = false;
      bool WbcYKBgeUo = false;
      bool mJDgsEyifF = false;
      bool lemGWwepPE = false;
      bool PtjLKRbrRI = false;
      bool qCxxdhAOTP = false;
      bool JdusbroWBN = false;
      bool VpOQYrZrWb = false;
      bool bFQWNihobn = false;
      bool uQVztkzpiR = false;
      bool bFpDmAmgXl = false;
      bool IIrqmENXer = false;
      bool JztMRFUPVn = false;
      bool siIEzWPMjN = false;
      bool uBRpfmYdLp = false;
      bool XaLpIcKclg = false;
      bool fXKirZzmrQ = false;
      bool oAnRxIhEge = false;
      string TsyKMzxDaJ;
      string ICibmrPSjT;
      string TFomRunwoW;
      string ObUPAzWibn;
      string atcABIYWSR;
      string hTMRwODLsR;
      string KBODtghSan;
      string VgpusZbnkI;
      string VRdWnuOCTl;
      string wKoKdYeZFU;
      string ZFuewBcRYV;
      string lOajusXrOz;
      string RezoqKqeRT;
      string uCiQhQVyam;
      string RxdyzgjFPj;
      string BmKGCuGGmk;
      string JxzEZjfJBe;
      string XYQbgUBBoK;
      string akPgRIrbOm;
      string JPLocTxYXP;
      if(TsyKMzxDaJ == ZFuewBcRYV){AZEkMcVhYh = true;}
      else if(ZFuewBcRYV == TsyKMzxDaJ){bFQWNihobn = true;}
      if(ICibmrPSjT == lOajusXrOz){sFRbJXhRlR = true;}
      else if(lOajusXrOz == ICibmrPSjT){uQVztkzpiR = true;}
      if(TFomRunwoW == RezoqKqeRT){rDuNUHNayN = true;}
      else if(RezoqKqeRT == TFomRunwoW){bFpDmAmgXl = true;}
      if(ObUPAzWibn == uCiQhQVyam){WbcYKBgeUo = true;}
      else if(uCiQhQVyam == ObUPAzWibn){IIrqmENXer = true;}
      if(atcABIYWSR == RxdyzgjFPj){mJDgsEyifF = true;}
      else if(RxdyzgjFPj == atcABIYWSR){JztMRFUPVn = true;}
      if(hTMRwODLsR == BmKGCuGGmk){lemGWwepPE = true;}
      else if(BmKGCuGGmk == hTMRwODLsR){siIEzWPMjN = true;}
      if(KBODtghSan == JxzEZjfJBe){PtjLKRbrRI = true;}
      else if(JxzEZjfJBe == KBODtghSan){uBRpfmYdLp = true;}
      if(VgpusZbnkI == XYQbgUBBoK){qCxxdhAOTP = true;}
      if(VRdWnuOCTl == akPgRIrbOm){JdusbroWBN = true;}
      if(wKoKdYeZFU == JPLocTxYXP){VpOQYrZrWb = true;}
      while(XYQbgUBBoK == VgpusZbnkI){XaLpIcKclg = true;}
      while(akPgRIrbOm == akPgRIrbOm){fXKirZzmrQ = true;}
      while(JPLocTxYXP == JPLocTxYXP){oAnRxIhEge = true;}
      if(AZEkMcVhYh == true){AZEkMcVhYh = false;}
      if(sFRbJXhRlR == true){sFRbJXhRlR = false;}
      if(rDuNUHNayN == true){rDuNUHNayN = false;}
      if(WbcYKBgeUo == true){WbcYKBgeUo = false;}
      if(mJDgsEyifF == true){mJDgsEyifF = false;}
      if(lemGWwepPE == true){lemGWwepPE = false;}
      if(PtjLKRbrRI == true){PtjLKRbrRI = false;}
      if(qCxxdhAOTP == true){qCxxdhAOTP = false;}
      if(JdusbroWBN == true){JdusbroWBN = false;}
      if(VpOQYrZrWb == true){VpOQYrZrWb = false;}
      if(bFQWNihobn == true){bFQWNihobn = false;}
      if(uQVztkzpiR == true){uQVztkzpiR = false;}
      if(bFpDmAmgXl == true){bFpDmAmgXl = false;}
      if(IIrqmENXer == true){IIrqmENXer = false;}
      if(JztMRFUPVn == true){JztMRFUPVn = false;}
      if(siIEzWPMjN == true){siIEzWPMjN = false;}
      if(uBRpfmYdLp == true){uBRpfmYdLp = false;}
      if(XaLpIcKclg == true){XaLpIcKclg = false;}
      if(fXKirZzmrQ == true){fXKirZzmrQ = false;}
      if(oAnRxIhEge == true){oAnRxIhEge = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DTABGAKNGN
{ 
  void wHYoSpwdDY()
  { 
      bool qTopbUqjSZ = false;
      bool qmZmNErBLG = false;
      bool jRkDYiUgSA = false;
      bool JTdClNkOPY = false;
      bool OprtSLnstX = false;
      bool klqqCZhlQs = false;
      bool zJRSQXJXAA = false;
      bool ScklrsYpjM = false;
      bool tbltOWblzL = false;
      bool ezyWYEYZdt = false;
      bool CQrtFHAEyJ = false;
      bool BrBDyXXWDs = false;
      bool uWYoVSeONn = false;
      bool TyMoKdjroH = false;
      bool VXnoKlytXA = false;
      bool gCVjzeqDYm = false;
      bool blpKpCeLsU = false;
      bool nehuWXijdi = false;
      bool TkpMGRbcpe = false;
      bool lfSCfjgcyI = false;
      string yEyQZlgbBd;
      string jsQYElzeUI;
      string fpJKQSgZfj;
      string fPjSWAJkIE;
      string ZUAzTBACQc;
      string UmZFJHunzG;
      string EGwRyTJxBD;
      string bNVPiKpJqP;
      string SdJDOspdJP;
      string FVlCzSYiBA;
      string rDBNAHRJVy;
      string RkwjnZxrCF;
      string cemlxFbTQP;
      string lGHqWPcwCH;
      string afIKxUYBOY;
      string VffwWNPOPt;
      string exsFMKcFMX;
      string xBoudwVboc;
      string gaFqtFaLUJ;
      string zOfIcdmCyl;
      if(yEyQZlgbBd == rDBNAHRJVy){qTopbUqjSZ = true;}
      else if(rDBNAHRJVy == yEyQZlgbBd){CQrtFHAEyJ = true;}
      if(jsQYElzeUI == RkwjnZxrCF){qmZmNErBLG = true;}
      else if(RkwjnZxrCF == jsQYElzeUI){BrBDyXXWDs = true;}
      if(fpJKQSgZfj == cemlxFbTQP){jRkDYiUgSA = true;}
      else if(cemlxFbTQP == fpJKQSgZfj){uWYoVSeONn = true;}
      if(fPjSWAJkIE == lGHqWPcwCH){JTdClNkOPY = true;}
      else if(lGHqWPcwCH == fPjSWAJkIE){TyMoKdjroH = true;}
      if(ZUAzTBACQc == afIKxUYBOY){OprtSLnstX = true;}
      else if(afIKxUYBOY == ZUAzTBACQc){VXnoKlytXA = true;}
      if(UmZFJHunzG == VffwWNPOPt){klqqCZhlQs = true;}
      else if(VffwWNPOPt == UmZFJHunzG){gCVjzeqDYm = true;}
      if(EGwRyTJxBD == exsFMKcFMX){zJRSQXJXAA = true;}
      else if(exsFMKcFMX == EGwRyTJxBD){blpKpCeLsU = true;}
      if(bNVPiKpJqP == xBoudwVboc){ScklrsYpjM = true;}
      if(SdJDOspdJP == gaFqtFaLUJ){tbltOWblzL = true;}
      if(FVlCzSYiBA == zOfIcdmCyl){ezyWYEYZdt = true;}
      while(xBoudwVboc == bNVPiKpJqP){nehuWXijdi = true;}
      while(gaFqtFaLUJ == gaFqtFaLUJ){TkpMGRbcpe = true;}
      while(zOfIcdmCyl == zOfIcdmCyl){lfSCfjgcyI = true;}
      if(qTopbUqjSZ == true){qTopbUqjSZ = false;}
      if(qmZmNErBLG == true){qmZmNErBLG = false;}
      if(jRkDYiUgSA == true){jRkDYiUgSA = false;}
      if(JTdClNkOPY == true){JTdClNkOPY = false;}
      if(OprtSLnstX == true){OprtSLnstX = false;}
      if(klqqCZhlQs == true){klqqCZhlQs = false;}
      if(zJRSQXJXAA == true){zJRSQXJXAA = false;}
      if(ScklrsYpjM == true){ScklrsYpjM = false;}
      if(tbltOWblzL == true){tbltOWblzL = false;}
      if(ezyWYEYZdt == true){ezyWYEYZdt = false;}
      if(CQrtFHAEyJ == true){CQrtFHAEyJ = false;}
      if(BrBDyXXWDs == true){BrBDyXXWDs = false;}
      if(uWYoVSeONn == true){uWYoVSeONn = false;}
      if(TyMoKdjroH == true){TyMoKdjroH = false;}
      if(VXnoKlytXA == true){VXnoKlytXA = false;}
      if(gCVjzeqDYm == true){gCVjzeqDYm = false;}
      if(blpKpCeLsU == true){blpKpCeLsU = false;}
      if(nehuWXijdi == true){nehuWXijdi = false;}
      if(TkpMGRbcpe == true){TkpMGRbcpe = false;}
      if(lfSCfjgcyI == true){lfSCfjgcyI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SHZQCBTLUI
{ 
  void YLFbrkJTIm()
  { 
      bool OQaqPsVsyY = false;
      bool yatnpVgOaS = false;
      bool DOQCtYsHRP = false;
      bool hATZtEQzGf = false;
      bool RQBOumYlUA = false;
      bool OcHapzAYwp = false;
      bool ANbhpyHUsk = false;
      bool FhIuAlMBGG = false;
      bool AGlQAFrsiu = false;
      bool EqCPfEoTXx = false;
      bool fDsjugTMZE = false;
      bool lySposbLgk = false;
      bool FNkygNerOw = false;
      bool xscJaxNCUS = false;
      bool kfTeIFYMZl = false;
      bool bclmqRRtCh = false;
      bool CbxcGrUIOr = false;
      bool fYpOzZsGoa = false;
      bool oRRcTTHpRI = false;
      bool eSzmQJhpXQ = false;
      string rxHYcGetkR;
      string SscOdnMeus;
      string CmQfVKHgdB;
      string AXCcVllzdg;
      string CtoLurDiaE;
      string oPeerFfaDS;
      string yxPiRupzrq;
      string BMBNqHyFkd;
      string GXGZLjsWkP;
      string FKwKiEeFsA;
      string BDpiCcOQmj;
      string HaoYLZqpWh;
      string BSOblqPpem;
      string csLkcDgWWi;
      string UbjAIdUXzI;
      string NohLyhoOwL;
      string MXkAHOPEwI;
      string rTSnMYiNTI;
      string VqqWZnKAmt;
      string OSaLVwqist;
      if(rxHYcGetkR == BDpiCcOQmj){OQaqPsVsyY = true;}
      else if(BDpiCcOQmj == rxHYcGetkR){fDsjugTMZE = true;}
      if(SscOdnMeus == HaoYLZqpWh){yatnpVgOaS = true;}
      else if(HaoYLZqpWh == SscOdnMeus){lySposbLgk = true;}
      if(CmQfVKHgdB == BSOblqPpem){DOQCtYsHRP = true;}
      else if(BSOblqPpem == CmQfVKHgdB){FNkygNerOw = true;}
      if(AXCcVllzdg == csLkcDgWWi){hATZtEQzGf = true;}
      else if(csLkcDgWWi == AXCcVllzdg){xscJaxNCUS = true;}
      if(CtoLurDiaE == UbjAIdUXzI){RQBOumYlUA = true;}
      else if(UbjAIdUXzI == CtoLurDiaE){kfTeIFYMZl = true;}
      if(oPeerFfaDS == NohLyhoOwL){OcHapzAYwp = true;}
      else if(NohLyhoOwL == oPeerFfaDS){bclmqRRtCh = true;}
      if(yxPiRupzrq == MXkAHOPEwI){ANbhpyHUsk = true;}
      else if(MXkAHOPEwI == yxPiRupzrq){CbxcGrUIOr = true;}
      if(BMBNqHyFkd == rTSnMYiNTI){FhIuAlMBGG = true;}
      if(GXGZLjsWkP == VqqWZnKAmt){AGlQAFrsiu = true;}
      if(FKwKiEeFsA == OSaLVwqist){EqCPfEoTXx = true;}
      while(rTSnMYiNTI == BMBNqHyFkd){fYpOzZsGoa = true;}
      while(VqqWZnKAmt == VqqWZnKAmt){oRRcTTHpRI = true;}
      while(OSaLVwqist == OSaLVwqist){eSzmQJhpXQ = true;}
      if(OQaqPsVsyY == true){OQaqPsVsyY = false;}
      if(yatnpVgOaS == true){yatnpVgOaS = false;}
      if(DOQCtYsHRP == true){DOQCtYsHRP = false;}
      if(hATZtEQzGf == true){hATZtEQzGf = false;}
      if(RQBOumYlUA == true){RQBOumYlUA = false;}
      if(OcHapzAYwp == true){OcHapzAYwp = false;}
      if(ANbhpyHUsk == true){ANbhpyHUsk = false;}
      if(FhIuAlMBGG == true){FhIuAlMBGG = false;}
      if(AGlQAFrsiu == true){AGlQAFrsiu = false;}
      if(EqCPfEoTXx == true){EqCPfEoTXx = false;}
      if(fDsjugTMZE == true){fDsjugTMZE = false;}
      if(lySposbLgk == true){lySposbLgk = false;}
      if(FNkygNerOw == true){FNkygNerOw = false;}
      if(xscJaxNCUS == true){xscJaxNCUS = false;}
      if(kfTeIFYMZl == true){kfTeIFYMZl = false;}
      if(bclmqRRtCh == true){bclmqRRtCh = false;}
      if(CbxcGrUIOr == true){CbxcGrUIOr = false;}
      if(fYpOzZsGoa == true){fYpOzZsGoa = false;}
      if(oRRcTTHpRI == true){oRRcTTHpRI = false;}
      if(eSzmQJhpXQ == true){eSzmQJhpXQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QFPFJSSVIF
{ 
  void reazCtXIae()
  { 
      bool YOpBDhJEid = false;
      bool TWqdKTVWjW = false;
      bool BxWrEJztoY = false;
      bool nHzOyZGeyE = false;
      bool cfxTykOdrV = false;
      bool xeBdqbqxxt = false;
      bool hOOJpLXbfj = false;
      bool NpXftVzDtS = false;
      bool tVCTbIGUWW = false;
      bool iIhDVNQmcl = false;
      bool NlxHHHHVjV = false;
      bool bmDdAMMuhg = false;
      bool FCEIBEITKf = false;
      bool FMZHFHXuLT = false;
      bool BjjBuuFDDI = false;
      bool BUFwfKgoFC = false;
      bool znKEPPNuYs = false;
      bool oQiOfVRCNG = false;
      bool zEPCdkHdbt = false;
      bool ZhoJUNYfle = false;
      string IMWMdPlOmF;
      string OrnxrJbQNr;
      string eQZdWVBgpe;
      string VbjPabPiNC;
      string KecaZYHIsQ;
      string HAFeCrlroH;
      string NEXIjmIXWQ;
      string trCKgAmzEx;
      string OrmYWUVFMi;
      string pWsGoGGUql;
      string QilcWPyuxC;
      string ElmrrVRzkO;
      string RCKmaXyuVa;
      string sSISgfFKCG;
      string VaIKeDiBOT;
      string oGibUOXzCz;
      string QCYVIkdULM;
      string PHxerptAIX;
      string nwgClljgxS;
      string HGHCoSBlBs;
      if(IMWMdPlOmF == QilcWPyuxC){YOpBDhJEid = true;}
      else if(QilcWPyuxC == IMWMdPlOmF){NlxHHHHVjV = true;}
      if(OrnxrJbQNr == ElmrrVRzkO){TWqdKTVWjW = true;}
      else if(ElmrrVRzkO == OrnxrJbQNr){bmDdAMMuhg = true;}
      if(eQZdWVBgpe == RCKmaXyuVa){BxWrEJztoY = true;}
      else if(RCKmaXyuVa == eQZdWVBgpe){FCEIBEITKf = true;}
      if(VbjPabPiNC == sSISgfFKCG){nHzOyZGeyE = true;}
      else if(sSISgfFKCG == VbjPabPiNC){FMZHFHXuLT = true;}
      if(KecaZYHIsQ == VaIKeDiBOT){cfxTykOdrV = true;}
      else if(VaIKeDiBOT == KecaZYHIsQ){BjjBuuFDDI = true;}
      if(HAFeCrlroH == oGibUOXzCz){xeBdqbqxxt = true;}
      else if(oGibUOXzCz == HAFeCrlroH){BUFwfKgoFC = true;}
      if(NEXIjmIXWQ == QCYVIkdULM){hOOJpLXbfj = true;}
      else if(QCYVIkdULM == NEXIjmIXWQ){znKEPPNuYs = true;}
      if(trCKgAmzEx == PHxerptAIX){NpXftVzDtS = true;}
      if(OrmYWUVFMi == nwgClljgxS){tVCTbIGUWW = true;}
      if(pWsGoGGUql == HGHCoSBlBs){iIhDVNQmcl = true;}
      while(PHxerptAIX == trCKgAmzEx){oQiOfVRCNG = true;}
      while(nwgClljgxS == nwgClljgxS){zEPCdkHdbt = true;}
      while(HGHCoSBlBs == HGHCoSBlBs){ZhoJUNYfle = true;}
      if(YOpBDhJEid == true){YOpBDhJEid = false;}
      if(TWqdKTVWjW == true){TWqdKTVWjW = false;}
      if(BxWrEJztoY == true){BxWrEJztoY = false;}
      if(nHzOyZGeyE == true){nHzOyZGeyE = false;}
      if(cfxTykOdrV == true){cfxTykOdrV = false;}
      if(xeBdqbqxxt == true){xeBdqbqxxt = false;}
      if(hOOJpLXbfj == true){hOOJpLXbfj = false;}
      if(NpXftVzDtS == true){NpXftVzDtS = false;}
      if(tVCTbIGUWW == true){tVCTbIGUWW = false;}
      if(iIhDVNQmcl == true){iIhDVNQmcl = false;}
      if(NlxHHHHVjV == true){NlxHHHHVjV = false;}
      if(bmDdAMMuhg == true){bmDdAMMuhg = false;}
      if(FCEIBEITKf == true){FCEIBEITKf = false;}
      if(FMZHFHXuLT == true){FMZHFHXuLT = false;}
      if(BjjBuuFDDI == true){BjjBuuFDDI = false;}
      if(BUFwfKgoFC == true){BUFwfKgoFC = false;}
      if(znKEPPNuYs == true){znKEPPNuYs = false;}
      if(oQiOfVRCNG == true){oQiOfVRCNG = false;}
      if(zEPCdkHdbt == true){zEPCdkHdbt = false;}
      if(ZhoJUNYfle == true){ZhoJUNYfle = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EDFPDXTZUH
{ 
  void nXxmXBXcQy()
  { 
      bool FoEKmpTbNb = false;
      bool OZxbrJwjQQ = false;
      bool aXpfjjQMbg = false;
      bool yfMoEUsBSC = false;
      bool hxfcXhXjfb = false;
      bool fQdSrgzXaD = false;
      bool ZHzgchNIWW = false;
      bool QtQNidrMIC = false;
      bool MfgAAgCswX = false;
      bool UUaPcZxyAo = false;
      bool TOMILlzPjO = false;
      bool nlPYptazgw = false;
      bool fUMzCwSAkJ = false;
      bool gtMShxSXnO = false;
      bool glIgiiECYi = false;
      bool yxRDJzCNGP = false;
      bool SzeahtYxps = false;
      bool aCKROtLeQS = false;
      bool qQaCKicIIh = false;
      bool tFRYwhGhzn = false;
      string aYJQSwWNRa;
      string RQzeQrwikz;
      string UtFlYxGbel;
      string WsIXQajJtO;
      string pyeVobxwJG;
      string PMMgufTXqV;
      string ptieEXHbNo;
      string HmnWJVPixw;
      string zJSYWJgbbk;
      string HMRYXuPzRQ;
      string UViTycXoXZ;
      string QjatVfbKIt;
      string pezchuUPyN;
      string GPRfbdayZt;
      string kqECTXkWCr;
      string TEqWaoVFlC;
      string HIpJSASuSr;
      string EcjOIzawIJ;
      string KxMGsBYtEl;
      string HUMXlbAtjT;
      if(aYJQSwWNRa == UViTycXoXZ){FoEKmpTbNb = true;}
      else if(UViTycXoXZ == aYJQSwWNRa){TOMILlzPjO = true;}
      if(RQzeQrwikz == QjatVfbKIt){OZxbrJwjQQ = true;}
      else if(QjatVfbKIt == RQzeQrwikz){nlPYptazgw = true;}
      if(UtFlYxGbel == pezchuUPyN){aXpfjjQMbg = true;}
      else if(pezchuUPyN == UtFlYxGbel){fUMzCwSAkJ = true;}
      if(WsIXQajJtO == GPRfbdayZt){yfMoEUsBSC = true;}
      else if(GPRfbdayZt == WsIXQajJtO){gtMShxSXnO = true;}
      if(pyeVobxwJG == kqECTXkWCr){hxfcXhXjfb = true;}
      else if(kqECTXkWCr == pyeVobxwJG){glIgiiECYi = true;}
      if(PMMgufTXqV == TEqWaoVFlC){fQdSrgzXaD = true;}
      else if(TEqWaoVFlC == PMMgufTXqV){yxRDJzCNGP = true;}
      if(ptieEXHbNo == HIpJSASuSr){ZHzgchNIWW = true;}
      else if(HIpJSASuSr == ptieEXHbNo){SzeahtYxps = true;}
      if(HmnWJVPixw == EcjOIzawIJ){QtQNidrMIC = true;}
      if(zJSYWJgbbk == KxMGsBYtEl){MfgAAgCswX = true;}
      if(HMRYXuPzRQ == HUMXlbAtjT){UUaPcZxyAo = true;}
      while(EcjOIzawIJ == HmnWJVPixw){aCKROtLeQS = true;}
      while(KxMGsBYtEl == KxMGsBYtEl){qQaCKicIIh = true;}
      while(HUMXlbAtjT == HUMXlbAtjT){tFRYwhGhzn = true;}
      if(FoEKmpTbNb == true){FoEKmpTbNb = false;}
      if(OZxbrJwjQQ == true){OZxbrJwjQQ = false;}
      if(aXpfjjQMbg == true){aXpfjjQMbg = false;}
      if(yfMoEUsBSC == true){yfMoEUsBSC = false;}
      if(hxfcXhXjfb == true){hxfcXhXjfb = false;}
      if(fQdSrgzXaD == true){fQdSrgzXaD = false;}
      if(ZHzgchNIWW == true){ZHzgchNIWW = false;}
      if(QtQNidrMIC == true){QtQNidrMIC = false;}
      if(MfgAAgCswX == true){MfgAAgCswX = false;}
      if(UUaPcZxyAo == true){UUaPcZxyAo = false;}
      if(TOMILlzPjO == true){TOMILlzPjO = false;}
      if(nlPYptazgw == true){nlPYptazgw = false;}
      if(fUMzCwSAkJ == true){fUMzCwSAkJ = false;}
      if(gtMShxSXnO == true){gtMShxSXnO = false;}
      if(glIgiiECYi == true){glIgiiECYi = false;}
      if(yxRDJzCNGP == true){yxRDJzCNGP = false;}
      if(SzeahtYxps == true){SzeahtYxps = false;}
      if(aCKROtLeQS == true){aCKROtLeQS = false;}
      if(qQaCKicIIh == true){qQaCKicIIh = false;}
      if(tFRYwhGhzn == true){tFRYwhGhzn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KHDXFZEHRL
{ 
  void baHupOwBcP()
  { 
      bool NwtqzbmcBg = false;
      bool drrKNyfXae = false;
      bool gZLCMSuIym = false;
      bool amMoEegKRD = false;
      bool OdXXxHiLJL = false;
      bool NCxxTmkqef = false;
      bool fddOFpfCGh = false;
      bool RVGXPDBnFE = false;
      bool ijymZYpOVR = false;
      bool toekXkzJRG = false;
      bool bgFasqwZgP = false;
      bool kxlZwweqof = false;
      bool AQfnncpUze = false;
      bool ErVzbCeXsh = false;
      bool cQhFfxCEob = false;
      bool FHUlFKBQmQ = false;
      bool AbZCLsamyt = false;
      bool QHyGDiiZIq = false;
      bool ZleXYwXjsQ = false;
      bool ANjchBqesl = false;
      string DfAYhDtHAj;
      string BXPQmDdWgT;
      string eCTNeSdUWy;
      string KWXzUjBMQX;
      string jwfRcgzIMg;
      string ptIOkBLlsP;
      string ZZjBYezPwD;
      string CRGIihVCjy;
      string SVIpIPOJOS;
      string XOSAyKMdry;
      string gjLIdXnzeD;
      string eJBuwEQsrJ;
      string QpBdfnyFKY;
      string VrJlRiJzMj;
      string bOdKmNMXEs;
      string sSkihpihco;
      string ZhoGhmAPdM;
      string tRfIgBLppR;
      string baBQeCGZEa;
      string AbVXKtZXcA;
      if(DfAYhDtHAj == gjLIdXnzeD){NwtqzbmcBg = true;}
      else if(gjLIdXnzeD == DfAYhDtHAj){bgFasqwZgP = true;}
      if(BXPQmDdWgT == eJBuwEQsrJ){drrKNyfXae = true;}
      else if(eJBuwEQsrJ == BXPQmDdWgT){kxlZwweqof = true;}
      if(eCTNeSdUWy == QpBdfnyFKY){gZLCMSuIym = true;}
      else if(QpBdfnyFKY == eCTNeSdUWy){AQfnncpUze = true;}
      if(KWXzUjBMQX == VrJlRiJzMj){amMoEegKRD = true;}
      else if(VrJlRiJzMj == KWXzUjBMQX){ErVzbCeXsh = true;}
      if(jwfRcgzIMg == bOdKmNMXEs){OdXXxHiLJL = true;}
      else if(bOdKmNMXEs == jwfRcgzIMg){cQhFfxCEob = true;}
      if(ptIOkBLlsP == sSkihpihco){NCxxTmkqef = true;}
      else if(sSkihpihco == ptIOkBLlsP){FHUlFKBQmQ = true;}
      if(ZZjBYezPwD == ZhoGhmAPdM){fddOFpfCGh = true;}
      else if(ZhoGhmAPdM == ZZjBYezPwD){AbZCLsamyt = true;}
      if(CRGIihVCjy == tRfIgBLppR){RVGXPDBnFE = true;}
      if(SVIpIPOJOS == baBQeCGZEa){ijymZYpOVR = true;}
      if(XOSAyKMdry == AbVXKtZXcA){toekXkzJRG = true;}
      while(tRfIgBLppR == CRGIihVCjy){QHyGDiiZIq = true;}
      while(baBQeCGZEa == baBQeCGZEa){ZleXYwXjsQ = true;}
      while(AbVXKtZXcA == AbVXKtZXcA){ANjchBqesl = true;}
      if(NwtqzbmcBg == true){NwtqzbmcBg = false;}
      if(drrKNyfXae == true){drrKNyfXae = false;}
      if(gZLCMSuIym == true){gZLCMSuIym = false;}
      if(amMoEegKRD == true){amMoEegKRD = false;}
      if(OdXXxHiLJL == true){OdXXxHiLJL = false;}
      if(NCxxTmkqef == true){NCxxTmkqef = false;}
      if(fddOFpfCGh == true){fddOFpfCGh = false;}
      if(RVGXPDBnFE == true){RVGXPDBnFE = false;}
      if(ijymZYpOVR == true){ijymZYpOVR = false;}
      if(toekXkzJRG == true){toekXkzJRG = false;}
      if(bgFasqwZgP == true){bgFasqwZgP = false;}
      if(kxlZwweqof == true){kxlZwweqof = false;}
      if(AQfnncpUze == true){AQfnncpUze = false;}
      if(ErVzbCeXsh == true){ErVzbCeXsh = false;}
      if(cQhFfxCEob == true){cQhFfxCEob = false;}
      if(FHUlFKBQmQ == true){FHUlFKBQmQ = false;}
      if(AbZCLsamyt == true){AbZCLsamyt = false;}
      if(QHyGDiiZIq == true){QHyGDiiZIq = false;}
      if(ZleXYwXjsQ == true){ZleXYwXjsQ = false;}
      if(ANjchBqesl == true){ANjchBqesl = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ADUYBRXLSR
{ 
  void nuelgDPQEE()
  { 
      bool gcpLClEjlm = false;
      bool PrfEhUeTIP = false;
      bool RfxicBQMgO = false;
      bool WqrisLpOfH = false;
      bool ezshiCuzTe = false;
      bool QmNsUBeHiB = false;
      bool VqCWQXAwLV = false;
      bool ePnBjSlPza = false;
      bool GQrByicyOr = false;
      bool ghfdlmwWQK = false;
      bool BynyAAzMIB = false;
      bool PPHHtFzBYh = false;
      bool uBhSnwjJOw = false;
      bool VOyoYIbtBy = false;
      bool iPoMcRaXjg = false;
      bool nuyiOAPVJD = false;
      bool PoyMeAQSil = false;
      bool cEAZyfLUpW = false;
      bool VLZgHaQOZY = false;
      bool rGjxhtRchc = false;
      string FnLiXwOswN;
      string CMwsArWHgJ;
      string iAQLeSAcaB;
      string lotZtCDkAg;
      string xYwhRkZKuu;
      string rVPPNFWADE;
      string ulStaMSWzy;
      string nlGzMKDist;
      string rogeJQYdMo;
      string fDgwlLGCGF;
      string RDLxHGPPDD;
      string BmHJMGEmkK;
      string mnJJSkEKNA;
      string DAbmbnciXC;
      string AJZImrwXje;
      string GTRwRdsCeV;
      string BFCCzBLTgX;
      string cUOhJdOBVZ;
      string ngahsjifVy;
      string taEBAuHWIl;
      if(FnLiXwOswN == RDLxHGPPDD){gcpLClEjlm = true;}
      else if(RDLxHGPPDD == FnLiXwOswN){BynyAAzMIB = true;}
      if(CMwsArWHgJ == BmHJMGEmkK){PrfEhUeTIP = true;}
      else if(BmHJMGEmkK == CMwsArWHgJ){PPHHtFzBYh = true;}
      if(iAQLeSAcaB == mnJJSkEKNA){RfxicBQMgO = true;}
      else if(mnJJSkEKNA == iAQLeSAcaB){uBhSnwjJOw = true;}
      if(lotZtCDkAg == DAbmbnciXC){WqrisLpOfH = true;}
      else if(DAbmbnciXC == lotZtCDkAg){VOyoYIbtBy = true;}
      if(xYwhRkZKuu == AJZImrwXje){ezshiCuzTe = true;}
      else if(AJZImrwXje == xYwhRkZKuu){iPoMcRaXjg = true;}
      if(rVPPNFWADE == GTRwRdsCeV){QmNsUBeHiB = true;}
      else if(GTRwRdsCeV == rVPPNFWADE){nuyiOAPVJD = true;}
      if(ulStaMSWzy == BFCCzBLTgX){VqCWQXAwLV = true;}
      else if(BFCCzBLTgX == ulStaMSWzy){PoyMeAQSil = true;}
      if(nlGzMKDist == cUOhJdOBVZ){ePnBjSlPza = true;}
      if(rogeJQYdMo == ngahsjifVy){GQrByicyOr = true;}
      if(fDgwlLGCGF == taEBAuHWIl){ghfdlmwWQK = true;}
      while(cUOhJdOBVZ == nlGzMKDist){cEAZyfLUpW = true;}
      while(ngahsjifVy == ngahsjifVy){VLZgHaQOZY = true;}
      while(taEBAuHWIl == taEBAuHWIl){rGjxhtRchc = true;}
      if(gcpLClEjlm == true){gcpLClEjlm = false;}
      if(PrfEhUeTIP == true){PrfEhUeTIP = false;}
      if(RfxicBQMgO == true){RfxicBQMgO = false;}
      if(WqrisLpOfH == true){WqrisLpOfH = false;}
      if(ezshiCuzTe == true){ezshiCuzTe = false;}
      if(QmNsUBeHiB == true){QmNsUBeHiB = false;}
      if(VqCWQXAwLV == true){VqCWQXAwLV = false;}
      if(ePnBjSlPza == true){ePnBjSlPza = false;}
      if(GQrByicyOr == true){GQrByicyOr = false;}
      if(ghfdlmwWQK == true){ghfdlmwWQK = false;}
      if(BynyAAzMIB == true){BynyAAzMIB = false;}
      if(PPHHtFzBYh == true){PPHHtFzBYh = false;}
      if(uBhSnwjJOw == true){uBhSnwjJOw = false;}
      if(VOyoYIbtBy == true){VOyoYIbtBy = false;}
      if(iPoMcRaXjg == true){iPoMcRaXjg = false;}
      if(nuyiOAPVJD == true){nuyiOAPVJD = false;}
      if(PoyMeAQSil == true){PoyMeAQSil = false;}
      if(cEAZyfLUpW == true){cEAZyfLUpW = false;}
      if(VLZgHaQOZY == true){VLZgHaQOZY = false;}
      if(rGjxhtRchc == true){rGjxhtRchc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SRWSUQBCLQ
{ 
  void MIFRtQYjnX()
  { 
      bool ghXJDOclLa = false;
      bool kDwgIbMJqA = false;
      bool pGEBhwnUWd = false;
      bool EokKHKmTTu = false;
      bool dclFWbJruI = false;
      bool pfpXVadWbH = false;
      bool aIPhVwSkZx = false;
      bool ospuWMTydF = false;
      bool mwZNmYyNnf = false;
      bool gPrLUVNEiZ = false;
      bool JXSxTxpuBJ = false;
      bool GerWUdhCtx = false;
      bool yExsQYAdaA = false;
      bool yPOEuPdbso = false;
      bool iDMeZBXUqp = false;
      bool mLJkbEZYgu = false;
      bool wLTAKpPnbF = false;
      bool AGIEOurRTP = false;
      bool AfmjMmGdbp = false;
      bool oEuyrWiLTi = false;
      string qOmmyZAzTj;
      string tdxCCwNTCS;
      string SsDPfsOyCj;
      string RztGesdNlr;
      string pcJoeztfFh;
      string fbTSHyKXGF;
      string KCxjStipWI;
      string SzhKboiOkg;
      string yirTEoeazg;
      string hFHfgmeZWj;
      string PEJwOftJqR;
      string OwlZHbJdfc;
      string PHqoaMrCcn;
      string JJqBQsZuFX;
      string uVjgRlTgXF;
      string dBssUINTVB;
      string mqfCIFEkJN;
      string BOYHhsWJWn;
      string sKdWKHPygH;
      string fFZDkDohYM;
      if(qOmmyZAzTj == PEJwOftJqR){ghXJDOclLa = true;}
      else if(PEJwOftJqR == qOmmyZAzTj){JXSxTxpuBJ = true;}
      if(tdxCCwNTCS == OwlZHbJdfc){kDwgIbMJqA = true;}
      else if(OwlZHbJdfc == tdxCCwNTCS){GerWUdhCtx = true;}
      if(SsDPfsOyCj == PHqoaMrCcn){pGEBhwnUWd = true;}
      else if(PHqoaMrCcn == SsDPfsOyCj){yExsQYAdaA = true;}
      if(RztGesdNlr == JJqBQsZuFX){EokKHKmTTu = true;}
      else if(JJqBQsZuFX == RztGesdNlr){yPOEuPdbso = true;}
      if(pcJoeztfFh == uVjgRlTgXF){dclFWbJruI = true;}
      else if(uVjgRlTgXF == pcJoeztfFh){iDMeZBXUqp = true;}
      if(fbTSHyKXGF == dBssUINTVB){pfpXVadWbH = true;}
      else if(dBssUINTVB == fbTSHyKXGF){mLJkbEZYgu = true;}
      if(KCxjStipWI == mqfCIFEkJN){aIPhVwSkZx = true;}
      else if(mqfCIFEkJN == KCxjStipWI){wLTAKpPnbF = true;}
      if(SzhKboiOkg == BOYHhsWJWn){ospuWMTydF = true;}
      if(yirTEoeazg == sKdWKHPygH){mwZNmYyNnf = true;}
      if(hFHfgmeZWj == fFZDkDohYM){gPrLUVNEiZ = true;}
      while(BOYHhsWJWn == SzhKboiOkg){AGIEOurRTP = true;}
      while(sKdWKHPygH == sKdWKHPygH){AfmjMmGdbp = true;}
      while(fFZDkDohYM == fFZDkDohYM){oEuyrWiLTi = true;}
      if(ghXJDOclLa == true){ghXJDOclLa = false;}
      if(kDwgIbMJqA == true){kDwgIbMJqA = false;}
      if(pGEBhwnUWd == true){pGEBhwnUWd = false;}
      if(EokKHKmTTu == true){EokKHKmTTu = false;}
      if(dclFWbJruI == true){dclFWbJruI = false;}
      if(pfpXVadWbH == true){pfpXVadWbH = false;}
      if(aIPhVwSkZx == true){aIPhVwSkZx = false;}
      if(ospuWMTydF == true){ospuWMTydF = false;}
      if(mwZNmYyNnf == true){mwZNmYyNnf = false;}
      if(gPrLUVNEiZ == true){gPrLUVNEiZ = false;}
      if(JXSxTxpuBJ == true){JXSxTxpuBJ = false;}
      if(GerWUdhCtx == true){GerWUdhCtx = false;}
      if(yExsQYAdaA == true){yExsQYAdaA = false;}
      if(yPOEuPdbso == true){yPOEuPdbso = false;}
      if(iDMeZBXUqp == true){iDMeZBXUqp = false;}
      if(mLJkbEZYgu == true){mLJkbEZYgu = false;}
      if(wLTAKpPnbF == true){wLTAKpPnbF = false;}
      if(AGIEOurRTP == true){AGIEOurRTP = false;}
      if(AfmjMmGdbp == true){AfmjMmGdbp = false;}
      if(oEuyrWiLTi == true){oEuyrWiLTi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DLYLZYTQPU
{ 
  void tczYptDddW()
  { 
      bool xNTUpCOyEW = false;
      bool CTNrgHpFCT = false;
      bool CmYaSduskT = false;
      bool cbPrpkpUdW = false;
      bool ApcgkshKRy = false;
      bool ettnuKorBL = false;
      bool HUMjtaDVBu = false;
      bool QUaeTMJqoY = false;
      bool wkamohgbwf = false;
      bool wxSSPedrmK = false;
      bool pMIWgJSOTi = false;
      bool AxCcPVsYuL = false;
      bool sbYUySCIPn = false;
      bool hQEeiibEGe = false;
      bool ORHNfzupCs = false;
      bool lExaQOnnXN = false;
      bool GJzfAisXsf = false;
      bool toSPwbTKoz = false;
      bool XMGVCdTcNU = false;
      bool unfrYFRbNw = false;
      string eFkwdSWOoe;
      string GNLCfhgObt;
      string AcBugfIVpu;
      string UUozMPQakB;
      string jCwsnuTYBn;
      string OykMDBDpOy;
      string JBYsKkYLpQ;
      string RFcaJyfIHk;
      string MiTBfVibwK;
      string LZMlqicChC;
      string WHikWJVJDE;
      string UtickagWNo;
      string CwPeEqrpja;
      string VdaEOFAieV;
      string AYYqoQqiBQ;
      string BGosAZqDff;
      string QeYbiqTdOf;
      string ZyrqocXXpm;
      string zJoTZOngEn;
      string NwCyFUAwIx;
      if(eFkwdSWOoe == WHikWJVJDE){xNTUpCOyEW = true;}
      else if(WHikWJVJDE == eFkwdSWOoe){pMIWgJSOTi = true;}
      if(GNLCfhgObt == UtickagWNo){CTNrgHpFCT = true;}
      else if(UtickagWNo == GNLCfhgObt){AxCcPVsYuL = true;}
      if(AcBugfIVpu == CwPeEqrpja){CmYaSduskT = true;}
      else if(CwPeEqrpja == AcBugfIVpu){sbYUySCIPn = true;}
      if(UUozMPQakB == VdaEOFAieV){cbPrpkpUdW = true;}
      else if(VdaEOFAieV == UUozMPQakB){hQEeiibEGe = true;}
      if(jCwsnuTYBn == AYYqoQqiBQ){ApcgkshKRy = true;}
      else if(AYYqoQqiBQ == jCwsnuTYBn){ORHNfzupCs = true;}
      if(OykMDBDpOy == BGosAZqDff){ettnuKorBL = true;}
      else if(BGosAZqDff == OykMDBDpOy){lExaQOnnXN = true;}
      if(JBYsKkYLpQ == QeYbiqTdOf){HUMjtaDVBu = true;}
      else if(QeYbiqTdOf == JBYsKkYLpQ){GJzfAisXsf = true;}
      if(RFcaJyfIHk == ZyrqocXXpm){QUaeTMJqoY = true;}
      if(MiTBfVibwK == zJoTZOngEn){wkamohgbwf = true;}
      if(LZMlqicChC == NwCyFUAwIx){wxSSPedrmK = true;}
      while(ZyrqocXXpm == RFcaJyfIHk){toSPwbTKoz = true;}
      while(zJoTZOngEn == zJoTZOngEn){XMGVCdTcNU = true;}
      while(NwCyFUAwIx == NwCyFUAwIx){unfrYFRbNw = true;}
      if(xNTUpCOyEW == true){xNTUpCOyEW = false;}
      if(CTNrgHpFCT == true){CTNrgHpFCT = false;}
      if(CmYaSduskT == true){CmYaSduskT = false;}
      if(cbPrpkpUdW == true){cbPrpkpUdW = false;}
      if(ApcgkshKRy == true){ApcgkshKRy = false;}
      if(ettnuKorBL == true){ettnuKorBL = false;}
      if(HUMjtaDVBu == true){HUMjtaDVBu = false;}
      if(QUaeTMJqoY == true){QUaeTMJqoY = false;}
      if(wkamohgbwf == true){wkamohgbwf = false;}
      if(wxSSPedrmK == true){wxSSPedrmK = false;}
      if(pMIWgJSOTi == true){pMIWgJSOTi = false;}
      if(AxCcPVsYuL == true){AxCcPVsYuL = false;}
      if(sbYUySCIPn == true){sbYUySCIPn = false;}
      if(hQEeiibEGe == true){hQEeiibEGe = false;}
      if(ORHNfzupCs == true){ORHNfzupCs = false;}
      if(lExaQOnnXN == true){lExaQOnnXN = false;}
      if(GJzfAisXsf == true){GJzfAisXsf = false;}
      if(toSPwbTKoz == true){toSPwbTKoz = false;}
      if(XMGVCdTcNU == true){XMGVCdTcNU = false;}
      if(unfrYFRbNw == true){unfrYFRbNw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HAMCKQTRZF
{ 
  void TdqzghYWqr()
  { 
      bool ZaWBpKcLNx = false;
      bool toyfPMzKCW = false;
      bool BxCqmEWNSk = false;
      bool IMesfIJNSw = false;
      bool CwbFTSfgsJ = false;
      bool nOOgebXeNk = false;
      bool mPsbngKFzH = false;
      bool ClqGTEVxXU = false;
      bool XpUIxtyxNE = false;
      bool jezhImDzQm = false;
      bool IahkGCWHMa = false;
      bool GaHsMxtjet = false;
      bool iMqtXXmJyg = false;
      bool AVgjMxHiCt = false;
      bool WpjmyJWFUK = false;
      bool rTUVEuimNh = false;
      bool PNJfZjtWgP = false;
      bool oKgkXrbEuN = false;
      bool AgomngAKfQ = false;
      bool OUVTdIfEMN = false;
      string KXnEIYEXNn;
      string EdSJtaMaXD;
      string xgLNrnzNTu;
      string kjimksshrh;
      string VjUSZZtKHw;
      string fJqtwcLZLU;
      string YpDRDTufXB;
      string heiphMLsmS;
      string soGXPbsnpH;
      string gSVXIefCOF;
      string eNmGOBZAMi;
      string RSmsajjfCb;
      string aGpPTOILgC;
      string fIdhmfBzIO;
      string CrbGpNfcwX;
      string uAAmeYXXlz;
      string rNTJCJnFGu;
      string hjWnZheiYG;
      string ifOIHfIpCO;
      string HdNJVnYREf;
      if(KXnEIYEXNn == eNmGOBZAMi){ZaWBpKcLNx = true;}
      else if(eNmGOBZAMi == KXnEIYEXNn){IahkGCWHMa = true;}
      if(EdSJtaMaXD == RSmsajjfCb){toyfPMzKCW = true;}
      else if(RSmsajjfCb == EdSJtaMaXD){GaHsMxtjet = true;}
      if(xgLNrnzNTu == aGpPTOILgC){BxCqmEWNSk = true;}
      else if(aGpPTOILgC == xgLNrnzNTu){iMqtXXmJyg = true;}
      if(kjimksshrh == fIdhmfBzIO){IMesfIJNSw = true;}
      else if(fIdhmfBzIO == kjimksshrh){AVgjMxHiCt = true;}
      if(VjUSZZtKHw == CrbGpNfcwX){CwbFTSfgsJ = true;}
      else if(CrbGpNfcwX == VjUSZZtKHw){WpjmyJWFUK = true;}
      if(fJqtwcLZLU == uAAmeYXXlz){nOOgebXeNk = true;}
      else if(uAAmeYXXlz == fJqtwcLZLU){rTUVEuimNh = true;}
      if(YpDRDTufXB == rNTJCJnFGu){mPsbngKFzH = true;}
      else if(rNTJCJnFGu == YpDRDTufXB){PNJfZjtWgP = true;}
      if(heiphMLsmS == hjWnZheiYG){ClqGTEVxXU = true;}
      if(soGXPbsnpH == ifOIHfIpCO){XpUIxtyxNE = true;}
      if(gSVXIefCOF == HdNJVnYREf){jezhImDzQm = true;}
      while(hjWnZheiYG == heiphMLsmS){oKgkXrbEuN = true;}
      while(ifOIHfIpCO == ifOIHfIpCO){AgomngAKfQ = true;}
      while(HdNJVnYREf == HdNJVnYREf){OUVTdIfEMN = true;}
      if(ZaWBpKcLNx == true){ZaWBpKcLNx = false;}
      if(toyfPMzKCW == true){toyfPMzKCW = false;}
      if(BxCqmEWNSk == true){BxCqmEWNSk = false;}
      if(IMesfIJNSw == true){IMesfIJNSw = false;}
      if(CwbFTSfgsJ == true){CwbFTSfgsJ = false;}
      if(nOOgebXeNk == true){nOOgebXeNk = false;}
      if(mPsbngKFzH == true){mPsbngKFzH = false;}
      if(ClqGTEVxXU == true){ClqGTEVxXU = false;}
      if(XpUIxtyxNE == true){XpUIxtyxNE = false;}
      if(jezhImDzQm == true){jezhImDzQm = false;}
      if(IahkGCWHMa == true){IahkGCWHMa = false;}
      if(GaHsMxtjet == true){GaHsMxtjet = false;}
      if(iMqtXXmJyg == true){iMqtXXmJyg = false;}
      if(AVgjMxHiCt == true){AVgjMxHiCt = false;}
      if(WpjmyJWFUK == true){WpjmyJWFUK = false;}
      if(rTUVEuimNh == true){rTUVEuimNh = false;}
      if(PNJfZjtWgP == true){PNJfZjtWgP = false;}
      if(oKgkXrbEuN == true){oKgkXrbEuN = false;}
      if(AgomngAKfQ == true){AgomngAKfQ = false;}
      if(OUVTdIfEMN == true){OUVTdIfEMN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZHEKMVZJZJ
{ 
  void CEhNsBcTrA()
  { 
      bool tQRkjUHgGX = false;
      bool mElWcOBcBj = false;
      bool WYXVrqStUw = false;
      bool YLyrTkjKZO = false;
      bool BieSrOlXzj = false;
      bool pZErdUswsr = false;
      bool CMMJybGsDq = false;
      bool dqCmintEcd = false;
      bool FuukFzHFNy = false;
      bool rnQhLnWUUQ = false;
      bool HrTHFzWQJp = false;
      bool anSPmGRbEy = false;
      bool TAfEypZUat = false;
      bool RZjMVNameG = false;
      bool bpwXXDUBuF = false;
      bool WuQVwGMOIO = false;
      bool ZCpPXADGsY = false;
      bool eewxVcEHgu = false;
      bool VXZVXWJOPP = false;
      bool BLdPgYAgzW = false;
      string Wlhyrrnsze;
      string KubZJJUraE;
      string sSoXNGIHxP;
      string UHraxsrqsB;
      string JBSjRPdqVd;
      string OoWyXEwJkl;
      string jfwiJCCAbP;
      string QczZurzFFx;
      string bRlTBfktDs;
      string wiGGQcFiZP;
      string McOFMlpwDf;
      string EqDmJRtbhd;
      string QIxiThVrJR;
      string lQfokyDfGO;
      string oopWoFeTwz;
      string icAuzEfeke;
      string QxmktmKNBL;
      string ONVTnbRasO;
      string CBuKiJPkpf;
      string SXtbyrCFcB;
      if(Wlhyrrnsze == McOFMlpwDf){tQRkjUHgGX = true;}
      else if(McOFMlpwDf == Wlhyrrnsze){HrTHFzWQJp = true;}
      if(KubZJJUraE == EqDmJRtbhd){mElWcOBcBj = true;}
      else if(EqDmJRtbhd == KubZJJUraE){anSPmGRbEy = true;}
      if(sSoXNGIHxP == QIxiThVrJR){WYXVrqStUw = true;}
      else if(QIxiThVrJR == sSoXNGIHxP){TAfEypZUat = true;}
      if(UHraxsrqsB == lQfokyDfGO){YLyrTkjKZO = true;}
      else if(lQfokyDfGO == UHraxsrqsB){RZjMVNameG = true;}
      if(JBSjRPdqVd == oopWoFeTwz){BieSrOlXzj = true;}
      else if(oopWoFeTwz == JBSjRPdqVd){bpwXXDUBuF = true;}
      if(OoWyXEwJkl == icAuzEfeke){pZErdUswsr = true;}
      else if(icAuzEfeke == OoWyXEwJkl){WuQVwGMOIO = true;}
      if(jfwiJCCAbP == QxmktmKNBL){CMMJybGsDq = true;}
      else if(QxmktmKNBL == jfwiJCCAbP){ZCpPXADGsY = true;}
      if(QczZurzFFx == ONVTnbRasO){dqCmintEcd = true;}
      if(bRlTBfktDs == CBuKiJPkpf){FuukFzHFNy = true;}
      if(wiGGQcFiZP == SXtbyrCFcB){rnQhLnWUUQ = true;}
      while(ONVTnbRasO == QczZurzFFx){eewxVcEHgu = true;}
      while(CBuKiJPkpf == CBuKiJPkpf){VXZVXWJOPP = true;}
      while(SXtbyrCFcB == SXtbyrCFcB){BLdPgYAgzW = true;}
      if(tQRkjUHgGX == true){tQRkjUHgGX = false;}
      if(mElWcOBcBj == true){mElWcOBcBj = false;}
      if(WYXVrqStUw == true){WYXVrqStUw = false;}
      if(YLyrTkjKZO == true){YLyrTkjKZO = false;}
      if(BieSrOlXzj == true){BieSrOlXzj = false;}
      if(pZErdUswsr == true){pZErdUswsr = false;}
      if(CMMJybGsDq == true){CMMJybGsDq = false;}
      if(dqCmintEcd == true){dqCmintEcd = false;}
      if(FuukFzHFNy == true){FuukFzHFNy = false;}
      if(rnQhLnWUUQ == true){rnQhLnWUUQ = false;}
      if(HrTHFzWQJp == true){HrTHFzWQJp = false;}
      if(anSPmGRbEy == true){anSPmGRbEy = false;}
      if(TAfEypZUat == true){TAfEypZUat = false;}
      if(RZjMVNameG == true){RZjMVNameG = false;}
      if(bpwXXDUBuF == true){bpwXXDUBuF = false;}
      if(WuQVwGMOIO == true){WuQVwGMOIO = false;}
      if(ZCpPXADGsY == true){ZCpPXADGsY = false;}
      if(eewxVcEHgu == true){eewxVcEHgu = false;}
      if(VXZVXWJOPP == true){VXZVXWJOPP = false;}
      if(BLdPgYAgzW == true){BLdPgYAgzW = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PWLOVISWPT
{ 
  void CcrdBVRein()
  { 
      bool eXzeeljUDE = false;
      bool GUZctUSgnb = false;
      bool mKRwUITkBs = false;
      bool OAZdJbDHFu = false;
      bool wjCrlcwuBq = false;
      bool eCSzEUGCKV = false;
      bool RzSKpGtwTS = false;
      bool QLasfCIYJr = false;
      bool MYiKNLgaYn = false;
      bool VAxicwRqCf = false;
      bool hzaiZTaZTB = false;
      bool LgrRJrPIwd = false;
      bool ZTWexPOqPi = false;
      bool kQWdJMyzjt = false;
      bool kGHWcnIeZj = false;
      bool gIuXFOculJ = false;
      bool FiKkQuJOIx = false;
      bool tPXBsSMzCI = false;
      bool HiReBbVzYu = false;
      bool jCNaoBwWGg = false;
      string zSkOMILyyn;
      string wQMiKEcLbL;
      string jAOVGDqlxy;
      string CfTmNPQwrY;
      string lsfZSxeWuV;
      string xJQHxzEPHJ;
      string RYAzlPxZUt;
      string mgSCYqTnCJ;
      string XTpaBsPLpO;
      string LAniHMrgnC;
      string AFBAiQpNhG;
      string eQNykxhZys;
      string AkjFimrjuM;
      string ymAzISRprj;
      string CCdijuloxK;
      string eRhtCOVwRu;
      string XFRVIrPOpR;
      string erhNpyHAVn;
      string ADmkgfEFws;
      string xVXwMxjHar;
      if(zSkOMILyyn == AFBAiQpNhG){eXzeeljUDE = true;}
      else if(AFBAiQpNhG == zSkOMILyyn){hzaiZTaZTB = true;}
      if(wQMiKEcLbL == eQNykxhZys){GUZctUSgnb = true;}
      else if(eQNykxhZys == wQMiKEcLbL){LgrRJrPIwd = true;}
      if(jAOVGDqlxy == AkjFimrjuM){mKRwUITkBs = true;}
      else if(AkjFimrjuM == jAOVGDqlxy){ZTWexPOqPi = true;}
      if(CfTmNPQwrY == ymAzISRprj){OAZdJbDHFu = true;}
      else if(ymAzISRprj == CfTmNPQwrY){kQWdJMyzjt = true;}
      if(lsfZSxeWuV == CCdijuloxK){wjCrlcwuBq = true;}
      else if(CCdijuloxK == lsfZSxeWuV){kGHWcnIeZj = true;}
      if(xJQHxzEPHJ == eRhtCOVwRu){eCSzEUGCKV = true;}
      else if(eRhtCOVwRu == xJQHxzEPHJ){gIuXFOculJ = true;}
      if(RYAzlPxZUt == XFRVIrPOpR){RzSKpGtwTS = true;}
      else if(XFRVIrPOpR == RYAzlPxZUt){FiKkQuJOIx = true;}
      if(mgSCYqTnCJ == erhNpyHAVn){QLasfCIYJr = true;}
      if(XTpaBsPLpO == ADmkgfEFws){MYiKNLgaYn = true;}
      if(LAniHMrgnC == xVXwMxjHar){VAxicwRqCf = true;}
      while(erhNpyHAVn == mgSCYqTnCJ){tPXBsSMzCI = true;}
      while(ADmkgfEFws == ADmkgfEFws){HiReBbVzYu = true;}
      while(xVXwMxjHar == xVXwMxjHar){jCNaoBwWGg = true;}
      if(eXzeeljUDE == true){eXzeeljUDE = false;}
      if(GUZctUSgnb == true){GUZctUSgnb = false;}
      if(mKRwUITkBs == true){mKRwUITkBs = false;}
      if(OAZdJbDHFu == true){OAZdJbDHFu = false;}
      if(wjCrlcwuBq == true){wjCrlcwuBq = false;}
      if(eCSzEUGCKV == true){eCSzEUGCKV = false;}
      if(RzSKpGtwTS == true){RzSKpGtwTS = false;}
      if(QLasfCIYJr == true){QLasfCIYJr = false;}
      if(MYiKNLgaYn == true){MYiKNLgaYn = false;}
      if(VAxicwRqCf == true){VAxicwRqCf = false;}
      if(hzaiZTaZTB == true){hzaiZTaZTB = false;}
      if(LgrRJrPIwd == true){LgrRJrPIwd = false;}
      if(ZTWexPOqPi == true){ZTWexPOqPi = false;}
      if(kQWdJMyzjt == true){kQWdJMyzjt = false;}
      if(kGHWcnIeZj == true){kGHWcnIeZj = false;}
      if(gIuXFOculJ == true){gIuXFOculJ = false;}
      if(FiKkQuJOIx == true){FiKkQuJOIx = false;}
      if(tPXBsSMzCI == true){tPXBsSMzCI = false;}
      if(HiReBbVzYu == true){HiReBbVzYu = false;}
      if(jCNaoBwWGg == true){jCNaoBwWGg = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RDZRYPLYYB
{ 
  void OIKmhycBTm()
  { 
      bool PIbPLGXcJt = false;
      bool BRbLjXDgwS = false;
      bool MnEgVwBMxo = false;
      bool dhFBjDWUai = false;
      bool QiwiYpqBRq = false;
      bool kUGcjbEtsO = false;
      bool ELeUQTLabf = false;
      bool AiGNCcqUIe = false;
      bool FAtGDIacRH = false;
      bool zbaLeEEbSF = false;
      bool dQZkpBOrZG = false;
      bool jilNJEQYQH = false;
      bool riJTgYHtix = false;
      bool YPCDAoZIPB = false;
      bool shCKXPoMCa = false;
      bool neKKMuHGER = false;
      bool sUJcgTfiMT = false;
      bool dIIOOJgyuQ = false;
      bool iuqIJtErpF = false;
      bool hSEyHYYYdD = false;
      string qIrtdxKAVs;
      string ySylileogO;
      string sWgVAVBblf;
      string roFDSZBuJg;
      string sPetmaTNfk;
      string oxiROTaWsI;
      string wiOpAINtTJ;
      string jjfzUdqmiZ;
      string BmuAqtFmoy;
      string yeCBjJBosd;
      string NYtWBERixk;
      string bnmTGInNzG;
      string CaFUKEhdOK;
      string bCsEMbsYzl;
      string bWoEWKqitb;
      string VmpmalTZLw;
      string WmTlIZthGe;
      string PPOMdfkByA;
      string MyjmooMGdZ;
      string ISNCOnCDuw;
      if(qIrtdxKAVs == NYtWBERixk){PIbPLGXcJt = true;}
      else if(NYtWBERixk == qIrtdxKAVs){dQZkpBOrZG = true;}
      if(ySylileogO == bnmTGInNzG){BRbLjXDgwS = true;}
      else if(bnmTGInNzG == ySylileogO){jilNJEQYQH = true;}
      if(sWgVAVBblf == CaFUKEhdOK){MnEgVwBMxo = true;}
      else if(CaFUKEhdOK == sWgVAVBblf){riJTgYHtix = true;}
      if(roFDSZBuJg == bCsEMbsYzl){dhFBjDWUai = true;}
      else if(bCsEMbsYzl == roFDSZBuJg){YPCDAoZIPB = true;}
      if(sPetmaTNfk == bWoEWKqitb){QiwiYpqBRq = true;}
      else if(bWoEWKqitb == sPetmaTNfk){shCKXPoMCa = true;}
      if(oxiROTaWsI == VmpmalTZLw){kUGcjbEtsO = true;}
      else if(VmpmalTZLw == oxiROTaWsI){neKKMuHGER = true;}
      if(wiOpAINtTJ == WmTlIZthGe){ELeUQTLabf = true;}
      else if(WmTlIZthGe == wiOpAINtTJ){sUJcgTfiMT = true;}
      if(jjfzUdqmiZ == PPOMdfkByA){AiGNCcqUIe = true;}
      if(BmuAqtFmoy == MyjmooMGdZ){FAtGDIacRH = true;}
      if(yeCBjJBosd == ISNCOnCDuw){zbaLeEEbSF = true;}
      while(PPOMdfkByA == jjfzUdqmiZ){dIIOOJgyuQ = true;}
      while(MyjmooMGdZ == MyjmooMGdZ){iuqIJtErpF = true;}
      while(ISNCOnCDuw == ISNCOnCDuw){hSEyHYYYdD = true;}
      if(PIbPLGXcJt == true){PIbPLGXcJt = false;}
      if(BRbLjXDgwS == true){BRbLjXDgwS = false;}
      if(MnEgVwBMxo == true){MnEgVwBMxo = false;}
      if(dhFBjDWUai == true){dhFBjDWUai = false;}
      if(QiwiYpqBRq == true){QiwiYpqBRq = false;}
      if(kUGcjbEtsO == true){kUGcjbEtsO = false;}
      if(ELeUQTLabf == true){ELeUQTLabf = false;}
      if(AiGNCcqUIe == true){AiGNCcqUIe = false;}
      if(FAtGDIacRH == true){FAtGDIacRH = false;}
      if(zbaLeEEbSF == true){zbaLeEEbSF = false;}
      if(dQZkpBOrZG == true){dQZkpBOrZG = false;}
      if(jilNJEQYQH == true){jilNJEQYQH = false;}
      if(riJTgYHtix == true){riJTgYHtix = false;}
      if(YPCDAoZIPB == true){YPCDAoZIPB = false;}
      if(shCKXPoMCa == true){shCKXPoMCa = false;}
      if(neKKMuHGER == true){neKKMuHGER = false;}
      if(sUJcgTfiMT == true){sUJcgTfiMT = false;}
      if(dIIOOJgyuQ == true){dIIOOJgyuQ = false;}
      if(iuqIJtErpF == true){iuqIJtErpF = false;}
      if(hSEyHYYYdD == true){hSEyHYYYdD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZXAMXWFQKJ
{ 
  void byVQphZjRr()
  { 
      bool CxOyKVUVmT = false;
      bool CJBrfFZUhw = false;
      bool moHdgqjYjC = false;
      bool yyzszSTyNV = false;
      bool DudWPZcpaR = false;
      bool gXKbHMpjRT = false;
      bool lwkjUEKIrM = false;
      bool kbHMsqKjOG = false;
      bool NsxSVZmWll = false;
      bool cifsjGJsBL = false;
      bool PUxOfEQyiP = false;
      bool BGVKGrySOu = false;
      bool QnVFjBwOcl = false;
      bool gQhCqruilQ = false;
      bool JonVjWNVmq = false;
      bool HqeGtbyqde = false;
      bool IBVwhkXSTN = false;
      bool wPgykbzzVN = false;
      bool nZakdplbHP = false;
      bool tDEHHHSrnz = false;
      string pNtBUKzYDz;
      string apygsxXArZ;
      string pyoASWTYcr;
      string zKArrDpGtP;
      string JgGmGwFSsQ;
      string AnZYDrXthy;
      string qZwKAPoGnp;
      string HRGcQJqCsV;
      string QAfMrLjbnA;
      string euXwXijwlq;
      string HzeDIhgxHt;
      string AVALgqtiSo;
      string eMRnKLtKEg;
      string YuPCOQVbDK;
      string dmUtzIuLOg;
      string oAdxLugpsw;
      string mWsaUEGxxP;
      string VSfHKrrXnB;
      string QhwXrXRRpS;
      string iehGWHssYt;
      if(pNtBUKzYDz == HzeDIhgxHt){CxOyKVUVmT = true;}
      else if(HzeDIhgxHt == pNtBUKzYDz){PUxOfEQyiP = true;}
      if(apygsxXArZ == AVALgqtiSo){CJBrfFZUhw = true;}
      else if(AVALgqtiSo == apygsxXArZ){BGVKGrySOu = true;}
      if(pyoASWTYcr == eMRnKLtKEg){moHdgqjYjC = true;}
      else if(eMRnKLtKEg == pyoASWTYcr){QnVFjBwOcl = true;}
      if(zKArrDpGtP == YuPCOQVbDK){yyzszSTyNV = true;}
      else if(YuPCOQVbDK == zKArrDpGtP){gQhCqruilQ = true;}
      if(JgGmGwFSsQ == dmUtzIuLOg){DudWPZcpaR = true;}
      else if(dmUtzIuLOg == JgGmGwFSsQ){JonVjWNVmq = true;}
      if(AnZYDrXthy == oAdxLugpsw){gXKbHMpjRT = true;}
      else if(oAdxLugpsw == AnZYDrXthy){HqeGtbyqde = true;}
      if(qZwKAPoGnp == mWsaUEGxxP){lwkjUEKIrM = true;}
      else if(mWsaUEGxxP == qZwKAPoGnp){IBVwhkXSTN = true;}
      if(HRGcQJqCsV == VSfHKrrXnB){kbHMsqKjOG = true;}
      if(QAfMrLjbnA == QhwXrXRRpS){NsxSVZmWll = true;}
      if(euXwXijwlq == iehGWHssYt){cifsjGJsBL = true;}
      while(VSfHKrrXnB == HRGcQJqCsV){wPgykbzzVN = true;}
      while(QhwXrXRRpS == QhwXrXRRpS){nZakdplbHP = true;}
      while(iehGWHssYt == iehGWHssYt){tDEHHHSrnz = true;}
      if(CxOyKVUVmT == true){CxOyKVUVmT = false;}
      if(CJBrfFZUhw == true){CJBrfFZUhw = false;}
      if(moHdgqjYjC == true){moHdgqjYjC = false;}
      if(yyzszSTyNV == true){yyzszSTyNV = false;}
      if(DudWPZcpaR == true){DudWPZcpaR = false;}
      if(gXKbHMpjRT == true){gXKbHMpjRT = false;}
      if(lwkjUEKIrM == true){lwkjUEKIrM = false;}
      if(kbHMsqKjOG == true){kbHMsqKjOG = false;}
      if(NsxSVZmWll == true){NsxSVZmWll = false;}
      if(cifsjGJsBL == true){cifsjGJsBL = false;}
      if(PUxOfEQyiP == true){PUxOfEQyiP = false;}
      if(BGVKGrySOu == true){BGVKGrySOu = false;}
      if(QnVFjBwOcl == true){QnVFjBwOcl = false;}
      if(gQhCqruilQ == true){gQhCqruilQ = false;}
      if(JonVjWNVmq == true){JonVjWNVmq = false;}
      if(HqeGtbyqde == true){HqeGtbyqde = false;}
      if(IBVwhkXSTN == true){IBVwhkXSTN = false;}
      if(wPgykbzzVN == true){wPgykbzzVN = false;}
      if(nZakdplbHP == true){nZakdplbHP = false;}
      if(tDEHHHSrnz == true){tDEHHHSrnz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XTTRHKYKUT
{ 
  void feDpdqFGZY()
  { 
      bool BOBmiVknue = false;
      bool qnuqPOyQDF = false;
      bool wSpZdPRLSh = false;
      bool gGYBjZUeGx = false;
      bool xtXUdqoHHI = false;
      bool HKpyVhYaJS = false;
      bool EYVNyjPtGo = false;
      bool sSJGICtUwm = false;
      bool aKEKZWQZfB = false;
      bool BsUnrYXKWs = false;
      bool auASOoAipT = false;
      bool nDeWrsULmb = false;
      bool ToNFmAxVNj = false;
      bool diyDLfKluk = false;
      bool VgjMNyOyuy = false;
      bool DMXtZkgueO = false;
      bool qeVGoEEbKL = false;
      bool iiQCGlFBqn = false;
      bool kzZOfQLEda = false;
      bool zOGfspLdWJ = false;
      string XwybTxKsCM;
      string LSSjYOMJYn;
      string wkHBteRmHp;
      string zGHKcUhmrO;
      string DOqIiVumHf;
      string cxqNYmoxgo;
      string eXkMNFLHGE;
      string fYRJkgsyRY;
      string jeeaMCPskp;
      string ekFCiedeud;
      string KmfDzEfgbr;
      string speIFzrJzK;
      string pRVQZuSOZF;
      string CODnNwzbaI;
      string WhQsxnFwwi;
      string lTejmrQOcE;
      string kQGWoemquI;
      string AQhpnHBcTl;
      string oVSpSiiTAU;
      string ejyWXGatER;
      if(XwybTxKsCM == KmfDzEfgbr){BOBmiVknue = true;}
      else if(KmfDzEfgbr == XwybTxKsCM){auASOoAipT = true;}
      if(LSSjYOMJYn == speIFzrJzK){qnuqPOyQDF = true;}
      else if(speIFzrJzK == LSSjYOMJYn){nDeWrsULmb = true;}
      if(wkHBteRmHp == pRVQZuSOZF){wSpZdPRLSh = true;}
      else if(pRVQZuSOZF == wkHBteRmHp){ToNFmAxVNj = true;}
      if(zGHKcUhmrO == CODnNwzbaI){gGYBjZUeGx = true;}
      else if(CODnNwzbaI == zGHKcUhmrO){diyDLfKluk = true;}
      if(DOqIiVumHf == WhQsxnFwwi){xtXUdqoHHI = true;}
      else if(WhQsxnFwwi == DOqIiVumHf){VgjMNyOyuy = true;}
      if(cxqNYmoxgo == lTejmrQOcE){HKpyVhYaJS = true;}
      else if(lTejmrQOcE == cxqNYmoxgo){DMXtZkgueO = true;}
      if(eXkMNFLHGE == kQGWoemquI){EYVNyjPtGo = true;}
      else if(kQGWoemquI == eXkMNFLHGE){qeVGoEEbKL = true;}
      if(fYRJkgsyRY == AQhpnHBcTl){sSJGICtUwm = true;}
      if(jeeaMCPskp == oVSpSiiTAU){aKEKZWQZfB = true;}
      if(ekFCiedeud == ejyWXGatER){BsUnrYXKWs = true;}
      while(AQhpnHBcTl == fYRJkgsyRY){iiQCGlFBqn = true;}
      while(oVSpSiiTAU == oVSpSiiTAU){kzZOfQLEda = true;}
      while(ejyWXGatER == ejyWXGatER){zOGfspLdWJ = true;}
      if(BOBmiVknue == true){BOBmiVknue = false;}
      if(qnuqPOyQDF == true){qnuqPOyQDF = false;}
      if(wSpZdPRLSh == true){wSpZdPRLSh = false;}
      if(gGYBjZUeGx == true){gGYBjZUeGx = false;}
      if(xtXUdqoHHI == true){xtXUdqoHHI = false;}
      if(HKpyVhYaJS == true){HKpyVhYaJS = false;}
      if(EYVNyjPtGo == true){EYVNyjPtGo = false;}
      if(sSJGICtUwm == true){sSJGICtUwm = false;}
      if(aKEKZWQZfB == true){aKEKZWQZfB = false;}
      if(BsUnrYXKWs == true){BsUnrYXKWs = false;}
      if(auASOoAipT == true){auASOoAipT = false;}
      if(nDeWrsULmb == true){nDeWrsULmb = false;}
      if(ToNFmAxVNj == true){ToNFmAxVNj = false;}
      if(diyDLfKluk == true){diyDLfKluk = false;}
      if(VgjMNyOyuy == true){VgjMNyOyuy = false;}
      if(DMXtZkgueO == true){DMXtZkgueO = false;}
      if(qeVGoEEbKL == true){qeVGoEEbKL = false;}
      if(iiQCGlFBqn == true){iiQCGlFBqn = false;}
      if(kzZOfQLEda == true){kzZOfQLEda = false;}
      if(zOGfspLdWJ == true){zOGfspLdWJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XCZGXKRBKJ
{ 
  void fkiKgEsZPs()
  { 
      bool OkVKwVtJLs = false;
      bool LBOxYyXKfY = false;
      bool DwDnLRqXma = false;
      bool WEiYotCzVR = false;
      bool lFFhgNqRZd = false;
      bool YmEEBRhgzd = false;
      bool dVVjWHRpId = false;
      bool uXoNlyqUHx = false;
      bool aYxIBizdXW = false;
      bool MVAqnPiULZ = false;
      bool uBJwiAyqRC = false;
      bool ZNYBCEYNim = false;
      bool HnRkRYXxZW = false;
      bool PWIhBZPBZA = false;
      bool kIMUwywzoi = false;
      bool qVkXRHiXdJ = false;
      bool cBQumDEqAc = false;
      bool oWmhCyaxoS = false;
      bool cYHqeskkPs = false;
      bool qqXQWjrQgs = false;
      string HoIeSeiXRm;
      string MVEoOpCVey;
      string ManoOZQHqE;
      string ZdpZKqGuiD;
      string eXAAqZmYFl;
      string GQoCpfBTQr;
      string NbfMNifdeW;
      string jHlbTnamyn;
      string mrBFyJnmjd;
      string plepoDqmmI;
      string tItiBzdNiB;
      string FNVbFVZaOD;
      string tstBgxbulQ;
      string xmAptdySDz;
      string eZDhXcQDkT;
      string cQYiNBNrCE;
      string WhbSESNega;
      string XHWOXToMdE;
      string fYJRmfOQbs;
      string PFPzbxxjcE;
      if(HoIeSeiXRm == tItiBzdNiB){OkVKwVtJLs = true;}
      else if(tItiBzdNiB == HoIeSeiXRm){uBJwiAyqRC = true;}
      if(MVEoOpCVey == FNVbFVZaOD){LBOxYyXKfY = true;}
      else if(FNVbFVZaOD == MVEoOpCVey){ZNYBCEYNim = true;}
      if(ManoOZQHqE == tstBgxbulQ){DwDnLRqXma = true;}
      else if(tstBgxbulQ == ManoOZQHqE){HnRkRYXxZW = true;}
      if(ZdpZKqGuiD == xmAptdySDz){WEiYotCzVR = true;}
      else if(xmAptdySDz == ZdpZKqGuiD){PWIhBZPBZA = true;}
      if(eXAAqZmYFl == eZDhXcQDkT){lFFhgNqRZd = true;}
      else if(eZDhXcQDkT == eXAAqZmYFl){kIMUwywzoi = true;}
      if(GQoCpfBTQr == cQYiNBNrCE){YmEEBRhgzd = true;}
      else if(cQYiNBNrCE == GQoCpfBTQr){qVkXRHiXdJ = true;}
      if(NbfMNifdeW == WhbSESNega){dVVjWHRpId = true;}
      else if(WhbSESNega == NbfMNifdeW){cBQumDEqAc = true;}
      if(jHlbTnamyn == XHWOXToMdE){uXoNlyqUHx = true;}
      if(mrBFyJnmjd == fYJRmfOQbs){aYxIBizdXW = true;}
      if(plepoDqmmI == PFPzbxxjcE){MVAqnPiULZ = true;}
      while(XHWOXToMdE == jHlbTnamyn){oWmhCyaxoS = true;}
      while(fYJRmfOQbs == fYJRmfOQbs){cYHqeskkPs = true;}
      while(PFPzbxxjcE == PFPzbxxjcE){qqXQWjrQgs = true;}
      if(OkVKwVtJLs == true){OkVKwVtJLs = false;}
      if(LBOxYyXKfY == true){LBOxYyXKfY = false;}
      if(DwDnLRqXma == true){DwDnLRqXma = false;}
      if(WEiYotCzVR == true){WEiYotCzVR = false;}
      if(lFFhgNqRZd == true){lFFhgNqRZd = false;}
      if(YmEEBRhgzd == true){YmEEBRhgzd = false;}
      if(dVVjWHRpId == true){dVVjWHRpId = false;}
      if(uXoNlyqUHx == true){uXoNlyqUHx = false;}
      if(aYxIBizdXW == true){aYxIBizdXW = false;}
      if(MVAqnPiULZ == true){MVAqnPiULZ = false;}
      if(uBJwiAyqRC == true){uBJwiAyqRC = false;}
      if(ZNYBCEYNim == true){ZNYBCEYNim = false;}
      if(HnRkRYXxZW == true){HnRkRYXxZW = false;}
      if(PWIhBZPBZA == true){PWIhBZPBZA = false;}
      if(kIMUwywzoi == true){kIMUwywzoi = false;}
      if(qVkXRHiXdJ == true){qVkXRHiXdJ = false;}
      if(cBQumDEqAc == true){cBQumDEqAc = false;}
      if(oWmhCyaxoS == true){oWmhCyaxoS = false;}
      if(cYHqeskkPs == true){cYHqeskkPs = false;}
      if(qqXQWjrQgs == true){qqXQWjrQgs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OOBNEISCEP
{ 
  void QCrEEJKFRj()
  { 
      bool xCWnBSdqOL = false;
      bool zhjqRHkzXb = false;
      bool zEMrORHgHV = false;
      bool sjSqMgRwLV = false;
      bool ZmzxXTUIMb = false;
      bool ramRTroeZi = false;
      bool KxjrSPOrdk = false;
      bool pomBBMEPUu = false;
      bool xzTedSwBXU = false;
      bool qlVcLcCHeH = false;
      bool FbdblDmlPH = false;
      bool mZKelLcFOA = false;
      bool TMUJtESJpY = false;
      bool JxkMpUUrrZ = false;
      bool eJlPgQaqqz = false;
      bool AxsdnrHUgh = false;
      bool kdwpxooynQ = false;
      bool qjMnUJaEbc = false;
      bool caSpYMEGXx = false;
      bool NALKoBNuRq = false;
      string WqlXjdTkBl;
      string GwxAdRVLAO;
      string DJIANwEkyV;
      string cPbJCsWcET;
      string QWmUahTBKG;
      string QSeaHNikKn;
      string ZhleIpUnxj;
      string usGfyfVnYe;
      string bDarFKkABo;
      string KkRQthkILj;
      string NfDZLXLycb;
      string quLfLVHsYy;
      string FTlIZJaplP;
      string uNboQuzAgg;
      string zlXZxPZlFX;
      string ZqiNettuAB;
      string wVMMfdSWOQ;
      string WHMqzSuMKJ;
      string iokNoEdNJW;
      string BEzejcZLsT;
      if(WqlXjdTkBl == NfDZLXLycb){xCWnBSdqOL = true;}
      else if(NfDZLXLycb == WqlXjdTkBl){FbdblDmlPH = true;}
      if(GwxAdRVLAO == quLfLVHsYy){zhjqRHkzXb = true;}
      else if(quLfLVHsYy == GwxAdRVLAO){mZKelLcFOA = true;}
      if(DJIANwEkyV == FTlIZJaplP){zEMrORHgHV = true;}
      else if(FTlIZJaplP == DJIANwEkyV){TMUJtESJpY = true;}
      if(cPbJCsWcET == uNboQuzAgg){sjSqMgRwLV = true;}
      else if(uNboQuzAgg == cPbJCsWcET){JxkMpUUrrZ = true;}
      if(QWmUahTBKG == zlXZxPZlFX){ZmzxXTUIMb = true;}
      else if(zlXZxPZlFX == QWmUahTBKG){eJlPgQaqqz = true;}
      if(QSeaHNikKn == ZqiNettuAB){ramRTroeZi = true;}
      else if(ZqiNettuAB == QSeaHNikKn){AxsdnrHUgh = true;}
      if(ZhleIpUnxj == wVMMfdSWOQ){KxjrSPOrdk = true;}
      else if(wVMMfdSWOQ == ZhleIpUnxj){kdwpxooynQ = true;}
      if(usGfyfVnYe == WHMqzSuMKJ){pomBBMEPUu = true;}
      if(bDarFKkABo == iokNoEdNJW){xzTedSwBXU = true;}
      if(KkRQthkILj == BEzejcZLsT){qlVcLcCHeH = true;}
      while(WHMqzSuMKJ == usGfyfVnYe){qjMnUJaEbc = true;}
      while(iokNoEdNJW == iokNoEdNJW){caSpYMEGXx = true;}
      while(BEzejcZLsT == BEzejcZLsT){NALKoBNuRq = true;}
      if(xCWnBSdqOL == true){xCWnBSdqOL = false;}
      if(zhjqRHkzXb == true){zhjqRHkzXb = false;}
      if(zEMrORHgHV == true){zEMrORHgHV = false;}
      if(sjSqMgRwLV == true){sjSqMgRwLV = false;}
      if(ZmzxXTUIMb == true){ZmzxXTUIMb = false;}
      if(ramRTroeZi == true){ramRTroeZi = false;}
      if(KxjrSPOrdk == true){KxjrSPOrdk = false;}
      if(pomBBMEPUu == true){pomBBMEPUu = false;}
      if(xzTedSwBXU == true){xzTedSwBXU = false;}
      if(qlVcLcCHeH == true){qlVcLcCHeH = false;}
      if(FbdblDmlPH == true){FbdblDmlPH = false;}
      if(mZKelLcFOA == true){mZKelLcFOA = false;}
      if(TMUJtESJpY == true){TMUJtESJpY = false;}
      if(JxkMpUUrrZ == true){JxkMpUUrrZ = false;}
      if(eJlPgQaqqz == true){eJlPgQaqqz = false;}
      if(AxsdnrHUgh == true){AxsdnrHUgh = false;}
      if(kdwpxooynQ == true){kdwpxooynQ = false;}
      if(qjMnUJaEbc == true){qjMnUJaEbc = false;}
      if(caSpYMEGXx == true){caSpYMEGXx = false;}
      if(NALKoBNuRq == true){NALKoBNuRq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DMAHHOKYQU
{ 
  void ZzbngVbyOK()
  { 
      bool cGZpiFfGzk = false;
      bool wNMUAYhzEk = false;
      bool CzIprhutAc = false;
      bool gcGeRzFLSZ = false;
      bool lEXGfMHSiw = false;
      bool SxZiMcyiyB = false;
      bool JkGVDTHgCA = false;
      bool TWgezxaBKe = false;
      bool ZcKEKiDXRo = false;
      bool BWMQPAxCCL = false;
      bool FaPibuQtjO = false;
      bool CwXSagZCKT = false;
      bool PiONzwJPPy = false;
      bool zYyyZzMbbP = false;
      bool SInDlEytpi = false;
      bool auqhuVLCif = false;
      bool kBelPcxZra = false;
      bool TxauBHuLat = false;
      bool yPZgWEgQCE = false;
      bool nKswdQcCmI = false;
      string uGNjoTCryi;
      string uJIsQtUpAY;
      string ehlHaAkzIz;
      string keSCqtTCix;
      string dSWpIRufiu;
      string uDqzuMUFJo;
      string hCjzxQpGBW;
      string aRGCuMLuzI;
      string FFPMDhmTGe;
      string ZNstOahXsr;
      string andMUcwoEh;
      string BYGAjclWuP;
      string cWzWwNYMmT;
      string dmBGgQaDrZ;
      string kyOYcAGsKk;
      string nLbdOEoGPH;
      string EjBjENXzST;
      string CieUddHatu;
      string GeBBeSLXqc;
      string aTmjUAxeTO;
      if(uGNjoTCryi == andMUcwoEh){cGZpiFfGzk = true;}
      else if(andMUcwoEh == uGNjoTCryi){FaPibuQtjO = true;}
      if(uJIsQtUpAY == BYGAjclWuP){wNMUAYhzEk = true;}
      else if(BYGAjclWuP == uJIsQtUpAY){CwXSagZCKT = true;}
      if(ehlHaAkzIz == cWzWwNYMmT){CzIprhutAc = true;}
      else if(cWzWwNYMmT == ehlHaAkzIz){PiONzwJPPy = true;}
      if(keSCqtTCix == dmBGgQaDrZ){gcGeRzFLSZ = true;}
      else if(dmBGgQaDrZ == keSCqtTCix){zYyyZzMbbP = true;}
      if(dSWpIRufiu == kyOYcAGsKk){lEXGfMHSiw = true;}
      else if(kyOYcAGsKk == dSWpIRufiu){SInDlEytpi = true;}
      if(uDqzuMUFJo == nLbdOEoGPH){SxZiMcyiyB = true;}
      else if(nLbdOEoGPH == uDqzuMUFJo){auqhuVLCif = true;}
      if(hCjzxQpGBW == EjBjENXzST){JkGVDTHgCA = true;}
      else if(EjBjENXzST == hCjzxQpGBW){kBelPcxZra = true;}
      if(aRGCuMLuzI == CieUddHatu){TWgezxaBKe = true;}
      if(FFPMDhmTGe == GeBBeSLXqc){ZcKEKiDXRo = true;}
      if(ZNstOahXsr == aTmjUAxeTO){BWMQPAxCCL = true;}
      while(CieUddHatu == aRGCuMLuzI){TxauBHuLat = true;}
      while(GeBBeSLXqc == GeBBeSLXqc){yPZgWEgQCE = true;}
      while(aTmjUAxeTO == aTmjUAxeTO){nKswdQcCmI = true;}
      if(cGZpiFfGzk == true){cGZpiFfGzk = false;}
      if(wNMUAYhzEk == true){wNMUAYhzEk = false;}
      if(CzIprhutAc == true){CzIprhutAc = false;}
      if(gcGeRzFLSZ == true){gcGeRzFLSZ = false;}
      if(lEXGfMHSiw == true){lEXGfMHSiw = false;}
      if(SxZiMcyiyB == true){SxZiMcyiyB = false;}
      if(JkGVDTHgCA == true){JkGVDTHgCA = false;}
      if(TWgezxaBKe == true){TWgezxaBKe = false;}
      if(ZcKEKiDXRo == true){ZcKEKiDXRo = false;}
      if(BWMQPAxCCL == true){BWMQPAxCCL = false;}
      if(FaPibuQtjO == true){FaPibuQtjO = false;}
      if(CwXSagZCKT == true){CwXSagZCKT = false;}
      if(PiONzwJPPy == true){PiONzwJPPy = false;}
      if(zYyyZzMbbP == true){zYyyZzMbbP = false;}
      if(SInDlEytpi == true){SInDlEytpi = false;}
      if(auqhuVLCif == true){auqhuVLCif = false;}
      if(kBelPcxZra == true){kBelPcxZra = false;}
      if(TxauBHuLat == true){TxauBHuLat = false;}
      if(yPZgWEgQCE == true){yPZgWEgQCE = false;}
      if(nKswdQcCmI == true){nKswdQcCmI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JFCRRZXIEK
{ 
  void pdoKCHgHBy()
  { 
      bool xpmIwEDJjd = false;
      bool gOgxjwTBDu = false;
      bool sgXkflbjLr = false;
      bool UHazGGIRks = false;
      bool lmbPTQdAqe = false;
      bool DBoAPPSBbJ = false;
      bool bSyqJZmgpd = false;
      bool mBQKJGFHRP = false;
      bool HbsgoSSajp = false;
      bool nGpbQSojGt = false;
      bool dxJMqBkdpJ = false;
      bool KKjLBAZRCp = false;
      bool GLwApfLosq = false;
      bool QVBWlKSfom = false;
      bool hnjafcWdZQ = false;
      bool rWgQmRAJWL = false;
      bool pwkzroJMZq = false;
      bool gsbPHSaVTM = false;
      bool IhOcytnjjU = false;
      bool KJJdxlDkhq = false;
      string VHDwrjMasT;
      string pGSJjUyQLN;
      string mmKfLMmmKI;
      string hflwSssgap;
      string eznRWyJnoO;
      string ddgNbixWje;
      string xYQkkKUWqq;
      string bRpUimyrIN;
      string DyyweEzYTi;
      string uVwusFiDfM;
      string dmxxmMnLdn;
      string kENWbyaUGf;
      string KuCHuFUtbc;
      string EtLujPBAIS;
      string QmtfypIZTt;
      string eOxzBhkoCg;
      string qsjBjpsRmL;
      string xZLmPXAWIE;
      string xuZhxoxLtB;
      string eLyaaWDsuI;
      if(VHDwrjMasT == dmxxmMnLdn){xpmIwEDJjd = true;}
      else if(dmxxmMnLdn == VHDwrjMasT){dxJMqBkdpJ = true;}
      if(pGSJjUyQLN == kENWbyaUGf){gOgxjwTBDu = true;}
      else if(kENWbyaUGf == pGSJjUyQLN){KKjLBAZRCp = true;}
      if(mmKfLMmmKI == KuCHuFUtbc){sgXkflbjLr = true;}
      else if(KuCHuFUtbc == mmKfLMmmKI){GLwApfLosq = true;}
      if(hflwSssgap == EtLujPBAIS){UHazGGIRks = true;}
      else if(EtLujPBAIS == hflwSssgap){QVBWlKSfom = true;}
      if(eznRWyJnoO == QmtfypIZTt){lmbPTQdAqe = true;}
      else if(QmtfypIZTt == eznRWyJnoO){hnjafcWdZQ = true;}
      if(ddgNbixWje == eOxzBhkoCg){DBoAPPSBbJ = true;}
      else if(eOxzBhkoCg == ddgNbixWje){rWgQmRAJWL = true;}
      if(xYQkkKUWqq == qsjBjpsRmL){bSyqJZmgpd = true;}
      else if(qsjBjpsRmL == xYQkkKUWqq){pwkzroJMZq = true;}
      if(bRpUimyrIN == xZLmPXAWIE){mBQKJGFHRP = true;}
      if(DyyweEzYTi == xuZhxoxLtB){HbsgoSSajp = true;}
      if(uVwusFiDfM == eLyaaWDsuI){nGpbQSojGt = true;}
      while(xZLmPXAWIE == bRpUimyrIN){gsbPHSaVTM = true;}
      while(xuZhxoxLtB == xuZhxoxLtB){IhOcytnjjU = true;}
      while(eLyaaWDsuI == eLyaaWDsuI){KJJdxlDkhq = true;}
      if(xpmIwEDJjd == true){xpmIwEDJjd = false;}
      if(gOgxjwTBDu == true){gOgxjwTBDu = false;}
      if(sgXkflbjLr == true){sgXkflbjLr = false;}
      if(UHazGGIRks == true){UHazGGIRks = false;}
      if(lmbPTQdAqe == true){lmbPTQdAqe = false;}
      if(DBoAPPSBbJ == true){DBoAPPSBbJ = false;}
      if(bSyqJZmgpd == true){bSyqJZmgpd = false;}
      if(mBQKJGFHRP == true){mBQKJGFHRP = false;}
      if(HbsgoSSajp == true){HbsgoSSajp = false;}
      if(nGpbQSojGt == true){nGpbQSojGt = false;}
      if(dxJMqBkdpJ == true){dxJMqBkdpJ = false;}
      if(KKjLBAZRCp == true){KKjLBAZRCp = false;}
      if(GLwApfLosq == true){GLwApfLosq = false;}
      if(QVBWlKSfom == true){QVBWlKSfom = false;}
      if(hnjafcWdZQ == true){hnjafcWdZQ = false;}
      if(rWgQmRAJWL == true){rWgQmRAJWL = false;}
      if(pwkzroJMZq == true){pwkzroJMZq = false;}
      if(gsbPHSaVTM == true){gsbPHSaVTM = false;}
      if(IhOcytnjjU == true){IhOcytnjjU = false;}
      if(KJJdxlDkhq == true){KJJdxlDkhq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NSADLRWROH
{ 
  void hRkKHxJsaU()
  { 
      bool RSPJecEcbk = false;
      bool qmeHekKJfl = false;
      bool gzUmlnnAiH = false;
      bool VTzxWMHGnR = false;
      bool eBYRfVoUOb = false;
      bool OURhYSycXp = false;
      bool nDsUOXmhjR = false;
      bool dDVNGeaDGP = false;
      bool zSlHsBBRXE = false;
      bool OeixdbIDCI = false;
      bool iHkwxKzDas = false;
      bool qtVHqURlaN = false;
      bool VUJfDzTXje = false;
      bool WWAygrhoJh = false;
      bool BypdMTIbGJ = false;
      bool KVbnmNxEsy = false;
      bool paTwWTtWaz = false;
      bool lbbsMXefSi = false;
      bool prRaUBtLLS = false;
      bool tcuLrZdtHs = false;
      string zgmpumjdAD;
      string gLlBYfHmVa;
      string RlVlQDTRsC;
      string ZedgunFqFR;
      string CjpUyiWLnH;
      string XdnnlMKAYq;
      string SSkxdBgFFj;
      string aQhEXgwAmh;
      string gHckdwqoui;
      string cERNyiyrqx;
      string BQFEgHuiPE;
      string PKNqKKpzSh;
      string JrKWHVyTGn;
      string iDJpgPCYEW;
      string AIZpSSpaDm;
      string szYJmJGhwK;
      string JacaVLAXKt;
      string IQtwgBRTDo;
      string fXQStieACW;
      string ITbtNYeyTG;
      if(zgmpumjdAD == BQFEgHuiPE){RSPJecEcbk = true;}
      else if(BQFEgHuiPE == zgmpumjdAD){iHkwxKzDas = true;}
      if(gLlBYfHmVa == PKNqKKpzSh){qmeHekKJfl = true;}
      else if(PKNqKKpzSh == gLlBYfHmVa){qtVHqURlaN = true;}
      if(RlVlQDTRsC == JrKWHVyTGn){gzUmlnnAiH = true;}
      else if(JrKWHVyTGn == RlVlQDTRsC){VUJfDzTXje = true;}
      if(ZedgunFqFR == iDJpgPCYEW){VTzxWMHGnR = true;}
      else if(iDJpgPCYEW == ZedgunFqFR){WWAygrhoJh = true;}
      if(CjpUyiWLnH == AIZpSSpaDm){eBYRfVoUOb = true;}
      else if(AIZpSSpaDm == CjpUyiWLnH){BypdMTIbGJ = true;}
      if(XdnnlMKAYq == szYJmJGhwK){OURhYSycXp = true;}
      else if(szYJmJGhwK == XdnnlMKAYq){KVbnmNxEsy = true;}
      if(SSkxdBgFFj == JacaVLAXKt){nDsUOXmhjR = true;}
      else if(JacaVLAXKt == SSkxdBgFFj){paTwWTtWaz = true;}
      if(aQhEXgwAmh == IQtwgBRTDo){dDVNGeaDGP = true;}
      if(gHckdwqoui == fXQStieACW){zSlHsBBRXE = true;}
      if(cERNyiyrqx == ITbtNYeyTG){OeixdbIDCI = true;}
      while(IQtwgBRTDo == aQhEXgwAmh){lbbsMXefSi = true;}
      while(fXQStieACW == fXQStieACW){prRaUBtLLS = true;}
      while(ITbtNYeyTG == ITbtNYeyTG){tcuLrZdtHs = true;}
      if(RSPJecEcbk == true){RSPJecEcbk = false;}
      if(qmeHekKJfl == true){qmeHekKJfl = false;}
      if(gzUmlnnAiH == true){gzUmlnnAiH = false;}
      if(VTzxWMHGnR == true){VTzxWMHGnR = false;}
      if(eBYRfVoUOb == true){eBYRfVoUOb = false;}
      if(OURhYSycXp == true){OURhYSycXp = false;}
      if(nDsUOXmhjR == true){nDsUOXmhjR = false;}
      if(dDVNGeaDGP == true){dDVNGeaDGP = false;}
      if(zSlHsBBRXE == true){zSlHsBBRXE = false;}
      if(OeixdbIDCI == true){OeixdbIDCI = false;}
      if(iHkwxKzDas == true){iHkwxKzDas = false;}
      if(qtVHqURlaN == true){qtVHqURlaN = false;}
      if(VUJfDzTXje == true){VUJfDzTXje = false;}
      if(WWAygrhoJh == true){WWAygrhoJh = false;}
      if(BypdMTIbGJ == true){BypdMTIbGJ = false;}
      if(KVbnmNxEsy == true){KVbnmNxEsy = false;}
      if(paTwWTtWaz == true){paTwWTtWaz = false;}
      if(lbbsMXefSi == true){lbbsMXefSi = false;}
      if(prRaUBtLLS == true){prRaUBtLLS = false;}
      if(tcuLrZdtHs == true){tcuLrZdtHs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IRDHNJQNFX
{ 
  void BLpEtTIPHW()
  { 
      bool CTwYNzwfFu = false;
      bool lxwdJDWyII = false;
      bool MPguZeGAny = false;
      bool LzjPIPiLzY = false;
      bool IbADfyCPMm = false;
      bool oGacTmmByh = false;
      bool rurHilNgVs = false;
      bool TLyiAkPcPa = false;
      bool cqnAIFXGuV = false;
      bool chMRfMisTs = false;
      bool kUsSTdHqch = false;
      bool XJkXFBHqyc = false;
      bool OGkAQkzfDm = false;
      bool BVIWpHxUge = false;
      bool xfdMxSxnMe = false;
      bool FrtqddjrCh = false;
      bool ebnogBBOhx = false;
      bool DlKTehbXCy = false;
      bool nKSXhJbRrL = false;
      bool XESyAVqofw = false;
      string QruttdWSVj;
      string dsIASELmug;
      string IwmDoCUeqU;
      string CRcBKUfNZZ;
      string afPZCjwhWA;
      string WylJOUFqEr;
      string JqbBgLQHoy;
      string XyMVGcfNBF;
      string FlHpyEfSlp;
      string PoIhPiENrq;
      string SorYbYkQDl;
      string nqpzlXjkDc;
      string eyVziioeFW;
      string XSVKlPsiOY;
      string SPPzCXzJur;
      string zSKPLUTsor;
      string iGLfNVVeMW;
      string eAWdJPiVwK;
      string ifjMXFIFyR;
      string wzsVpLEilg;
      if(QruttdWSVj == SorYbYkQDl){CTwYNzwfFu = true;}
      else if(SorYbYkQDl == QruttdWSVj){kUsSTdHqch = true;}
      if(dsIASELmug == nqpzlXjkDc){lxwdJDWyII = true;}
      else if(nqpzlXjkDc == dsIASELmug){XJkXFBHqyc = true;}
      if(IwmDoCUeqU == eyVziioeFW){MPguZeGAny = true;}
      else if(eyVziioeFW == IwmDoCUeqU){OGkAQkzfDm = true;}
      if(CRcBKUfNZZ == XSVKlPsiOY){LzjPIPiLzY = true;}
      else if(XSVKlPsiOY == CRcBKUfNZZ){BVIWpHxUge = true;}
      if(afPZCjwhWA == SPPzCXzJur){IbADfyCPMm = true;}
      else if(SPPzCXzJur == afPZCjwhWA){xfdMxSxnMe = true;}
      if(WylJOUFqEr == zSKPLUTsor){oGacTmmByh = true;}
      else if(zSKPLUTsor == WylJOUFqEr){FrtqddjrCh = true;}
      if(JqbBgLQHoy == iGLfNVVeMW){rurHilNgVs = true;}
      else if(iGLfNVVeMW == JqbBgLQHoy){ebnogBBOhx = true;}
      if(XyMVGcfNBF == eAWdJPiVwK){TLyiAkPcPa = true;}
      if(FlHpyEfSlp == ifjMXFIFyR){cqnAIFXGuV = true;}
      if(PoIhPiENrq == wzsVpLEilg){chMRfMisTs = true;}
      while(eAWdJPiVwK == XyMVGcfNBF){DlKTehbXCy = true;}
      while(ifjMXFIFyR == ifjMXFIFyR){nKSXhJbRrL = true;}
      while(wzsVpLEilg == wzsVpLEilg){XESyAVqofw = true;}
      if(CTwYNzwfFu == true){CTwYNzwfFu = false;}
      if(lxwdJDWyII == true){lxwdJDWyII = false;}
      if(MPguZeGAny == true){MPguZeGAny = false;}
      if(LzjPIPiLzY == true){LzjPIPiLzY = false;}
      if(IbADfyCPMm == true){IbADfyCPMm = false;}
      if(oGacTmmByh == true){oGacTmmByh = false;}
      if(rurHilNgVs == true){rurHilNgVs = false;}
      if(TLyiAkPcPa == true){TLyiAkPcPa = false;}
      if(cqnAIFXGuV == true){cqnAIFXGuV = false;}
      if(chMRfMisTs == true){chMRfMisTs = false;}
      if(kUsSTdHqch == true){kUsSTdHqch = false;}
      if(XJkXFBHqyc == true){XJkXFBHqyc = false;}
      if(OGkAQkzfDm == true){OGkAQkzfDm = false;}
      if(BVIWpHxUge == true){BVIWpHxUge = false;}
      if(xfdMxSxnMe == true){xfdMxSxnMe = false;}
      if(FrtqddjrCh == true){FrtqddjrCh = false;}
      if(ebnogBBOhx == true){ebnogBBOhx = false;}
      if(DlKTehbXCy == true){DlKTehbXCy = false;}
      if(nKSXhJbRrL == true){nKSXhJbRrL = false;}
      if(XESyAVqofw == true){XESyAVqofw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DRMHTUUAKS
{ 
  void PRqmIKIVRj()
  { 
      bool mSHkaGULNB = false;
      bool jPdAPACzFQ = false;
      bool fNSJdFgDel = false;
      bool NuFPTlkoEF = false;
      bool BPPGSjNfzw = false;
      bool ObARdYzyHV = false;
      bool YRfbnoKOgd = false;
      bool ONuAOQyMps = false;
      bool qBDiWltmkt = false;
      bool lmhhfjimSd = false;
      bool ZzyVgBfADM = false;
      bool amHNeXNxEB = false;
      bool tWcmqbeNTd = false;
      bool gKtMSqCgBV = false;
      bool FtjYSzeTCI = false;
      bool WliQjCVLeF = false;
      bool VSgYfQruum = false;
      bool UgwhuzmETq = false;
      bool wLKKHuacbQ = false;
      bool VqCwAmNKsC = false;
      string rlcwxHONNb;
      string mIuRVHGcbI;
      string KymNxBVeAt;
      string QzRTjRkVqE;
      string GGaaazmdxr;
      string tDHyBtyOzu;
      string ljMTLmqahE;
      string WkkXBDWLmx;
      string OrgHAMqSSp;
      string SVWuSVsHPm;
      string ILusVISjwx;
      string XmaBgsCFTy;
      string wXIrkAOoEf;
      string KPuznDwnlq;
      string tGWUSetUOY;
      string TnnRqkCDHV;
      string tceQmHqbMd;
      string xNSbDoAmZG;
      string FsZfLWadID;
      string ArgPxRaFhK;
      if(rlcwxHONNb == ILusVISjwx){mSHkaGULNB = true;}
      else if(ILusVISjwx == rlcwxHONNb){ZzyVgBfADM = true;}
      if(mIuRVHGcbI == XmaBgsCFTy){jPdAPACzFQ = true;}
      else if(XmaBgsCFTy == mIuRVHGcbI){amHNeXNxEB = true;}
      if(KymNxBVeAt == wXIrkAOoEf){fNSJdFgDel = true;}
      else if(wXIrkAOoEf == KymNxBVeAt){tWcmqbeNTd = true;}
      if(QzRTjRkVqE == KPuznDwnlq){NuFPTlkoEF = true;}
      else if(KPuznDwnlq == QzRTjRkVqE){gKtMSqCgBV = true;}
      if(GGaaazmdxr == tGWUSetUOY){BPPGSjNfzw = true;}
      else if(tGWUSetUOY == GGaaazmdxr){FtjYSzeTCI = true;}
      if(tDHyBtyOzu == TnnRqkCDHV){ObARdYzyHV = true;}
      else if(TnnRqkCDHV == tDHyBtyOzu){WliQjCVLeF = true;}
      if(ljMTLmqahE == tceQmHqbMd){YRfbnoKOgd = true;}
      else if(tceQmHqbMd == ljMTLmqahE){VSgYfQruum = true;}
      if(WkkXBDWLmx == xNSbDoAmZG){ONuAOQyMps = true;}
      if(OrgHAMqSSp == FsZfLWadID){qBDiWltmkt = true;}
      if(SVWuSVsHPm == ArgPxRaFhK){lmhhfjimSd = true;}
      while(xNSbDoAmZG == WkkXBDWLmx){UgwhuzmETq = true;}
      while(FsZfLWadID == FsZfLWadID){wLKKHuacbQ = true;}
      while(ArgPxRaFhK == ArgPxRaFhK){VqCwAmNKsC = true;}
      if(mSHkaGULNB == true){mSHkaGULNB = false;}
      if(jPdAPACzFQ == true){jPdAPACzFQ = false;}
      if(fNSJdFgDel == true){fNSJdFgDel = false;}
      if(NuFPTlkoEF == true){NuFPTlkoEF = false;}
      if(BPPGSjNfzw == true){BPPGSjNfzw = false;}
      if(ObARdYzyHV == true){ObARdYzyHV = false;}
      if(YRfbnoKOgd == true){YRfbnoKOgd = false;}
      if(ONuAOQyMps == true){ONuAOQyMps = false;}
      if(qBDiWltmkt == true){qBDiWltmkt = false;}
      if(lmhhfjimSd == true){lmhhfjimSd = false;}
      if(ZzyVgBfADM == true){ZzyVgBfADM = false;}
      if(amHNeXNxEB == true){amHNeXNxEB = false;}
      if(tWcmqbeNTd == true){tWcmqbeNTd = false;}
      if(gKtMSqCgBV == true){gKtMSqCgBV = false;}
      if(FtjYSzeTCI == true){FtjYSzeTCI = false;}
      if(WliQjCVLeF == true){WliQjCVLeF = false;}
      if(VSgYfQruum == true){VSgYfQruum = false;}
      if(UgwhuzmETq == true){UgwhuzmETq = false;}
      if(wLKKHuacbQ == true){wLKKHuacbQ = false;}
      if(VqCwAmNKsC == true){VqCwAmNKsC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OTQKJCXXJW
{ 
  void xZruFMBwjp()
  { 
      bool lEtqAotleU = false;
      bool zsPJcnCmFb = false;
      bool GmtWOaVSiu = false;
      bool eyuDIofSQQ = false;
      bool tPNByKconE = false;
      bool WRJWEyKWcZ = false;
      bool SaDipByMAn = false;
      bool IjVogqZrYD = false;
      bool ygedXMnYzK = false;
      bool eIoqxSMeKx = false;
      bool MPnAwmwUoM = false;
      bool YRyZblwYKd = false;
      bool MYNpIAteqN = false;
      bool ethaqhrdhk = false;
      bool PTilkNJPOL = false;
      bool VmTgkmBxqu = false;
      bool rRcbhdjWis = false;
      bool sRqZNrpbFu = false;
      bool pePfqraBpD = false;
      bool gumlsyqHEG = false;
      string IZglqBunFY;
      string dLRNeFZMie;
      string eQtPwrZymG;
      string tlYJkXemVl;
      string gFzoplDoZM;
      string rYlYKqeRpz;
      string axAzermHcG;
      string amqwyYREZJ;
      string njXaNVzzzt;
      string sMwVKiEAKy;
      string zuMSkFhpZk;
      string ZZrfLqZZkJ;
      string smIhNOpUkO;
      string TQtbKrXEoI;
      string mJZOTSUPKT;
      string JeOBEMmeFW;
      string OMEKQIfQVS;
      string TUkcViMsOu;
      string sZJRFoBsWy;
      string ErzRsZEOyK;
      if(IZglqBunFY == zuMSkFhpZk){lEtqAotleU = true;}
      else if(zuMSkFhpZk == IZglqBunFY){MPnAwmwUoM = true;}
      if(dLRNeFZMie == ZZrfLqZZkJ){zsPJcnCmFb = true;}
      else if(ZZrfLqZZkJ == dLRNeFZMie){YRyZblwYKd = true;}
      if(eQtPwrZymG == smIhNOpUkO){GmtWOaVSiu = true;}
      else if(smIhNOpUkO == eQtPwrZymG){MYNpIAteqN = true;}
      if(tlYJkXemVl == TQtbKrXEoI){eyuDIofSQQ = true;}
      else if(TQtbKrXEoI == tlYJkXemVl){ethaqhrdhk = true;}
      if(gFzoplDoZM == mJZOTSUPKT){tPNByKconE = true;}
      else if(mJZOTSUPKT == gFzoplDoZM){PTilkNJPOL = true;}
      if(rYlYKqeRpz == JeOBEMmeFW){WRJWEyKWcZ = true;}
      else if(JeOBEMmeFW == rYlYKqeRpz){VmTgkmBxqu = true;}
      if(axAzermHcG == OMEKQIfQVS){SaDipByMAn = true;}
      else if(OMEKQIfQVS == axAzermHcG){rRcbhdjWis = true;}
      if(amqwyYREZJ == TUkcViMsOu){IjVogqZrYD = true;}
      if(njXaNVzzzt == sZJRFoBsWy){ygedXMnYzK = true;}
      if(sMwVKiEAKy == ErzRsZEOyK){eIoqxSMeKx = true;}
      while(TUkcViMsOu == amqwyYREZJ){sRqZNrpbFu = true;}
      while(sZJRFoBsWy == sZJRFoBsWy){pePfqraBpD = true;}
      while(ErzRsZEOyK == ErzRsZEOyK){gumlsyqHEG = true;}
      if(lEtqAotleU == true){lEtqAotleU = false;}
      if(zsPJcnCmFb == true){zsPJcnCmFb = false;}
      if(GmtWOaVSiu == true){GmtWOaVSiu = false;}
      if(eyuDIofSQQ == true){eyuDIofSQQ = false;}
      if(tPNByKconE == true){tPNByKconE = false;}
      if(WRJWEyKWcZ == true){WRJWEyKWcZ = false;}
      if(SaDipByMAn == true){SaDipByMAn = false;}
      if(IjVogqZrYD == true){IjVogqZrYD = false;}
      if(ygedXMnYzK == true){ygedXMnYzK = false;}
      if(eIoqxSMeKx == true){eIoqxSMeKx = false;}
      if(MPnAwmwUoM == true){MPnAwmwUoM = false;}
      if(YRyZblwYKd == true){YRyZblwYKd = false;}
      if(MYNpIAteqN == true){MYNpIAteqN = false;}
      if(ethaqhrdhk == true){ethaqhrdhk = false;}
      if(PTilkNJPOL == true){PTilkNJPOL = false;}
      if(VmTgkmBxqu == true){VmTgkmBxqu = false;}
      if(rRcbhdjWis == true){rRcbhdjWis = false;}
      if(sRqZNrpbFu == true){sRqZNrpbFu = false;}
      if(pePfqraBpD == true){pePfqraBpD = false;}
      if(gumlsyqHEG == true){gumlsyqHEG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BNRGODPSNQ
{ 
  void BExZVkBojM()
  { 
      bool PfMnSicwtU = false;
      bool knTTtxStHD = false;
      bool nhrSIHxQnB = false;
      bool IkrPKNKEYL = false;
      bool TxoUnabfOz = false;
      bool xAbZlhnteK = false;
      bool lLHrMAuEJg = false;
      bool lNUwiqrWdh = false;
      bool fQBoDHIScc = false;
      bool YQZOYHowIm = false;
      bool eXWhMjefpt = false;
      bool oRPxeUKSoF = false;
      bool qcafHVnGWb = false;
      bool CwOpCHtTbS = false;
      bool bcbimHAbiD = false;
      bool xWGDuCwpZq = false;
      bool wYIOrrDYtW = false;
      bool appVcOArsA = false;
      bool ULWTefoLuA = false;
      bool KEsjWDTNpt = false;
      string SIQFEAmYlQ;
      string oYYnFAEJpN;
      string KEuwlVfyJU;
      string bXxxbFfxsy;
      string NmytYplTBQ;
      string FhqeUNMTTh;
      string ToOQnQabMz;
      string UGQqAWGBWu;
      string jLwsPtgRbO;
      string rLdKdYXgGI;
      string PKHBwyRzey;
      string GLxXyIrVBt;
      string jIzxGIcFNb;
      string yYuOfZEuwQ;
      string bgmcHfOFMV;
      string gCfLXGKJtn;
      string ULlDzgQhrX;
      string DxYjRCUVpa;
      string sXsruwsipZ;
      string OuAIwQtjcZ;
      if(SIQFEAmYlQ == PKHBwyRzey){PfMnSicwtU = true;}
      else if(PKHBwyRzey == SIQFEAmYlQ){eXWhMjefpt = true;}
      if(oYYnFAEJpN == GLxXyIrVBt){knTTtxStHD = true;}
      else if(GLxXyIrVBt == oYYnFAEJpN){oRPxeUKSoF = true;}
      if(KEuwlVfyJU == jIzxGIcFNb){nhrSIHxQnB = true;}
      else if(jIzxGIcFNb == KEuwlVfyJU){qcafHVnGWb = true;}
      if(bXxxbFfxsy == yYuOfZEuwQ){IkrPKNKEYL = true;}
      else if(yYuOfZEuwQ == bXxxbFfxsy){CwOpCHtTbS = true;}
      if(NmytYplTBQ == bgmcHfOFMV){TxoUnabfOz = true;}
      else if(bgmcHfOFMV == NmytYplTBQ){bcbimHAbiD = true;}
      if(FhqeUNMTTh == gCfLXGKJtn){xAbZlhnteK = true;}
      else if(gCfLXGKJtn == FhqeUNMTTh){xWGDuCwpZq = true;}
      if(ToOQnQabMz == ULlDzgQhrX){lLHrMAuEJg = true;}
      else if(ULlDzgQhrX == ToOQnQabMz){wYIOrrDYtW = true;}
      if(UGQqAWGBWu == DxYjRCUVpa){lNUwiqrWdh = true;}
      if(jLwsPtgRbO == sXsruwsipZ){fQBoDHIScc = true;}
      if(rLdKdYXgGI == OuAIwQtjcZ){YQZOYHowIm = true;}
      while(DxYjRCUVpa == UGQqAWGBWu){appVcOArsA = true;}
      while(sXsruwsipZ == sXsruwsipZ){ULWTefoLuA = true;}
      while(OuAIwQtjcZ == OuAIwQtjcZ){KEsjWDTNpt = true;}
      if(PfMnSicwtU == true){PfMnSicwtU = false;}
      if(knTTtxStHD == true){knTTtxStHD = false;}
      if(nhrSIHxQnB == true){nhrSIHxQnB = false;}
      if(IkrPKNKEYL == true){IkrPKNKEYL = false;}
      if(TxoUnabfOz == true){TxoUnabfOz = false;}
      if(xAbZlhnteK == true){xAbZlhnteK = false;}
      if(lLHrMAuEJg == true){lLHrMAuEJg = false;}
      if(lNUwiqrWdh == true){lNUwiqrWdh = false;}
      if(fQBoDHIScc == true){fQBoDHIScc = false;}
      if(YQZOYHowIm == true){YQZOYHowIm = false;}
      if(eXWhMjefpt == true){eXWhMjefpt = false;}
      if(oRPxeUKSoF == true){oRPxeUKSoF = false;}
      if(qcafHVnGWb == true){qcafHVnGWb = false;}
      if(CwOpCHtTbS == true){CwOpCHtTbS = false;}
      if(bcbimHAbiD == true){bcbimHAbiD = false;}
      if(xWGDuCwpZq == true){xWGDuCwpZq = false;}
      if(wYIOrrDYtW == true){wYIOrrDYtW = false;}
      if(appVcOArsA == true){appVcOArsA = false;}
      if(ULWTefoLuA == true){ULWTefoLuA = false;}
      if(KEsjWDTNpt == true){KEsjWDTNpt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NWUNGXVKHM
{ 
  void sDLSQiCNAm()
  { 
      bool MjGHZIpYdd = false;
      bool LzLokmSChI = false;
      bool ExLsLbOGAN = false;
      bool xhdgRqIeTz = false;
      bool NSgqJBbRtw = false;
      bool jouUeOXMwy = false;
      bool siXQfcUdXs = false;
      bool eniitkQUiK = false;
      bool AwLDcIlJCD = false;
      bool TuiUFdtPKf = false;
      bool fxtBRwGMZs = false;
      bool xCclyxWUmX = false;
      bool pLaTZiFNcP = false;
      bool pAKDdCBKlf = false;
      bool BZiIFRSxWH = false;
      bool GZbloUVZah = false;
      bool qsiJDwqZHu = false;
      bool uLiHWQbSOI = false;
      bool QHpVIQYYNJ = false;
      bool faiEoIqZFI = false;
      string noqRgJrJVT;
      string nJBhQdpNkk;
      string ZJugRzpygZ;
      string YslTHeycUl;
      string VjcRsYlAEj;
      string zEcqjdtKAg;
      string TsbwtuXgak;
      string fMszPpgdns;
      string MotxZTQtjc;
      string kncmRcIfWy;
      string AEZSEmKcbt;
      string xPWHTdyeWa;
      string ddxxzrjFgl;
      string gAHorHPbrm;
      string InSofUQVSn;
      string ecIuTWHcTM;
      string CFuoMwIgOL;
      string OWmjRpLHQd;
      string aOlsHEqLUV;
      string AmHfjDrccj;
      if(noqRgJrJVT == AEZSEmKcbt){MjGHZIpYdd = true;}
      else if(AEZSEmKcbt == noqRgJrJVT){fxtBRwGMZs = true;}
      if(nJBhQdpNkk == xPWHTdyeWa){LzLokmSChI = true;}
      else if(xPWHTdyeWa == nJBhQdpNkk){xCclyxWUmX = true;}
      if(ZJugRzpygZ == ddxxzrjFgl){ExLsLbOGAN = true;}
      else if(ddxxzrjFgl == ZJugRzpygZ){pLaTZiFNcP = true;}
      if(YslTHeycUl == gAHorHPbrm){xhdgRqIeTz = true;}
      else if(gAHorHPbrm == YslTHeycUl){pAKDdCBKlf = true;}
      if(VjcRsYlAEj == InSofUQVSn){NSgqJBbRtw = true;}
      else if(InSofUQVSn == VjcRsYlAEj){BZiIFRSxWH = true;}
      if(zEcqjdtKAg == ecIuTWHcTM){jouUeOXMwy = true;}
      else if(ecIuTWHcTM == zEcqjdtKAg){GZbloUVZah = true;}
      if(TsbwtuXgak == CFuoMwIgOL){siXQfcUdXs = true;}
      else if(CFuoMwIgOL == TsbwtuXgak){qsiJDwqZHu = true;}
      if(fMszPpgdns == OWmjRpLHQd){eniitkQUiK = true;}
      if(MotxZTQtjc == aOlsHEqLUV){AwLDcIlJCD = true;}
      if(kncmRcIfWy == AmHfjDrccj){TuiUFdtPKf = true;}
      while(OWmjRpLHQd == fMszPpgdns){uLiHWQbSOI = true;}
      while(aOlsHEqLUV == aOlsHEqLUV){QHpVIQYYNJ = true;}
      while(AmHfjDrccj == AmHfjDrccj){faiEoIqZFI = true;}
      if(MjGHZIpYdd == true){MjGHZIpYdd = false;}
      if(LzLokmSChI == true){LzLokmSChI = false;}
      if(ExLsLbOGAN == true){ExLsLbOGAN = false;}
      if(xhdgRqIeTz == true){xhdgRqIeTz = false;}
      if(NSgqJBbRtw == true){NSgqJBbRtw = false;}
      if(jouUeOXMwy == true){jouUeOXMwy = false;}
      if(siXQfcUdXs == true){siXQfcUdXs = false;}
      if(eniitkQUiK == true){eniitkQUiK = false;}
      if(AwLDcIlJCD == true){AwLDcIlJCD = false;}
      if(TuiUFdtPKf == true){TuiUFdtPKf = false;}
      if(fxtBRwGMZs == true){fxtBRwGMZs = false;}
      if(xCclyxWUmX == true){xCclyxWUmX = false;}
      if(pLaTZiFNcP == true){pLaTZiFNcP = false;}
      if(pAKDdCBKlf == true){pAKDdCBKlf = false;}
      if(BZiIFRSxWH == true){BZiIFRSxWH = false;}
      if(GZbloUVZah == true){GZbloUVZah = false;}
      if(qsiJDwqZHu == true){qsiJDwqZHu = false;}
      if(uLiHWQbSOI == true){uLiHWQbSOI = false;}
      if(QHpVIQYYNJ == true){QHpVIQYYNJ = false;}
      if(faiEoIqZFI == true){faiEoIqZFI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CVYNAVVWWI
{ 
  void lHahtShtGU()
  { 
      bool OuILaymuil = false;
      bool jwfXuXuKSb = false;
      bool foKJhPTDyF = false;
      bool WOkwLETzzt = false;
      bool ctyXUUqewC = false;
      bool IpPAnYjycL = false;
      bool eixdOULjLq = false;
      bool DwLYzlpmoN = false;
      bool XnHuxfzjHm = false;
      bool TUVmOsxUIz = false;
      bool IHSFzMepBC = false;
      bool hwJPnqbBgh = false;
      bool VbbYWhZXJF = false;
      bool oUeoEPUBwj = false;
      bool BJhEFWLGzX = false;
      bool RunZdfethQ = false;
      bool lGfuNMRTKX = false;
      bool WZEwgMtWTz = false;
      bool WTYlnsfFLY = false;
      bool kZTfxtclqr = false;
      string eHtOGkNNTm;
      string LXPFctIyPx;
      string WVPVAIDYdu;
      string dsJVddblmE;
      string kDrYjnmeKd;
      string VojnJIqYNy;
      string gnjYJSNtaz;
      string SWLLMSdqkM;
      string VYOzYnJVLU;
      string lDXnWwTTOR;
      string ZsDLLycYbq;
      string eAmBcIZjzU;
      string IrhGMPyWnP;
      string zJbSmbUCnZ;
      string KktUolPWLO;
      string WiUsJhKxyu;
      string zGjLqUXUOA;
      string mXwYKXxNQZ;
      string zbKZsPohJE;
      string RXDrZEAhcU;
      if(eHtOGkNNTm == ZsDLLycYbq){OuILaymuil = true;}
      else if(ZsDLLycYbq == eHtOGkNNTm){IHSFzMepBC = true;}
      if(LXPFctIyPx == eAmBcIZjzU){jwfXuXuKSb = true;}
      else if(eAmBcIZjzU == LXPFctIyPx){hwJPnqbBgh = true;}
      if(WVPVAIDYdu == IrhGMPyWnP){foKJhPTDyF = true;}
      else if(IrhGMPyWnP == WVPVAIDYdu){VbbYWhZXJF = true;}
      if(dsJVddblmE == zJbSmbUCnZ){WOkwLETzzt = true;}
      else if(zJbSmbUCnZ == dsJVddblmE){oUeoEPUBwj = true;}
      if(kDrYjnmeKd == KktUolPWLO){ctyXUUqewC = true;}
      else if(KktUolPWLO == kDrYjnmeKd){BJhEFWLGzX = true;}
      if(VojnJIqYNy == WiUsJhKxyu){IpPAnYjycL = true;}
      else if(WiUsJhKxyu == VojnJIqYNy){RunZdfethQ = true;}
      if(gnjYJSNtaz == zGjLqUXUOA){eixdOULjLq = true;}
      else if(zGjLqUXUOA == gnjYJSNtaz){lGfuNMRTKX = true;}
      if(SWLLMSdqkM == mXwYKXxNQZ){DwLYzlpmoN = true;}
      if(VYOzYnJVLU == zbKZsPohJE){XnHuxfzjHm = true;}
      if(lDXnWwTTOR == RXDrZEAhcU){TUVmOsxUIz = true;}
      while(mXwYKXxNQZ == SWLLMSdqkM){WZEwgMtWTz = true;}
      while(zbKZsPohJE == zbKZsPohJE){WTYlnsfFLY = true;}
      while(RXDrZEAhcU == RXDrZEAhcU){kZTfxtclqr = true;}
      if(OuILaymuil == true){OuILaymuil = false;}
      if(jwfXuXuKSb == true){jwfXuXuKSb = false;}
      if(foKJhPTDyF == true){foKJhPTDyF = false;}
      if(WOkwLETzzt == true){WOkwLETzzt = false;}
      if(ctyXUUqewC == true){ctyXUUqewC = false;}
      if(IpPAnYjycL == true){IpPAnYjycL = false;}
      if(eixdOULjLq == true){eixdOULjLq = false;}
      if(DwLYzlpmoN == true){DwLYzlpmoN = false;}
      if(XnHuxfzjHm == true){XnHuxfzjHm = false;}
      if(TUVmOsxUIz == true){TUVmOsxUIz = false;}
      if(IHSFzMepBC == true){IHSFzMepBC = false;}
      if(hwJPnqbBgh == true){hwJPnqbBgh = false;}
      if(VbbYWhZXJF == true){VbbYWhZXJF = false;}
      if(oUeoEPUBwj == true){oUeoEPUBwj = false;}
      if(BJhEFWLGzX == true){BJhEFWLGzX = false;}
      if(RunZdfethQ == true){RunZdfethQ = false;}
      if(lGfuNMRTKX == true){lGfuNMRTKX = false;}
      if(WZEwgMtWTz == true){WZEwgMtWTz = false;}
      if(WTYlnsfFLY == true){WTYlnsfFLY = false;}
      if(kZTfxtclqr == true){kZTfxtclqr = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LLXSBAZTHC
{ 
  void rVNZwMTrIi()
  { 
      bool GSQZprXGXp = false;
      bool RxgujrzPdZ = false;
      bool nmHIVTHcbB = false;
      bool JwNZznNqFa = false;
      bool fTijcUFVUx = false;
      bool XDaPcSKHEV = false;
      bool kteWgUiGuw = false;
      bool eHJCsUEXeA = false;
      bool OkIdcFUyxf = false;
      bool cFmiplnMDI = false;
      bool kyZXSQsYVH = false;
      bool OmcDHzkAsa = false;
      bool zkJPTYTaLG = false;
      bool frTWOPKkLM = false;
      bool MwboRocjbq = false;
      bool AmScIAoKDs = false;
      bool wSNPbWSGnq = false;
      bool oAofODITxd = false;
      bool wjzUGrYqNN = false;
      bool azgRQsiMNu = false;
      string fPtzjiDAuX;
      string uzPTHOTPxS;
      string DRJtxnLlci;
      string dBKnGDdlPm;
      string ZIPAWLyGLi;
      string BuPahOFACw;
      string WRXLstnbME;
      string khzJstCIWk;
      string oEZsgikjGq;
      string xdjTPIjDGc;
      string XWsHxTuFhm;
      string MwcQGRqdeK;
      string IEVVGGxetj;
      string JyCtNSKSwM;
      string meCSeAATNx;
      string QFYuTSAXBp;
      string WQLxcNthCn;
      string HYhRXcBDqF;
      string GdRwTDZGat;
      string RmRFpWTFoQ;
      if(fPtzjiDAuX == XWsHxTuFhm){GSQZprXGXp = true;}
      else if(XWsHxTuFhm == fPtzjiDAuX){kyZXSQsYVH = true;}
      if(uzPTHOTPxS == MwcQGRqdeK){RxgujrzPdZ = true;}
      else if(MwcQGRqdeK == uzPTHOTPxS){OmcDHzkAsa = true;}
      if(DRJtxnLlci == IEVVGGxetj){nmHIVTHcbB = true;}
      else if(IEVVGGxetj == DRJtxnLlci){zkJPTYTaLG = true;}
      if(dBKnGDdlPm == JyCtNSKSwM){JwNZznNqFa = true;}
      else if(JyCtNSKSwM == dBKnGDdlPm){frTWOPKkLM = true;}
      if(ZIPAWLyGLi == meCSeAATNx){fTijcUFVUx = true;}
      else if(meCSeAATNx == ZIPAWLyGLi){MwboRocjbq = true;}
      if(BuPahOFACw == QFYuTSAXBp){XDaPcSKHEV = true;}
      else if(QFYuTSAXBp == BuPahOFACw){AmScIAoKDs = true;}
      if(WRXLstnbME == WQLxcNthCn){kteWgUiGuw = true;}
      else if(WQLxcNthCn == WRXLstnbME){wSNPbWSGnq = true;}
      if(khzJstCIWk == HYhRXcBDqF){eHJCsUEXeA = true;}
      if(oEZsgikjGq == GdRwTDZGat){OkIdcFUyxf = true;}
      if(xdjTPIjDGc == RmRFpWTFoQ){cFmiplnMDI = true;}
      while(HYhRXcBDqF == khzJstCIWk){oAofODITxd = true;}
      while(GdRwTDZGat == GdRwTDZGat){wjzUGrYqNN = true;}
      while(RmRFpWTFoQ == RmRFpWTFoQ){azgRQsiMNu = true;}
      if(GSQZprXGXp == true){GSQZprXGXp = false;}
      if(RxgujrzPdZ == true){RxgujrzPdZ = false;}
      if(nmHIVTHcbB == true){nmHIVTHcbB = false;}
      if(JwNZznNqFa == true){JwNZznNqFa = false;}
      if(fTijcUFVUx == true){fTijcUFVUx = false;}
      if(XDaPcSKHEV == true){XDaPcSKHEV = false;}
      if(kteWgUiGuw == true){kteWgUiGuw = false;}
      if(eHJCsUEXeA == true){eHJCsUEXeA = false;}
      if(OkIdcFUyxf == true){OkIdcFUyxf = false;}
      if(cFmiplnMDI == true){cFmiplnMDI = false;}
      if(kyZXSQsYVH == true){kyZXSQsYVH = false;}
      if(OmcDHzkAsa == true){OmcDHzkAsa = false;}
      if(zkJPTYTaLG == true){zkJPTYTaLG = false;}
      if(frTWOPKkLM == true){frTWOPKkLM = false;}
      if(MwboRocjbq == true){MwboRocjbq = false;}
      if(AmScIAoKDs == true){AmScIAoKDs = false;}
      if(wSNPbWSGnq == true){wSNPbWSGnq = false;}
      if(oAofODITxd == true){oAofODITxd = false;}
      if(wjzUGrYqNN == true){wjzUGrYqNN = false;}
      if(azgRQsiMNu == true){azgRQsiMNu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YSLKDEHNTS
{ 
  void KXprPifwom()
  { 
      bool fEfPGFfqha = false;
      bool IqZTKBfWtH = false;
      bool wYHOwSSfuI = false;
      bool aPkoyFKWzL = false;
      bool oMIjPJnRGW = false;
      bool jYICPHxRTn = false;
      bool lAuZArFkUE = false;
      bool pJXhSjQXzl = false;
      bool eBYYXBHiYq = false;
      bool IsCLYnQhiX = false;
      bool DCxCgaBZie = false;
      bool qZRgIkaWCy = false;
      bool jWMcwwGLId = false;
      bool ThnbhbEWrx = false;
      bool AhzsAhaVCn = false;
      bool NtfkmgpTFj = false;
      bool ToQFLIlpzg = false;
      bool gDeEyCWScN = false;
      bool UkJUPYBCnA = false;
      bool kjQHcKQtgq = false;
      string fVptQKQFwi;
      string euCkbTTtYR;
      string JIrEzWHUcq;
      string XgMGxolBtP;
      string sszcwqObLk;
      string WUgLrWdlts;
      string DwcELxiuml;
      string hRAIXPIPIH;
      string HeIqUDzBOZ;
      string KAGAMxPuQy;
      string IToYDETYmb;
      string NOcmlXmljI;
      string yggNahLAua;
      string zNyKznpimQ;
      string iZOlLwWTNo;
      string JdKHzalayV;
      string rSdLczPSmy;
      string hDyTYXSuBe;
      string KqlGRUDqKX;
      string BOkRXQwCJM;
      if(fVptQKQFwi == IToYDETYmb){fEfPGFfqha = true;}
      else if(IToYDETYmb == fVptQKQFwi){DCxCgaBZie = true;}
      if(euCkbTTtYR == NOcmlXmljI){IqZTKBfWtH = true;}
      else if(NOcmlXmljI == euCkbTTtYR){qZRgIkaWCy = true;}
      if(JIrEzWHUcq == yggNahLAua){wYHOwSSfuI = true;}
      else if(yggNahLAua == JIrEzWHUcq){jWMcwwGLId = true;}
      if(XgMGxolBtP == zNyKznpimQ){aPkoyFKWzL = true;}
      else if(zNyKznpimQ == XgMGxolBtP){ThnbhbEWrx = true;}
      if(sszcwqObLk == iZOlLwWTNo){oMIjPJnRGW = true;}
      else if(iZOlLwWTNo == sszcwqObLk){AhzsAhaVCn = true;}
      if(WUgLrWdlts == JdKHzalayV){jYICPHxRTn = true;}
      else if(JdKHzalayV == WUgLrWdlts){NtfkmgpTFj = true;}
      if(DwcELxiuml == rSdLczPSmy){lAuZArFkUE = true;}
      else if(rSdLczPSmy == DwcELxiuml){ToQFLIlpzg = true;}
      if(hRAIXPIPIH == hDyTYXSuBe){pJXhSjQXzl = true;}
      if(HeIqUDzBOZ == KqlGRUDqKX){eBYYXBHiYq = true;}
      if(KAGAMxPuQy == BOkRXQwCJM){IsCLYnQhiX = true;}
      while(hDyTYXSuBe == hRAIXPIPIH){gDeEyCWScN = true;}
      while(KqlGRUDqKX == KqlGRUDqKX){UkJUPYBCnA = true;}
      while(BOkRXQwCJM == BOkRXQwCJM){kjQHcKQtgq = true;}
      if(fEfPGFfqha == true){fEfPGFfqha = false;}
      if(IqZTKBfWtH == true){IqZTKBfWtH = false;}
      if(wYHOwSSfuI == true){wYHOwSSfuI = false;}
      if(aPkoyFKWzL == true){aPkoyFKWzL = false;}
      if(oMIjPJnRGW == true){oMIjPJnRGW = false;}
      if(jYICPHxRTn == true){jYICPHxRTn = false;}
      if(lAuZArFkUE == true){lAuZArFkUE = false;}
      if(pJXhSjQXzl == true){pJXhSjQXzl = false;}
      if(eBYYXBHiYq == true){eBYYXBHiYq = false;}
      if(IsCLYnQhiX == true){IsCLYnQhiX = false;}
      if(DCxCgaBZie == true){DCxCgaBZie = false;}
      if(qZRgIkaWCy == true){qZRgIkaWCy = false;}
      if(jWMcwwGLId == true){jWMcwwGLId = false;}
      if(ThnbhbEWrx == true){ThnbhbEWrx = false;}
      if(AhzsAhaVCn == true){AhzsAhaVCn = false;}
      if(NtfkmgpTFj == true){NtfkmgpTFj = false;}
      if(ToQFLIlpzg == true){ToQFLIlpzg = false;}
      if(gDeEyCWScN == true){gDeEyCWScN = false;}
      if(UkJUPYBCnA == true){UkJUPYBCnA = false;}
      if(kjQHcKQtgq == true){kjQHcKQtgq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QXXDALLELW
{ 
  void RboaDdqYFL()
  { 
      bool yNYJEpfPKR = false;
      bool uSAbgDowPh = false;
      bool cUyZwnZDRH = false;
      bool CMHMGAZWus = false;
      bool GzmixwYKKc = false;
      bool PGDlydPyNQ = false;
      bool PcxCPiLnUf = false;
      bool cRAVbcgObz = false;
      bool PPrnrwJVNc = false;
      bool kbJdccBjTO = false;
      bool kjpeAyFgJV = false;
      bool wAWOBAjJKT = false;
      bool YoZqtAHVry = false;
      bool axuMNeYNms = false;
      bool pcLrgkEucc = false;
      bool srZzpuKyjA = false;
      bool qjnLtzEJAW = false;
      bool aVVRcwdhJI = false;
      bool wMsPFhWwWR = false;
      bool UZdmCGnAxh = false;
      string fDKCKgEqIN;
      string HNPkxVpfmN;
      string YhioTTzzxx;
      string XoNbmXHQja;
      string pCHCqYyqbl;
      string TVaJExUViq;
      string hEsLpRAmxY;
      string OZwLXWagxt;
      string UHWKcBODWp;
      string DlgrrkIwLZ;
      string YKtCXtZPlh;
      string WmXoVZghhn;
      string DxQGVcrDSm;
      string NdUXUKMTmi;
      string ayRnTQNrVS;
      string DaOXWbtngu;
      string CWwyUrRATL;
      string NNuyMEfbmF;
      string OzgAilPxBs;
      string PYmqfECSTa;
      if(fDKCKgEqIN == YKtCXtZPlh){yNYJEpfPKR = true;}
      else if(YKtCXtZPlh == fDKCKgEqIN){kjpeAyFgJV = true;}
      if(HNPkxVpfmN == WmXoVZghhn){uSAbgDowPh = true;}
      else if(WmXoVZghhn == HNPkxVpfmN){wAWOBAjJKT = true;}
      if(YhioTTzzxx == DxQGVcrDSm){cUyZwnZDRH = true;}
      else if(DxQGVcrDSm == YhioTTzzxx){YoZqtAHVry = true;}
      if(XoNbmXHQja == NdUXUKMTmi){CMHMGAZWus = true;}
      else if(NdUXUKMTmi == XoNbmXHQja){axuMNeYNms = true;}
      if(pCHCqYyqbl == ayRnTQNrVS){GzmixwYKKc = true;}
      else if(ayRnTQNrVS == pCHCqYyqbl){pcLrgkEucc = true;}
      if(TVaJExUViq == DaOXWbtngu){PGDlydPyNQ = true;}
      else if(DaOXWbtngu == TVaJExUViq){srZzpuKyjA = true;}
      if(hEsLpRAmxY == CWwyUrRATL){PcxCPiLnUf = true;}
      else if(CWwyUrRATL == hEsLpRAmxY){qjnLtzEJAW = true;}
      if(OZwLXWagxt == NNuyMEfbmF){cRAVbcgObz = true;}
      if(UHWKcBODWp == OzgAilPxBs){PPrnrwJVNc = true;}
      if(DlgrrkIwLZ == PYmqfECSTa){kbJdccBjTO = true;}
      while(NNuyMEfbmF == OZwLXWagxt){aVVRcwdhJI = true;}
      while(OzgAilPxBs == OzgAilPxBs){wMsPFhWwWR = true;}
      while(PYmqfECSTa == PYmqfECSTa){UZdmCGnAxh = true;}
      if(yNYJEpfPKR == true){yNYJEpfPKR = false;}
      if(uSAbgDowPh == true){uSAbgDowPh = false;}
      if(cUyZwnZDRH == true){cUyZwnZDRH = false;}
      if(CMHMGAZWus == true){CMHMGAZWus = false;}
      if(GzmixwYKKc == true){GzmixwYKKc = false;}
      if(PGDlydPyNQ == true){PGDlydPyNQ = false;}
      if(PcxCPiLnUf == true){PcxCPiLnUf = false;}
      if(cRAVbcgObz == true){cRAVbcgObz = false;}
      if(PPrnrwJVNc == true){PPrnrwJVNc = false;}
      if(kbJdccBjTO == true){kbJdccBjTO = false;}
      if(kjpeAyFgJV == true){kjpeAyFgJV = false;}
      if(wAWOBAjJKT == true){wAWOBAjJKT = false;}
      if(YoZqtAHVry == true){YoZqtAHVry = false;}
      if(axuMNeYNms == true){axuMNeYNms = false;}
      if(pcLrgkEucc == true){pcLrgkEucc = false;}
      if(srZzpuKyjA == true){srZzpuKyjA = false;}
      if(qjnLtzEJAW == true){qjnLtzEJAW = false;}
      if(aVVRcwdhJI == true){aVVRcwdhJI = false;}
      if(wMsPFhWwWR == true){wMsPFhWwWR = false;}
      if(UZdmCGnAxh == true){UZdmCGnAxh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ILPIDKRUMP
{ 
  void dEcOhVOoyh()
  { 
      bool uRtLqYqlRm = false;
      bool idkMUPXkwm = false;
      bool VtjqunGzEr = false;
      bool uGXgfCydTs = false;
      bool tUdVQxqUde = false;
      bool kSFpfBhDQC = false;
      bool SqhYrcFDgS = false;
      bool RtQDrBPNZx = false;
      bool xkVZHbAgBe = false;
      bool TtMRaCmFdn = false;
      bool rmnKMpBCye = false;
      bool kTdTEgVIQP = false;
      bool cXCCyXgzcp = false;
      bool NKjlFJwKMQ = false;
      bool ZyiVsAPsMs = false;
      bool fenNKTklMo = false;
      bool JBYHQZjrOu = false;
      bool kLuAoeofIy = false;
      bool HnmjJiWVak = false;
      bool YKxXHXYaFT = false;
      string PwGQyiTpgK;
      string psIUQtbNNL;
      string ROgkRfkjzl;
      string cYHCDISXeV;
      string IQhYDEoCKt;
      string sUHwABOUyS;
      string yjpugRkFRH;
      string bEiqDBfHgM;
      string piYFrIQKBB;
      string sxYLXXZmrl;
      string SikCSolPsz;
      string VGmlcPKkCY;
      string AaDPNdfXTe;
      string arPusGxWmD;
      string WARCflJmhI;
      string MokTlreUnn;
      string INwOYdqrrV;
      string RnyyBzIBjb;
      string TsoKRkHwIA;
      string achytIECdC;
      if(PwGQyiTpgK == SikCSolPsz){uRtLqYqlRm = true;}
      else if(SikCSolPsz == PwGQyiTpgK){rmnKMpBCye = true;}
      if(psIUQtbNNL == VGmlcPKkCY){idkMUPXkwm = true;}
      else if(VGmlcPKkCY == psIUQtbNNL){kTdTEgVIQP = true;}
      if(ROgkRfkjzl == AaDPNdfXTe){VtjqunGzEr = true;}
      else if(AaDPNdfXTe == ROgkRfkjzl){cXCCyXgzcp = true;}
      if(cYHCDISXeV == arPusGxWmD){uGXgfCydTs = true;}
      else if(arPusGxWmD == cYHCDISXeV){NKjlFJwKMQ = true;}
      if(IQhYDEoCKt == WARCflJmhI){tUdVQxqUde = true;}
      else if(WARCflJmhI == IQhYDEoCKt){ZyiVsAPsMs = true;}
      if(sUHwABOUyS == MokTlreUnn){kSFpfBhDQC = true;}
      else if(MokTlreUnn == sUHwABOUyS){fenNKTklMo = true;}
      if(yjpugRkFRH == INwOYdqrrV){SqhYrcFDgS = true;}
      else if(INwOYdqrrV == yjpugRkFRH){JBYHQZjrOu = true;}
      if(bEiqDBfHgM == RnyyBzIBjb){RtQDrBPNZx = true;}
      if(piYFrIQKBB == TsoKRkHwIA){xkVZHbAgBe = true;}
      if(sxYLXXZmrl == achytIECdC){TtMRaCmFdn = true;}
      while(RnyyBzIBjb == bEiqDBfHgM){kLuAoeofIy = true;}
      while(TsoKRkHwIA == TsoKRkHwIA){HnmjJiWVak = true;}
      while(achytIECdC == achytIECdC){YKxXHXYaFT = true;}
      if(uRtLqYqlRm == true){uRtLqYqlRm = false;}
      if(idkMUPXkwm == true){idkMUPXkwm = false;}
      if(VtjqunGzEr == true){VtjqunGzEr = false;}
      if(uGXgfCydTs == true){uGXgfCydTs = false;}
      if(tUdVQxqUde == true){tUdVQxqUde = false;}
      if(kSFpfBhDQC == true){kSFpfBhDQC = false;}
      if(SqhYrcFDgS == true){SqhYrcFDgS = false;}
      if(RtQDrBPNZx == true){RtQDrBPNZx = false;}
      if(xkVZHbAgBe == true){xkVZHbAgBe = false;}
      if(TtMRaCmFdn == true){TtMRaCmFdn = false;}
      if(rmnKMpBCye == true){rmnKMpBCye = false;}
      if(kTdTEgVIQP == true){kTdTEgVIQP = false;}
      if(cXCCyXgzcp == true){cXCCyXgzcp = false;}
      if(NKjlFJwKMQ == true){NKjlFJwKMQ = false;}
      if(ZyiVsAPsMs == true){ZyiVsAPsMs = false;}
      if(fenNKTklMo == true){fenNKTklMo = false;}
      if(JBYHQZjrOu == true){JBYHQZjrOu = false;}
      if(kLuAoeofIy == true){kLuAoeofIy = false;}
      if(HnmjJiWVak == true){HnmjJiWVak = false;}
      if(YKxXHXYaFT == true){YKxXHXYaFT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SRUOKBTGHG
{ 
  void leoytmnqfG()
  { 
      bool QjenqIlEZD = false;
      bool tkYKgTVlbx = false;
      bool jskGtuWoFK = false;
      bool NtCVmnEQrp = false;
      bool LVfbEoRZtR = false;
      bool bsdzoXyXtu = false;
      bool lxyPpzhCXr = false;
      bool OEslkNMJBE = false;
      bool dwtmhUQCgO = false;
      bool pfWxEOXQkF = false;
      bool HPCBCWzqNa = false;
      bool LoWoQoCKgV = false;
      bool heVICihugW = false;
      bool puXwxOtsjK = false;
      bool UYoGDBJgfO = false;
      bool LMiPWbtszT = false;
      bool nDKqjPQaqC = false;
      bool tJZVBJpOEB = false;
      bool ZBPSKdMIYV = false;
      bool UAsQWROyVn = false;
      string lXxbLNeOXI;
      string jHGogZGMUt;
      string BJYkGrGrIi;
      string KGAcrFGEIg;
      string JEOQTjFwKy;
      string YuVbqbyLUS;
      string HmGoycGcwa;
      string TCEmkxuIRV;
      string mKgKKPjYlQ;
      string GtioZVxZRp;
      string cbzidVpiqt;
      string fpsyfwFweh;
      string shqfUYTOMP;
      string zVpbsKKyfn;
      string YBhqjQBjJc;
      string PBuiwtjTOE;
      string CaVbtKzQFU;
      string lGPpIrAzuD;
      string XiFjzxZGfy;
      string ZpjakyeFxY;
      if(lXxbLNeOXI == cbzidVpiqt){QjenqIlEZD = true;}
      else if(cbzidVpiqt == lXxbLNeOXI){HPCBCWzqNa = true;}
      if(jHGogZGMUt == fpsyfwFweh){tkYKgTVlbx = true;}
      else if(fpsyfwFweh == jHGogZGMUt){LoWoQoCKgV = true;}
      if(BJYkGrGrIi == shqfUYTOMP){jskGtuWoFK = true;}
      else if(shqfUYTOMP == BJYkGrGrIi){heVICihugW = true;}
      if(KGAcrFGEIg == zVpbsKKyfn){NtCVmnEQrp = true;}
      else if(zVpbsKKyfn == KGAcrFGEIg){puXwxOtsjK = true;}
      if(JEOQTjFwKy == YBhqjQBjJc){LVfbEoRZtR = true;}
      else if(YBhqjQBjJc == JEOQTjFwKy){UYoGDBJgfO = true;}
      if(YuVbqbyLUS == PBuiwtjTOE){bsdzoXyXtu = true;}
      else if(PBuiwtjTOE == YuVbqbyLUS){LMiPWbtszT = true;}
      if(HmGoycGcwa == CaVbtKzQFU){lxyPpzhCXr = true;}
      else if(CaVbtKzQFU == HmGoycGcwa){nDKqjPQaqC = true;}
      if(TCEmkxuIRV == lGPpIrAzuD){OEslkNMJBE = true;}
      if(mKgKKPjYlQ == XiFjzxZGfy){dwtmhUQCgO = true;}
      if(GtioZVxZRp == ZpjakyeFxY){pfWxEOXQkF = true;}
      while(lGPpIrAzuD == TCEmkxuIRV){tJZVBJpOEB = true;}
      while(XiFjzxZGfy == XiFjzxZGfy){ZBPSKdMIYV = true;}
      while(ZpjakyeFxY == ZpjakyeFxY){UAsQWROyVn = true;}
      if(QjenqIlEZD == true){QjenqIlEZD = false;}
      if(tkYKgTVlbx == true){tkYKgTVlbx = false;}
      if(jskGtuWoFK == true){jskGtuWoFK = false;}
      if(NtCVmnEQrp == true){NtCVmnEQrp = false;}
      if(LVfbEoRZtR == true){LVfbEoRZtR = false;}
      if(bsdzoXyXtu == true){bsdzoXyXtu = false;}
      if(lxyPpzhCXr == true){lxyPpzhCXr = false;}
      if(OEslkNMJBE == true){OEslkNMJBE = false;}
      if(dwtmhUQCgO == true){dwtmhUQCgO = false;}
      if(pfWxEOXQkF == true){pfWxEOXQkF = false;}
      if(HPCBCWzqNa == true){HPCBCWzqNa = false;}
      if(LoWoQoCKgV == true){LoWoQoCKgV = false;}
      if(heVICihugW == true){heVICihugW = false;}
      if(puXwxOtsjK == true){puXwxOtsjK = false;}
      if(UYoGDBJgfO == true){UYoGDBJgfO = false;}
      if(LMiPWbtszT == true){LMiPWbtszT = false;}
      if(nDKqjPQaqC == true){nDKqjPQaqC = false;}
      if(tJZVBJpOEB == true){tJZVBJpOEB = false;}
      if(ZBPSKdMIYV == true){ZBPSKdMIYV = false;}
      if(UAsQWROyVn == true){UAsQWROyVn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ECKUXIVSJD
{ 
  void lhgSwtpwqU()
  { 
      bool muYOTkNKGu = false;
      bool NoMtBHakjh = false;
      bool hYEtCNfOGs = false;
      bool TzlYORtJVe = false;
      bool qRlsYDxyZy = false;
      bool PdHBVOpeMG = false;
      bool EzEMaNDnfj = false;
      bool MROEpPmLUy = false;
      bool PgnHcorjCA = false;
      bool aPxPZqNYRK = false;
      bool zJSplyPCKG = false;
      bool aeJNpWLwil = false;
      bool VTkeJxTzez = false;
      bool BNsJyGOxVe = false;
      bool ZoYwYuFjYC = false;
      bool eRofnfXmgn = false;
      bool dQXLEMlzPi = false;
      bool IOZELgtoFN = false;
      bool rjZbTjYNVR = false;
      bool iaPgAJQNqM = false;
      string ukchZlYFTL;
      string fuPabduOMl;
      string OPexZRhOUp;
      string GFsGGNdoSQ;
      string kVwZcqLBSO;
      string GeFaoeGeIH;
      string DKkdEZBITO;
      string zFuuAsTEUh;
      string GguFibARRa;
      string GyfLNZjSUD;
      string oChEPAjJZu;
      string LBKZAVPfuc;
      string YiqrHBZihZ;
      string IAjRsEzpph;
      string qpqVPFnGKq;
      string KDXqdFKoxN;
      string aAbzmKRrgw;
      string WYpMkiqUcC;
      string MTYpPEcrnm;
      string sJsbGqOriE;
      if(ukchZlYFTL == oChEPAjJZu){muYOTkNKGu = true;}
      else if(oChEPAjJZu == ukchZlYFTL){zJSplyPCKG = true;}
      if(fuPabduOMl == LBKZAVPfuc){NoMtBHakjh = true;}
      else if(LBKZAVPfuc == fuPabduOMl){aeJNpWLwil = true;}
      if(OPexZRhOUp == YiqrHBZihZ){hYEtCNfOGs = true;}
      else if(YiqrHBZihZ == OPexZRhOUp){VTkeJxTzez = true;}
      if(GFsGGNdoSQ == IAjRsEzpph){TzlYORtJVe = true;}
      else if(IAjRsEzpph == GFsGGNdoSQ){BNsJyGOxVe = true;}
      if(kVwZcqLBSO == qpqVPFnGKq){qRlsYDxyZy = true;}
      else if(qpqVPFnGKq == kVwZcqLBSO){ZoYwYuFjYC = true;}
      if(GeFaoeGeIH == KDXqdFKoxN){PdHBVOpeMG = true;}
      else if(KDXqdFKoxN == GeFaoeGeIH){eRofnfXmgn = true;}
      if(DKkdEZBITO == aAbzmKRrgw){EzEMaNDnfj = true;}
      else if(aAbzmKRrgw == DKkdEZBITO){dQXLEMlzPi = true;}
      if(zFuuAsTEUh == WYpMkiqUcC){MROEpPmLUy = true;}
      if(GguFibARRa == MTYpPEcrnm){PgnHcorjCA = true;}
      if(GyfLNZjSUD == sJsbGqOriE){aPxPZqNYRK = true;}
      while(WYpMkiqUcC == zFuuAsTEUh){IOZELgtoFN = true;}
      while(MTYpPEcrnm == MTYpPEcrnm){rjZbTjYNVR = true;}
      while(sJsbGqOriE == sJsbGqOriE){iaPgAJQNqM = true;}
      if(muYOTkNKGu == true){muYOTkNKGu = false;}
      if(NoMtBHakjh == true){NoMtBHakjh = false;}
      if(hYEtCNfOGs == true){hYEtCNfOGs = false;}
      if(TzlYORtJVe == true){TzlYORtJVe = false;}
      if(qRlsYDxyZy == true){qRlsYDxyZy = false;}
      if(PdHBVOpeMG == true){PdHBVOpeMG = false;}
      if(EzEMaNDnfj == true){EzEMaNDnfj = false;}
      if(MROEpPmLUy == true){MROEpPmLUy = false;}
      if(PgnHcorjCA == true){PgnHcorjCA = false;}
      if(aPxPZqNYRK == true){aPxPZqNYRK = false;}
      if(zJSplyPCKG == true){zJSplyPCKG = false;}
      if(aeJNpWLwil == true){aeJNpWLwil = false;}
      if(VTkeJxTzez == true){VTkeJxTzez = false;}
      if(BNsJyGOxVe == true){BNsJyGOxVe = false;}
      if(ZoYwYuFjYC == true){ZoYwYuFjYC = false;}
      if(eRofnfXmgn == true){eRofnfXmgn = false;}
      if(dQXLEMlzPi == true){dQXLEMlzPi = false;}
      if(IOZELgtoFN == true){IOZELgtoFN = false;}
      if(rjZbTjYNVR == true){rjZbTjYNVR = false;}
      if(iaPgAJQNqM == true){iaPgAJQNqM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZNPUNOCDWP
{ 
  void mPGIPmfGFx()
  { 
      bool GjITPIzmQY = false;
      bool AWhbZjEDRn = false;
      bool QuwUadfNAk = false;
      bool cfnrIgUBkT = false;
      bool AmyOCZdfLj = false;
      bool PVBKAKmmsk = false;
      bool bllfwThoSs = false;
      bool PCSxmcwhSA = false;
      bool zzBQVHecry = false;
      bool tgIzqOBSSi = false;
      bool qoMSTMDDXW = false;
      bool OFUrkbjVKn = false;
      bool oExUzhtSGY = false;
      bool bcOiIKmiwI = false;
      bool EaFUjnMPOP = false;
      bool deIdslrqTk = false;
      bool SjxztkpkjZ = false;
      bool oAYBjNzbkK = false;
      bool VeEptxTkpV = false;
      bool SuPKpTcGZy = false;
      string mPTKMmCdEo;
      string VDwFNzOjSH;
      string nRLwUlcpst;
      string HDJrDSrxyL;
      string xMVDZFqMSk;
      string oYQcPVEWpa;
      string KMjJhSsEBt;
      string HRkGJauHWO;
      string tPXHFhmfbh;
      string NUHXAkbMVH;
      string MmLXdXnJSn;
      string ZwAafuncDz;
      string eMbSflJNFY;
      string zilPCHEzdh;
      string tcNnIQUgrF;
      string wzIVbzKrqq;
      string NZDtnqKkXX;
      string pnzVFCznKO;
      string RcVAbAZPnC;
      string xMIlUTYSzR;
      if(mPTKMmCdEo == MmLXdXnJSn){GjITPIzmQY = true;}
      else if(MmLXdXnJSn == mPTKMmCdEo){qoMSTMDDXW = true;}
      if(VDwFNzOjSH == ZwAafuncDz){AWhbZjEDRn = true;}
      else if(ZwAafuncDz == VDwFNzOjSH){OFUrkbjVKn = true;}
      if(nRLwUlcpst == eMbSflJNFY){QuwUadfNAk = true;}
      else if(eMbSflJNFY == nRLwUlcpst){oExUzhtSGY = true;}
      if(HDJrDSrxyL == zilPCHEzdh){cfnrIgUBkT = true;}
      else if(zilPCHEzdh == HDJrDSrxyL){bcOiIKmiwI = true;}
      if(xMVDZFqMSk == tcNnIQUgrF){AmyOCZdfLj = true;}
      else if(tcNnIQUgrF == xMVDZFqMSk){EaFUjnMPOP = true;}
      if(oYQcPVEWpa == wzIVbzKrqq){PVBKAKmmsk = true;}
      else if(wzIVbzKrqq == oYQcPVEWpa){deIdslrqTk = true;}
      if(KMjJhSsEBt == NZDtnqKkXX){bllfwThoSs = true;}
      else if(NZDtnqKkXX == KMjJhSsEBt){SjxztkpkjZ = true;}
      if(HRkGJauHWO == pnzVFCznKO){PCSxmcwhSA = true;}
      if(tPXHFhmfbh == RcVAbAZPnC){zzBQVHecry = true;}
      if(NUHXAkbMVH == xMIlUTYSzR){tgIzqOBSSi = true;}
      while(pnzVFCznKO == HRkGJauHWO){oAYBjNzbkK = true;}
      while(RcVAbAZPnC == RcVAbAZPnC){VeEptxTkpV = true;}
      while(xMIlUTYSzR == xMIlUTYSzR){SuPKpTcGZy = true;}
      if(GjITPIzmQY == true){GjITPIzmQY = false;}
      if(AWhbZjEDRn == true){AWhbZjEDRn = false;}
      if(QuwUadfNAk == true){QuwUadfNAk = false;}
      if(cfnrIgUBkT == true){cfnrIgUBkT = false;}
      if(AmyOCZdfLj == true){AmyOCZdfLj = false;}
      if(PVBKAKmmsk == true){PVBKAKmmsk = false;}
      if(bllfwThoSs == true){bllfwThoSs = false;}
      if(PCSxmcwhSA == true){PCSxmcwhSA = false;}
      if(zzBQVHecry == true){zzBQVHecry = false;}
      if(tgIzqOBSSi == true){tgIzqOBSSi = false;}
      if(qoMSTMDDXW == true){qoMSTMDDXW = false;}
      if(OFUrkbjVKn == true){OFUrkbjVKn = false;}
      if(oExUzhtSGY == true){oExUzhtSGY = false;}
      if(bcOiIKmiwI == true){bcOiIKmiwI = false;}
      if(EaFUjnMPOP == true){EaFUjnMPOP = false;}
      if(deIdslrqTk == true){deIdslrqTk = false;}
      if(SjxztkpkjZ == true){SjxztkpkjZ = false;}
      if(oAYBjNzbkK == true){oAYBjNzbkK = false;}
      if(VeEptxTkpV == true){VeEptxTkpV = false;}
      if(SuPKpTcGZy == true){SuPKpTcGZy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class AVAMTWEXXW
{ 
  void OjxFJkuWzL()
  { 
      bool uglZGdYzHw = false;
      bool ErcIcIKzqZ = false;
      bool asoMapksyS = false;
      bool EpSXxCGcfK = false;
      bool ikojkhXpoS = false;
      bool DyNkwBqZyP = false;
      bool reAnrOYELa = false;
      bool ChSgqoDFbr = false;
      bool SuEfPGbhbu = false;
      bool KgdQREjArW = false;
      bool ISeNQVeftG = false;
      bool xlxGwxZUYk = false;
      bool lqbPQUDBPl = false;
      bool FDNEiAPpPs = false;
      bool aunVxYiXUU = false;
      bool dPaGjRpPxu = false;
      bool RcQHBtXsWw = false;
      bool gjFfKqCGRr = false;
      bool lfgtIobHTR = false;
      bool JArTbxdfUD = false;
      string xhISSAXxko;
      string SHLudOMOCR;
      string amQqEdJHXR;
      string LaFTNhWwME;
      string PCydbhlibr;
      string BrUkzJRQOo;
      string pQYTdqBrgu;
      string rQByCXnmPV;
      string qaffPTGUUD;
      string QFhXNzddAj;
      string gtBfzkqWia;
      string VdBrxPjrEK;
      string kgXcemicjC;
      string sKmeowYzfd;
      string bikshlDoZx;
      string OkZoyzWXAM;
      string CNlfhLqWCA;
      string qjMZhBudur;
      string isCmnHjxgo;
      string ZAySbOxFKW;
      if(xhISSAXxko == gtBfzkqWia){uglZGdYzHw = true;}
      else if(gtBfzkqWia == xhISSAXxko){ISeNQVeftG = true;}
      if(SHLudOMOCR == VdBrxPjrEK){ErcIcIKzqZ = true;}
      else if(VdBrxPjrEK == SHLudOMOCR){xlxGwxZUYk = true;}
      if(amQqEdJHXR == kgXcemicjC){asoMapksyS = true;}
      else if(kgXcemicjC == amQqEdJHXR){lqbPQUDBPl = true;}
      if(LaFTNhWwME == sKmeowYzfd){EpSXxCGcfK = true;}
      else if(sKmeowYzfd == LaFTNhWwME){FDNEiAPpPs = true;}
      if(PCydbhlibr == bikshlDoZx){ikojkhXpoS = true;}
      else if(bikshlDoZx == PCydbhlibr){aunVxYiXUU = true;}
      if(BrUkzJRQOo == OkZoyzWXAM){DyNkwBqZyP = true;}
      else if(OkZoyzWXAM == BrUkzJRQOo){dPaGjRpPxu = true;}
      if(pQYTdqBrgu == CNlfhLqWCA){reAnrOYELa = true;}
      else if(CNlfhLqWCA == pQYTdqBrgu){RcQHBtXsWw = true;}
      if(rQByCXnmPV == qjMZhBudur){ChSgqoDFbr = true;}
      if(qaffPTGUUD == isCmnHjxgo){SuEfPGbhbu = true;}
      if(QFhXNzddAj == ZAySbOxFKW){KgdQREjArW = true;}
      while(qjMZhBudur == rQByCXnmPV){gjFfKqCGRr = true;}
      while(isCmnHjxgo == isCmnHjxgo){lfgtIobHTR = true;}
      while(ZAySbOxFKW == ZAySbOxFKW){JArTbxdfUD = true;}
      if(uglZGdYzHw == true){uglZGdYzHw = false;}
      if(ErcIcIKzqZ == true){ErcIcIKzqZ = false;}
      if(asoMapksyS == true){asoMapksyS = false;}
      if(EpSXxCGcfK == true){EpSXxCGcfK = false;}
      if(ikojkhXpoS == true){ikojkhXpoS = false;}
      if(DyNkwBqZyP == true){DyNkwBqZyP = false;}
      if(reAnrOYELa == true){reAnrOYELa = false;}
      if(ChSgqoDFbr == true){ChSgqoDFbr = false;}
      if(SuEfPGbhbu == true){SuEfPGbhbu = false;}
      if(KgdQREjArW == true){KgdQREjArW = false;}
      if(ISeNQVeftG == true){ISeNQVeftG = false;}
      if(xlxGwxZUYk == true){xlxGwxZUYk = false;}
      if(lqbPQUDBPl == true){lqbPQUDBPl = false;}
      if(FDNEiAPpPs == true){FDNEiAPpPs = false;}
      if(aunVxYiXUU == true){aunVxYiXUU = false;}
      if(dPaGjRpPxu == true){dPaGjRpPxu = false;}
      if(RcQHBtXsWw == true){RcQHBtXsWw = false;}
      if(gjFfKqCGRr == true){gjFfKqCGRr = false;}
      if(lfgtIobHTR == true){lfgtIobHTR = false;}
      if(JArTbxdfUD == true){JArTbxdfUD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WKWZPUAXKP
{ 
  void HALgwsJFHy()
  { 
      bool ARgxJaCZew = false;
      bool pIUsLUdqso = false;
      bool ULNxkapwYk = false;
      bool lgTBMPSkAs = false;
      bool nKAoZNZQRI = false;
      bool sJplhoEiMi = false;
      bool jFAtKJnSgu = false;
      bool BspABJKXXX = false;
      bool BZeQrbpgSX = false;
      bool LjXRIhJGdQ = false;
      bool YGjVGATpkY = false;
      bool dfKogJajBi = false;
      bool SGAIjRUXqS = false;
      bool zEsGyupIKr = false;
      bool HzwtseYiky = false;
      bool HZEuxRFalo = false;
      bool ewySYYlDpl = false;
      bool TRZJaSKFPE = false;
      bool rklrKZAxJf = false;
      bool oPWUMLBdVT = false;
      string oOSPTguuVh;
      string kXBWViobec;
      string RhFEjCiznG;
      string HADqhMzMCS;
      string odrFKpSbVa;
      string rYExlHJOfQ;
      string xuHPakUKnI;
      string ibYLnnozts;
      string iHdMOiuGFa;
      string CkglVsVceY;
      string VImMqPzrIx;
      string RKTlgpRtAq;
      string oOENFyumpu;
      string RMTUXaTQVy;
      string iCxHIrQuEf;
      string aabmiHEZHs;
      string UMBNfFqzOG;
      string VAYCImQwEN;
      string gzhKTpjflQ;
      string VtJccWAjbD;
      if(oOSPTguuVh == VImMqPzrIx){ARgxJaCZew = true;}
      else if(VImMqPzrIx == oOSPTguuVh){YGjVGATpkY = true;}
      if(kXBWViobec == RKTlgpRtAq){pIUsLUdqso = true;}
      else if(RKTlgpRtAq == kXBWViobec){dfKogJajBi = true;}
      if(RhFEjCiznG == oOENFyumpu){ULNxkapwYk = true;}
      else if(oOENFyumpu == RhFEjCiznG){SGAIjRUXqS = true;}
      if(HADqhMzMCS == RMTUXaTQVy){lgTBMPSkAs = true;}
      else if(RMTUXaTQVy == HADqhMzMCS){zEsGyupIKr = true;}
      if(odrFKpSbVa == iCxHIrQuEf){nKAoZNZQRI = true;}
      else if(iCxHIrQuEf == odrFKpSbVa){HzwtseYiky = true;}
      if(rYExlHJOfQ == aabmiHEZHs){sJplhoEiMi = true;}
      else if(aabmiHEZHs == rYExlHJOfQ){HZEuxRFalo = true;}
      if(xuHPakUKnI == UMBNfFqzOG){jFAtKJnSgu = true;}
      else if(UMBNfFqzOG == xuHPakUKnI){ewySYYlDpl = true;}
      if(ibYLnnozts == VAYCImQwEN){BspABJKXXX = true;}
      if(iHdMOiuGFa == gzhKTpjflQ){BZeQrbpgSX = true;}
      if(CkglVsVceY == VtJccWAjbD){LjXRIhJGdQ = true;}
      while(VAYCImQwEN == ibYLnnozts){TRZJaSKFPE = true;}
      while(gzhKTpjflQ == gzhKTpjflQ){rklrKZAxJf = true;}
      while(VtJccWAjbD == VtJccWAjbD){oPWUMLBdVT = true;}
      if(ARgxJaCZew == true){ARgxJaCZew = false;}
      if(pIUsLUdqso == true){pIUsLUdqso = false;}
      if(ULNxkapwYk == true){ULNxkapwYk = false;}
      if(lgTBMPSkAs == true){lgTBMPSkAs = false;}
      if(nKAoZNZQRI == true){nKAoZNZQRI = false;}
      if(sJplhoEiMi == true){sJplhoEiMi = false;}
      if(jFAtKJnSgu == true){jFAtKJnSgu = false;}
      if(BspABJKXXX == true){BspABJKXXX = false;}
      if(BZeQrbpgSX == true){BZeQrbpgSX = false;}
      if(LjXRIhJGdQ == true){LjXRIhJGdQ = false;}
      if(YGjVGATpkY == true){YGjVGATpkY = false;}
      if(dfKogJajBi == true){dfKogJajBi = false;}
      if(SGAIjRUXqS == true){SGAIjRUXqS = false;}
      if(zEsGyupIKr == true){zEsGyupIKr = false;}
      if(HzwtseYiky == true){HzwtseYiky = false;}
      if(HZEuxRFalo == true){HZEuxRFalo = false;}
      if(ewySYYlDpl == true){ewySYYlDpl = false;}
      if(TRZJaSKFPE == true){TRZJaSKFPE = false;}
      if(rklrKZAxJf == true){rklrKZAxJf = false;}
      if(oPWUMLBdVT == true){oPWUMLBdVT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class AHVAPDBHCW
{ 
  void rpGYaabSOS()
  { 
      bool YfPjjXcbzL = false;
      bool bzfPUFuMCZ = false;
      bool FMWJsXnbnn = false;
      bool oUhPImEQWY = false;
      bool VVsMBMRdge = false;
      bool yaGRCLIFqM = false;
      bool jAnKVHRPHg = false;
      bool AdpcDiSKHf = false;
      bool FKuIpONPNx = false;
      bool dyZJoVhiqt = false;
      bool tBWzdTkdNS = false;
      bool ryyKGYEeUH = false;
      bool sCCJCBEfmh = false;
      bool BjTMlDmueV = false;
      bool cjBtEeAPDP = false;
      bool QCNiBrzMxb = false;
      bool PEpfRIekYC = false;
      bool lJAzeIAIFX = false;
      bool BncXRyWjLs = false;
      bool EayCFXtVbu = false;
      string umCkgkTmCe;
      string lGfsrakpbz;
      string xLdyeqYnsI;
      string hJDzxTqiUm;
      string MScAFxsjjT;
      string zHUpHjIDGo;
      string LessxbCfwX;
      string jBVnlehcrg;
      string yoEZJaoNrM;
      string KmcfbhZjDc;
      string uMmLjykfEc;
      string BfKYpWaSiW;
      string mnniNwXpME;
      string EyZOkkBroK;
      string oIJOLebDoF;
      string GGAgpHYDyn;
      string ZuTAXDFwnF;
      string UhdYgdeYXh;
      string TUAGxyxsYd;
      string MxtBjsnJqH;
      if(umCkgkTmCe == uMmLjykfEc){YfPjjXcbzL = true;}
      else if(uMmLjykfEc == umCkgkTmCe){tBWzdTkdNS = true;}
      if(lGfsrakpbz == BfKYpWaSiW){bzfPUFuMCZ = true;}
      else if(BfKYpWaSiW == lGfsrakpbz){ryyKGYEeUH = true;}
      if(xLdyeqYnsI == mnniNwXpME){FMWJsXnbnn = true;}
      else if(mnniNwXpME == xLdyeqYnsI){sCCJCBEfmh = true;}
      if(hJDzxTqiUm == EyZOkkBroK){oUhPImEQWY = true;}
      else if(EyZOkkBroK == hJDzxTqiUm){BjTMlDmueV = true;}
      if(MScAFxsjjT == oIJOLebDoF){VVsMBMRdge = true;}
      else if(oIJOLebDoF == MScAFxsjjT){cjBtEeAPDP = true;}
      if(zHUpHjIDGo == GGAgpHYDyn){yaGRCLIFqM = true;}
      else if(GGAgpHYDyn == zHUpHjIDGo){QCNiBrzMxb = true;}
      if(LessxbCfwX == ZuTAXDFwnF){jAnKVHRPHg = true;}
      else if(ZuTAXDFwnF == LessxbCfwX){PEpfRIekYC = true;}
      if(jBVnlehcrg == UhdYgdeYXh){AdpcDiSKHf = true;}
      if(yoEZJaoNrM == TUAGxyxsYd){FKuIpONPNx = true;}
      if(KmcfbhZjDc == MxtBjsnJqH){dyZJoVhiqt = true;}
      while(UhdYgdeYXh == jBVnlehcrg){lJAzeIAIFX = true;}
      while(TUAGxyxsYd == TUAGxyxsYd){BncXRyWjLs = true;}
      while(MxtBjsnJqH == MxtBjsnJqH){EayCFXtVbu = true;}
      if(YfPjjXcbzL == true){YfPjjXcbzL = false;}
      if(bzfPUFuMCZ == true){bzfPUFuMCZ = false;}
      if(FMWJsXnbnn == true){FMWJsXnbnn = false;}
      if(oUhPImEQWY == true){oUhPImEQWY = false;}
      if(VVsMBMRdge == true){VVsMBMRdge = false;}
      if(yaGRCLIFqM == true){yaGRCLIFqM = false;}
      if(jAnKVHRPHg == true){jAnKVHRPHg = false;}
      if(AdpcDiSKHf == true){AdpcDiSKHf = false;}
      if(FKuIpONPNx == true){FKuIpONPNx = false;}
      if(dyZJoVhiqt == true){dyZJoVhiqt = false;}
      if(tBWzdTkdNS == true){tBWzdTkdNS = false;}
      if(ryyKGYEeUH == true){ryyKGYEeUH = false;}
      if(sCCJCBEfmh == true){sCCJCBEfmh = false;}
      if(BjTMlDmueV == true){BjTMlDmueV = false;}
      if(cjBtEeAPDP == true){cjBtEeAPDP = false;}
      if(QCNiBrzMxb == true){QCNiBrzMxb = false;}
      if(PEpfRIekYC == true){PEpfRIekYC = false;}
      if(lJAzeIAIFX == true){lJAzeIAIFX = false;}
      if(BncXRyWjLs == true){BncXRyWjLs = false;}
      if(EayCFXtVbu == true){EayCFXtVbu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GQIAZDBXHV
{ 
  void pMAxUsLwSE()
  { 
      bool HCAezKoYwu = false;
      bool GBwgHhNfTN = false;
      bool RpaCdzBzaR = false;
      bool igwbAsxkSZ = false;
      bool JUlKNBVIxj = false;
      bool wSwKfQbyBA = false;
      bool JpxdLYEUgN = false;
      bool jbNsDwEQoc = false;
      bool crBqVPjlui = false;
      bool RzjSQTXHyn = false;
      bool wBbjzTFDHj = false;
      bool MLJYWQoFHw = false;
      bool oRZywjrtci = false;
      bool YwRAfbQQqC = false;
      bool JgeKVGpKuz = false;
      bool eRzBzdacmO = false;
      bool aKZXKQsIst = false;
      bool zXOFmHmObp = false;
      bool thzdEGwQBB = false;
      bool AmdbayHGzE = false;
      string VgStYQcJnJ;
      string iyqpzjCJkS;
      string LaBLBVOuEt;
      string wXRuatFMSa;
      string GLEDTEWUjM;
      string szrEFteHsD;
      string BhEtNKfuPE;
      string dpqJMzWxqw;
      string ztKqBweNpg;
      string OyTOZYGKpI;
      string XaFeiCHhrF;
      string WgPwDHWNrQ;
      string eSFipeVbjm;
      string sLHiYXoeIy;
      string oxUjJgiXLR;
      string CsSYcEVVoD;
      string feyEMLZfWf;
      string SqEsbhxUZI;
      string wyQRkzsiFU;
      string CSpFRJzQXy;
      if(VgStYQcJnJ == XaFeiCHhrF){HCAezKoYwu = true;}
      else if(XaFeiCHhrF == VgStYQcJnJ){wBbjzTFDHj = true;}
      if(iyqpzjCJkS == WgPwDHWNrQ){GBwgHhNfTN = true;}
      else if(WgPwDHWNrQ == iyqpzjCJkS){MLJYWQoFHw = true;}
      if(LaBLBVOuEt == eSFipeVbjm){RpaCdzBzaR = true;}
      else if(eSFipeVbjm == LaBLBVOuEt){oRZywjrtci = true;}
      if(wXRuatFMSa == sLHiYXoeIy){igwbAsxkSZ = true;}
      else if(sLHiYXoeIy == wXRuatFMSa){YwRAfbQQqC = true;}
      if(GLEDTEWUjM == oxUjJgiXLR){JUlKNBVIxj = true;}
      else if(oxUjJgiXLR == GLEDTEWUjM){JgeKVGpKuz = true;}
      if(szrEFteHsD == CsSYcEVVoD){wSwKfQbyBA = true;}
      else if(CsSYcEVVoD == szrEFteHsD){eRzBzdacmO = true;}
      if(BhEtNKfuPE == feyEMLZfWf){JpxdLYEUgN = true;}
      else if(feyEMLZfWf == BhEtNKfuPE){aKZXKQsIst = true;}
      if(dpqJMzWxqw == SqEsbhxUZI){jbNsDwEQoc = true;}
      if(ztKqBweNpg == wyQRkzsiFU){crBqVPjlui = true;}
      if(OyTOZYGKpI == CSpFRJzQXy){RzjSQTXHyn = true;}
      while(SqEsbhxUZI == dpqJMzWxqw){zXOFmHmObp = true;}
      while(wyQRkzsiFU == wyQRkzsiFU){thzdEGwQBB = true;}
      while(CSpFRJzQXy == CSpFRJzQXy){AmdbayHGzE = true;}
      if(HCAezKoYwu == true){HCAezKoYwu = false;}
      if(GBwgHhNfTN == true){GBwgHhNfTN = false;}
      if(RpaCdzBzaR == true){RpaCdzBzaR = false;}
      if(igwbAsxkSZ == true){igwbAsxkSZ = false;}
      if(JUlKNBVIxj == true){JUlKNBVIxj = false;}
      if(wSwKfQbyBA == true){wSwKfQbyBA = false;}
      if(JpxdLYEUgN == true){JpxdLYEUgN = false;}
      if(jbNsDwEQoc == true){jbNsDwEQoc = false;}
      if(crBqVPjlui == true){crBqVPjlui = false;}
      if(RzjSQTXHyn == true){RzjSQTXHyn = false;}
      if(wBbjzTFDHj == true){wBbjzTFDHj = false;}
      if(MLJYWQoFHw == true){MLJYWQoFHw = false;}
      if(oRZywjrtci == true){oRZywjrtci = false;}
      if(YwRAfbQQqC == true){YwRAfbQQqC = false;}
      if(JgeKVGpKuz == true){JgeKVGpKuz = false;}
      if(eRzBzdacmO == true){eRzBzdacmO = false;}
      if(aKZXKQsIst == true){aKZXKQsIst = false;}
      if(zXOFmHmObp == true){zXOFmHmObp = false;}
      if(thzdEGwQBB == true){thzdEGwQBB = false;}
      if(AmdbayHGzE == true){AmdbayHGzE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RWBDWVGJNT
{ 
  void RLDWfujNcV()
  { 
      bool urKOHZNFQT = false;
      bool oZWlhwwVUk = false;
      bool takUsyAbCW = false;
      bool FKmlnyoefW = false;
      bool hGqqruhbMM = false;
      bool shLUqUGoVI = false;
      bool YkDtNemjyi = false;
      bool GRpxDnjtcy = false;
      bool QYPodXzRuT = false;
      bool qAIWywDwcM = false;
      bool IxdcFbqNRC = false;
      bool kMLXqAVNPp = false;
      bool fUodrGITzH = false;
      bool kmQQoTtljC = false;
      bool fkAridnkey = false;
      bool ZoflgmIzky = false;
      bool SXOJRxwuhX = false;
      bool FjdNKtUwuN = false;
      bool SWnBguGdiC = false;
      bool LuBkiHWpak = false;
      string KsVAVWAyty;
      string lfIIsNWekw;
      string yTPqzbKaiP;
      string MpmYRdGeZY;
      string RpKHyOlBCL;
      string BSTjVqceBs;
      string LZOelusjpP;
      string GqELeIIHsj;
      string pJXXOtWDCa;
      string AadgNqbfVD;
      string VBKlEJNIFN;
      string hUlQDCBpPr;
      string YohPPfNXym;
      string gbloNnylTE;
      string fNynreoNcK;
      string okQddgTtWY;
      string wpJRgGiwCO;
      string DPWKMKqsXj;
      string pztmxuLILg;
      string UEOtIoEfGl;
      if(KsVAVWAyty == VBKlEJNIFN){urKOHZNFQT = true;}
      else if(VBKlEJNIFN == KsVAVWAyty){IxdcFbqNRC = true;}
      if(lfIIsNWekw == hUlQDCBpPr){oZWlhwwVUk = true;}
      else if(hUlQDCBpPr == lfIIsNWekw){kMLXqAVNPp = true;}
      if(yTPqzbKaiP == YohPPfNXym){takUsyAbCW = true;}
      else if(YohPPfNXym == yTPqzbKaiP){fUodrGITzH = true;}
      if(MpmYRdGeZY == gbloNnylTE){FKmlnyoefW = true;}
      else if(gbloNnylTE == MpmYRdGeZY){kmQQoTtljC = true;}
      if(RpKHyOlBCL == fNynreoNcK){hGqqruhbMM = true;}
      else if(fNynreoNcK == RpKHyOlBCL){fkAridnkey = true;}
      if(BSTjVqceBs == okQddgTtWY){shLUqUGoVI = true;}
      else if(okQddgTtWY == BSTjVqceBs){ZoflgmIzky = true;}
      if(LZOelusjpP == wpJRgGiwCO){YkDtNemjyi = true;}
      else if(wpJRgGiwCO == LZOelusjpP){SXOJRxwuhX = true;}
      if(GqELeIIHsj == DPWKMKqsXj){GRpxDnjtcy = true;}
      if(pJXXOtWDCa == pztmxuLILg){QYPodXzRuT = true;}
      if(AadgNqbfVD == UEOtIoEfGl){qAIWywDwcM = true;}
      while(DPWKMKqsXj == GqELeIIHsj){FjdNKtUwuN = true;}
      while(pztmxuLILg == pztmxuLILg){SWnBguGdiC = true;}
      while(UEOtIoEfGl == UEOtIoEfGl){LuBkiHWpak = true;}
      if(urKOHZNFQT == true){urKOHZNFQT = false;}
      if(oZWlhwwVUk == true){oZWlhwwVUk = false;}
      if(takUsyAbCW == true){takUsyAbCW = false;}
      if(FKmlnyoefW == true){FKmlnyoefW = false;}
      if(hGqqruhbMM == true){hGqqruhbMM = false;}
      if(shLUqUGoVI == true){shLUqUGoVI = false;}
      if(YkDtNemjyi == true){YkDtNemjyi = false;}
      if(GRpxDnjtcy == true){GRpxDnjtcy = false;}
      if(QYPodXzRuT == true){QYPodXzRuT = false;}
      if(qAIWywDwcM == true){qAIWywDwcM = false;}
      if(IxdcFbqNRC == true){IxdcFbqNRC = false;}
      if(kMLXqAVNPp == true){kMLXqAVNPp = false;}
      if(fUodrGITzH == true){fUodrGITzH = false;}
      if(kmQQoTtljC == true){kmQQoTtljC = false;}
      if(fkAridnkey == true){fkAridnkey = false;}
      if(ZoflgmIzky == true){ZoflgmIzky = false;}
      if(SXOJRxwuhX == true){SXOJRxwuhX = false;}
      if(FjdNKtUwuN == true){FjdNKtUwuN = false;}
      if(SWnBguGdiC == true){SWnBguGdiC = false;}
      if(LuBkiHWpak == true){LuBkiHWpak = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OYJSTULOVI
{ 
  void DwmRdDlykq()
  { 
      bool xSZheuTlcn = false;
      bool AoOqJLMNhG = false;
      bool cDKNrGzeKf = false;
      bool PwFPoOxIjV = false;
      bool KNlQCIylxO = false;
      bool QItPEsILpY = false;
      bool wnbwXDjyCt = false;
      bool OEdMuORsLj = false;
      bool HNhwOHCoLu = false;
      bool GkSKYVlosy = false;
      bool XziCzMdgTo = false;
      bool bOAFQhGfgP = false;
      bool BtmHnckSmY = false;
      bool bbANMIdewC = false;
      bool dUVWSqkMtq = false;
      bool PGHcwcMbOW = false;
      bool bLttMAOmsI = false;
      bool EbielfYQnx = false;
      bool rXYwJVTWQi = false;
      bool wCQZaSaaJS = false;
      string yBMilMZqDJ;
      string KTlRRFanie;
      string JIDaoUaCCB;
      string PkEbAgNctc;
      string ffSXknIiPy;
      string uNwnzpyksQ;
      string kWoBsZPKQj;
      string rxsFzcoepA;
      string MrlEGCoPcK;
      string icicrIqRdK;
      string YySeGxTpoV;
      string AStsEsjktq;
      string kzgGBPNPNp;
      string kXBebOYHyJ;
      string hyaGJPhpYy;
      string YGWlZYQLdq;
      string iEczkcROQp;
      string TclkJlYawl;
      string gWqOQeAOEC;
      string xNcHZdVbVg;
      if(yBMilMZqDJ == YySeGxTpoV){xSZheuTlcn = true;}
      else if(YySeGxTpoV == yBMilMZqDJ){XziCzMdgTo = true;}
      if(KTlRRFanie == AStsEsjktq){AoOqJLMNhG = true;}
      else if(AStsEsjktq == KTlRRFanie){bOAFQhGfgP = true;}
      if(JIDaoUaCCB == kzgGBPNPNp){cDKNrGzeKf = true;}
      else if(kzgGBPNPNp == JIDaoUaCCB){BtmHnckSmY = true;}
      if(PkEbAgNctc == kXBebOYHyJ){PwFPoOxIjV = true;}
      else if(kXBebOYHyJ == PkEbAgNctc){bbANMIdewC = true;}
      if(ffSXknIiPy == hyaGJPhpYy){KNlQCIylxO = true;}
      else if(hyaGJPhpYy == ffSXknIiPy){dUVWSqkMtq = true;}
      if(uNwnzpyksQ == YGWlZYQLdq){QItPEsILpY = true;}
      else if(YGWlZYQLdq == uNwnzpyksQ){PGHcwcMbOW = true;}
      if(kWoBsZPKQj == iEczkcROQp){wnbwXDjyCt = true;}
      else if(iEczkcROQp == kWoBsZPKQj){bLttMAOmsI = true;}
      if(rxsFzcoepA == TclkJlYawl){OEdMuORsLj = true;}
      if(MrlEGCoPcK == gWqOQeAOEC){HNhwOHCoLu = true;}
      if(icicrIqRdK == xNcHZdVbVg){GkSKYVlosy = true;}
      while(TclkJlYawl == rxsFzcoepA){EbielfYQnx = true;}
      while(gWqOQeAOEC == gWqOQeAOEC){rXYwJVTWQi = true;}
      while(xNcHZdVbVg == xNcHZdVbVg){wCQZaSaaJS = true;}
      if(xSZheuTlcn == true){xSZheuTlcn = false;}
      if(AoOqJLMNhG == true){AoOqJLMNhG = false;}
      if(cDKNrGzeKf == true){cDKNrGzeKf = false;}
      if(PwFPoOxIjV == true){PwFPoOxIjV = false;}
      if(KNlQCIylxO == true){KNlQCIylxO = false;}
      if(QItPEsILpY == true){QItPEsILpY = false;}
      if(wnbwXDjyCt == true){wnbwXDjyCt = false;}
      if(OEdMuORsLj == true){OEdMuORsLj = false;}
      if(HNhwOHCoLu == true){HNhwOHCoLu = false;}
      if(GkSKYVlosy == true){GkSKYVlosy = false;}
      if(XziCzMdgTo == true){XziCzMdgTo = false;}
      if(bOAFQhGfgP == true){bOAFQhGfgP = false;}
      if(BtmHnckSmY == true){BtmHnckSmY = false;}
      if(bbANMIdewC == true){bbANMIdewC = false;}
      if(dUVWSqkMtq == true){dUVWSqkMtq = false;}
      if(PGHcwcMbOW == true){PGHcwcMbOW = false;}
      if(bLttMAOmsI == true){bLttMAOmsI = false;}
      if(EbielfYQnx == true){EbielfYQnx = false;}
      if(rXYwJVTWQi == true){rXYwJVTWQi = false;}
      if(wCQZaSaaJS == true){wCQZaSaaJS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MHXYZGJDMV
{ 
  void ZBYraGIXsr()
  { 
      bool aFRbgczEJL = false;
      bool YasRVdHtlK = false;
      bool udSBCFYqHg = false;
      bool GrymBVRCDN = false;
      bool iIozKEmJFh = false;
      bool MFHRqwlQYa = false;
      bool EIxcEfcGPz = false;
      bool MEpYaloTqx = false;
      bool SOkYTBkfuF = false;
      bool jBccJaLLte = false;
      bool bgtFUHPipF = false;
      bool GRYbIoWXID = false;
      bool uwKJPLaMPA = false;
      bool ZcYMUlUfDL = false;
      bool lCppzRbBTT = false;
      bool EnDqifVjpJ = false;
      bool fCTpecsMGW = false;
      bool TqTwRsGTKx = false;
      bool iZsuwlZhco = false;
      bool PkptfjhALV = false;
      string zASPzSDWgK;
      string FLxAbiKlsb;
      string AzkfQBpouH;
      string VHheoFfkXi;
      string LlsIAzSAuM;
      string cosNKAPmzy;
      string cNaZoxdSKV;
      string DZOKQNFceR;
      string hptidgNTTo;
      string prlZGxLTSC;
      string VAoYJCZiww;
      string tlHgPmTruH;
      string jJyVfLnnET;
      string ClJcuhAUYa;
      string GdWXuDjEJS;
      string cSYKRLBTsx;
      string OlectIcelh;
      string lzSTBFmaZb;
      string bjdkqiNyLj;
      string fgabMUztcm;
      if(zASPzSDWgK == VAoYJCZiww){aFRbgczEJL = true;}
      else if(VAoYJCZiww == zASPzSDWgK){bgtFUHPipF = true;}
      if(FLxAbiKlsb == tlHgPmTruH){YasRVdHtlK = true;}
      else if(tlHgPmTruH == FLxAbiKlsb){GRYbIoWXID = true;}
      if(AzkfQBpouH == jJyVfLnnET){udSBCFYqHg = true;}
      else if(jJyVfLnnET == AzkfQBpouH){uwKJPLaMPA = true;}
      if(VHheoFfkXi == ClJcuhAUYa){GrymBVRCDN = true;}
      else if(ClJcuhAUYa == VHheoFfkXi){ZcYMUlUfDL = true;}
      if(LlsIAzSAuM == GdWXuDjEJS){iIozKEmJFh = true;}
      else if(GdWXuDjEJS == LlsIAzSAuM){lCppzRbBTT = true;}
      if(cosNKAPmzy == cSYKRLBTsx){MFHRqwlQYa = true;}
      else if(cSYKRLBTsx == cosNKAPmzy){EnDqifVjpJ = true;}
      if(cNaZoxdSKV == OlectIcelh){EIxcEfcGPz = true;}
      else if(OlectIcelh == cNaZoxdSKV){fCTpecsMGW = true;}
      if(DZOKQNFceR == lzSTBFmaZb){MEpYaloTqx = true;}
      if(hptidgNTTo == bjdkqiNyLj){SOkYTBkfuF = true;}
      if(prlZGxLTSC == fgabMUztcm){jBccJaLLte = true;}
      while(lzSTBFmaZb == DZOKQNFceR){TqTwRsGTKx = true;}
      while(bjdkqiNyLj == bjdkqiNyLj){iZsuwlZhco = true;}
      while(fgabMUztcm == fgabMUztcm){PkptfjhALV = true;}
      if(aFRbgczEJL == true){aFRbgczEJL = false;}
      if(YasRVdHtlK == true){YasRVdHtlK = false;}
      if(udSBCFYqHg == true){udSBCFYqHg = false;}
      if(GrymBVRCDN == true){GrymBVRCDN = false;}
      if(iIozKEmJFh == true){iIozKEmJFh = false;}
      if(MFHRqwlQYa == true){MFHRqwlQYa = false;}
      if(EIxcEfcGPz == true){EIxcEfcGPz = false;}
      if(MEpYaloTqx == true){MEpYaloTqx = false;}
      if(SOkYTBkfuF == true){SOkYTBkfuF = false;}
      if(jBccJaLLte == true){jBccJaLLte = false;}
      if(bgtFUHPipF == true){bgtFUHPipF = false;}
      if(GRYbIoWXID == true){GRYbIoWXID = false;}
      if(uwKJPLaMPA == true){uwKJPLaMPA = false;}
      if(ZcYMUlUfDL == true){ZcYMUlUfDL = false;}
      if(lCppzRbBTT == true){lCppzRbBTT = false;}
      if(EnDqifVjpJ == true){EnDqifVjpJ = false;}
      if(fCTpecsMGW == true){fCTpecsMGW = false;}
      if(TqTwRsGTKx == true){TqTwRsGTKx = false;}
      if(iZsuwlZhco == true){iZsuwlZhco = false;}
      if(PkptfjhALV == true){PkptfjhALV = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KIUEVPXMTC
{ 
  void wJZQwwtIbH()
  { 
      bool pFstjjTJcZ = false;
      bool txeCMHHLxu = false;
      bool gbJDFDcdEC = false;
      bool JWGQEhWRMj = false;
      bool gAwqSDVbYJ = false;
      bool YuReaNwoof = false;
      bool GpFZPXpNxd = false;
      bool kpyNOffRkB = false;
      bool hBEtdNEbpp = false;
      bool lhbfcqtVat = false;
      bool NIdtYixEQt = false;
      bool hdDmyEkKwC = false;
      bool TNWGrDNRjW = false;
      bool kJqqnZWKVu = false;
      bool QzLwIHdnhY = false;
      bool FjZYZzkXBV = false;
      bool UeUsIXmMzY = false;
      bool haoAkUCayk = false;
      bool YYjLRBjSQn = false;
      bool nzIAZleRmj = false;
      string pNBeDFAYic;
      string eyLDLLajbD;
      string PYtoXEOHOi;
      string MoVEywadkp;
      string SuqMLcphxF;
      string uefMBQTMGz;
      string qrKzQobHFO;
      string jxykkUhgdu;
      string NgJIngJJDg;
      string cOWlzZkqwV;
      string AibiVyftbf;
      string hlhLkJiBLi;
      string fSEXkUaMlK;
      string jpkHteEalo;
      string AOgWzqgKAo;
      string qifMpgdDxA;
      string XTIQwqAaRG;
      string EHOViQlgYt;
      string nPLxTAKYQN;
      string AAdkRfGPZH;
      if(pNBeDFAYic == AibiVyftbf){pFstjjTJcZ = true;}
      else if(AibiVyftbf == pNBeDFAYic){NIdtYixEQt = true;}
      if(eyLDLLajbD == hlhLkJiBLi){txeCMHHLxu = true;}
      else if(hlhLkJiBLi == eyLDLLajbD){hdDmyEkKwC = true;}
      if(PYtoXEOHOi == fSEXkUaMlK){gbJDFDcdEC = true;}
      else if(fSEXkUaMlK == PYtoXEOHOi){TNWGrDNRjW = true;}
      if(MoVEywadkp == jpkHteEalo){JWGQEhWRMj = true;}
      else if(jpkHteEalo == MoVEywadkp){kJqqnZWKVu = true;}
      if(SuqMLcphxF == AOgWzqgKAo){gAwqSDVbYJ = true;}
      else if(AOgWzqgKAo == SuqMLcphxF){QzLwIHdnhY = true;}
      if(uefMBQTMGz == qifMpgdDxA){YuReaNwoof = true;}
      else if(qifMpgdDxA == uefMBQTMGz){FjZYZzkXBV = true;}
      if(qrKzQobHFO == XTIQwqAaRG){GpFZPXpNxd = true;}
      else if(XTIQwqAaRG == qrKzQobHFO){UeUsIXmMzY = true;}
      if(jxykkUhgdu == EHOViQlgYt){kpyNOffRkB = true;}
      if(NgJIngJJDg == nPLxTAKYQN){hBEtdNEbpp = true;}
      if(cOWlzZkqwV == AAdkRfGPZH){lhbfcqtVat = true;}
      while(EHOViQlgYt == jxykkUhgdu){haoAkUCayk = true;}
      while(nPLxTAKYQN == nPLxTAKYQN){YYjLRBjSQn = true;}
      while(AAdkRfGPZH == AAdkRfGPZH){nzIAZleRmj = true;}
      if(pFstjjTJcZ == true){pFstjjTJcZ = false;}
      if(txeCMHHLxu == true){txeCMHHLxu = false;}
      if(gbJDFDcdEC == true){gbJDFDcdEC = false;}
      if(JWGQEhWRMj == true){JWGQEhWRMj = false;}
      if(gAwqSDVbYJ == true){gAwqSDVbYJ = false;}
      if(YuReaNwoof == true){YuReaNwoof = false;}
      if(GpFZPXpNxd == true){GpFZPXpNxd = false;}
      if(kpyNOffRkB == true){kpyNOffRkB = false;}
      if(hBEtdNEbpp == true){hBEtdNEbpp = false;}
      if(lhbfcqtVat == true){lhbfcqtVat = false;}
      if(NIdtYixEQt == true){NIdtYixEQt = false;}
      if(hdDmyEkKwC == true){hdDmyEkKwC = false;}
      if(TNWGrDNRjW == true){TNWGrDNRjW = false;}
      if(kJqqnZWKVu == true){kJqqnZWKVu = false;}
      if(QzLwIHdnhY == true){QzLwIHdnhY = false;}
      if(FjZYZzkXBV == true){FjZYZzkXBV = false;}
      if(UeUsIXmMzY == true){UeUsIXmMzY = false;}
      if(haoAkUCayk == true){haoAkUCayk = false;}
      if(YYjLRBjSQn == true){YYjLRBjSQn = false;}
      if(nzIAZleRmj == true){nzIAZleRmj = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UBSBGXKLYI
{ 
  void GjqFXkCQrD()
  { 
      bool YrjbiDeWzx = false;
      bool qfKqrIemen = false;
      bool dHjyeMHgJw = false;
      bool WDUqqodFoK = false;
      bool EluoZtHUcw = false;
      bool tHsZIQIkwV = false;
      bool uyMDanKTzC = false;
      bool dYxUjrLuXj = false;
      bool FlDNCOJthZ = false;
      bool grJSOFYrOl = false;
      bool inVaRUJXoQ = false;
      bool FMNMQItzBL = false;
      bool dQkZUlPeGd = false;
      bool STgkARQcug = false;
      bool FlERUUewBF = false;
      bool LZqCISCwFf = false;
      bool EeRKxkfbLz = false;
      bool rRETFqLSEA = false;
      bool HScASscdch = false;
      bool OUWOcaTsUi = false;
      string FNEigtIxKI;
      string MDOayeHpWI;
      string zAxwoKHDdJ;
      string jlRPNLtsyk;
      string OAdCKwjGlI;
      string yUiXmFPIsV;
      string lETBXZlyMm;
      string jNrktyWuks;
      string aARULPDtbI;
      string ulAbzJgbdk;
      string JksyYjcxco;
      string hTWzukmypI;
      string WrVEqyNPlj;
      string RtQAjoGSwa;
      string lFQmZbPAgN;
      string WhlVNROoGa;
      string VNLrXmCWPF;
      string dTCOVQaidP;
      string hNhXNiIulR;
      string ZRLVEYtXOm;
      if(FNEigtIxKI == JksyYjcxco){YrjbiDeWzx = true;}
      else if(JksyYjcxco == FNEigtIxKI){inVaRUJXoQ = true;}
      if(MDOayeHpWI == hTWzukmypI){qfKqrIemen = true;}
      else if(hTWzukmypI == MDOayeHpWI){FMNMQItzBL = true;}
      if(zAxwoKHDdJ == WrVEqyNPlj){dHjyeMHgJw = true;}
      else if(WrVEqyNPlj == zAxwoKHDdJ){dQkZUlPeGd = true;}
      if(jlRPNLtsyk == RtQAjoGSwa){WDUqqodFoK = true;}
      else if(RtQAjoGSwa == jlRPNLtsyk){STgkARQcug = true;}
      if(OAdCKwjGlI == lFQmZbPAgN){EluoZtHUcw = true;}
      else if(lFQmZbPAgN == OAdCKwjGlI){FlERUUewBF = true;}
      if(yUiXmFPIsV == WhlVNROoGa){tHsZIQIkwV = true;}
      else if(WhlVNROoGa == yUiXmFPIsV){LZqCISCwFf = true;}
      if(lETBXZlyMm == VNLrXmCWPF){uyMDanKTzC = true;}
      else if(VNLrXmCWPF == lETBXZlyMm){EeRKxkfbLz = true;}
      if(jNrktyWuks == dTCOVQaidP){dYxUjrLuXj = true;}
      if(aARULPDtbI == hNhXNiIulR){FlDNCOJthZ = true;}
      if(ulAbzJgbdk == ZRLVEYtXOm){grJSOFYrOl = true;}
      while(dTCOVQaidP == jNrktyWuks){rRETFqLSEA = true;}
      while(hNhXNiIulR == hNhXNiIulR){HScASscdch = true;}
      while(ZRLVEYtXOm == ZRLVEYtXOm){OUWOcaTsUi = true;}
      if(YrjbiDeWzx == true){YrjbiDeWzx = false;}
      if(qfKqrIemen == true){qfKqrIemen = false;}
      if(dHjyeMHgJw == true){dHjyeMHgJw = false;}
      if(WDUqqodFoK == true){WDUqqodFoK = false;}
      if(EluoZtHUcw == true){EluoZtHUcw = false;}
      if(tHsZIQIkwV == true){tHsZIQIkwV = false;}
      if(uyMDanKTzC == true){uyMDanKTzC = false;}
      if(dYxUjrLuXj == true){dYxUjrLuXj = false;}
      if(FlDNCOJthZ == true){FlDNCOJthZ = false;}
      if(grJSOFYrOl == true){grJSOFYrOl = false;}
      if(inVaRUJXoQ == true){inVaRUJXoQ = false;}
      if(FMNMQItzBL == true){FMNMQItzBL = false;}
      if(dQkZUlPeGd == true){dQkZUlPeGd = false;}
      if(STgkARQcug == true){STgkARQcug = false;}
      if(FlERUUewBF == true){FlERUUewBF = false;}
      if(LZqCISCwFf == true){LZqCISCwFf = false;}
      if(EeRKxkfbLz == true){EeRKxkfbLz = false;}
      if(rRETFqLSEA == true){rRETFqLSEA = false;}
      if(HScASscdch == true){HScASscdch = false;}
      if(OUWOcaTsUi == true){OUWOcaTsUi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TYXQEZWQYB
{ 
  void tDHTlbmUMw()
  { 
      bool sEVTjGHGnj = false;
      bool lkTtdVlNVx = false;
      bool VGxZgAWcYQ = false;
      bool rPYMJXTVZC = false;
      bool ugVYhezxZd = false;
      bool KQgsAlHwmi = false;
      bool jdgWrVLGkG = false;
      bool CQnTBnjXrB = false;
      bool AYDtGaheMW = false;
      bool PXipSQzzfL = false;
      bool SrMTbsbNqM = false;
      bool BjDkhKalPQ = false;
      bool nftNpdgyAj = false;
      bool okqghHMiSM = false;
      bool zTnzVfTxHH = false;
      bool pNiOWlyzdx = false;
      bool MdMGXPsWDa = false;
      bool yqTCRTrtqJ = false;
      bool KjprtaBbJu = false;
      bool JMFVAVGgbF = false;
      string VYJbPMHWzh;
      string xyokWZTLnb;
      string HMrodYdGXj;
      string ijEzPVDfXd;
      string YgeoszynJR;
      string GYuMRJgTxp;
      string hAzOyMsoVA;
      string WnnJCZhDwe;
      string apxFdKnrhG;
      string nihgwYhPpf;
      string GISItRjlJq;
      string adXSPqDskn;
      string YBGOMnFHNd;
      string qRMQYRXnKL;
      string ZChpeKLPQE;
      string UoeCGEBDwG;
      string LcXezIRGsx;
      string TOhYNWSQiC;
      string cppJICmKJT;
      string QTrEzweDqW;
      if(VYJbPMHWzh == GISItRjlJq){sEVTjGHGnj = true;}
      else if(GISItRjlJq == VYJbPMHWzh){SrMTbsbNqM = true;}
      if(xyokWZTLnb == adXSPqDskn){lkTtdVlNVx = true;}
      else if(adXSPqDskn == xyokWZTLnb){BjDkhKalPQ = true;}
      if(HMrodYdGXj == YBGOMnFHNd){VGxZgAWcYQ = true;}
      else if(YBGOMnFHNd == HMrodYdGXj){nftNpdgyAj = true;}
      if(ijEzPVDfXd == qRMQYRXnKL){rPYMJXTVZC = true;}
      else if(qRMQYRXnKL == ijEzPVDfXd){okqghHMiSM = true;}
      if(YgeoszynJR == ZChpeKLPQE){ugVYhezxZd = true;}
      else if(ZChpeKLPQE == YgeoszynJR){zTnzVfTxHH = true;}
      if(GYuMRJgTxp == UoeCGEBDwG){KQgsAlHwmi = true;}
      else if(UoeCGEBDwG == GYuMRJgTxp){pNiOWlyzdx = true;}
      if(hAzOyMsoVA == LcXezIRGsx){jdgWrVLGkG = true;}
      else if(LcXezIRGsx == hAzOyMsoVA){MdMGXPsWDa = true;}
      if(WnnJCZhDwe == TOhYNWSQiC){CQnTBnjXrB = true;}
      if(apxFdKnrhG == cppJICmKJT){AYDtGaheMW = true;}
      if(nihgwYhPpf == QTrEzweDqW){PXipSQzzfL = true;}
      while(TOhYNWSQiC == WnnJCZhDwe){yqTCRTrtqJ = true;}
      while(cppJICmKJT == cppJICmKJT){KjprtaBbJu = true;}
      while(QTrEzweDqW == QTrEzweDqW){JMFVAVGgbF = true;}
      if(sEVTjGHGnj == true){sEVTjGHGnj = false;}
      if(lkTtdVlNVx == true){lkTtdVlNVx = false;}
      if(VGxZgAWcYQ == true){VGxZgAWcYQ = false;}
      if(rPYMJXTVZC == true){rPYMJXTVZC = false;}
      if(ugVYhezxZd == true){ugVYhezxZd = false;}
      if(KQgsAlHwmi == true){KQgsAlHwmi = false;}
      if(jdgWrVLGkG == true){jdgWrVLGkG = false;}
      if(CQnTBnjXrB == true){CQnTBnjXrB = false;}
      if(AYDtGaheMW == true){AYDtGaheMW = false;}
      if(PXipSQzzfL == true){PXipSQzzfL = false;}
      if(SrMTbsbNqM == true){SrMTbsbNqM = false;}
      if(BjDkhKalPQ == true){BjDkhKalPQ = false;}
      if(nftNpdgyAj == true){nftNpdgyAj = false;}
      if(okqghHMiSM == true){okqghHMiSM = false;}
      if(zTnzVfTxHH == true){zTnzVfTxHH = false;}
      if(pNiOWlyzdx == true){pNiOWlyzdx = false;}
      if(MdMGXPsWDa == true){MdMGXPsWDa = false;}
      if(yqTCRTrtqJ == true){yqTCRTrtqJ = false;}
      if(KjprtaBbJu == true){KjprtaBbJu = false;}
      if(JMFVAVGgbF == true){JMFVAVGgbF = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EFFJCKGSTN
{ 
  void lhXGpsWYlP()
  { 
      bool LeVJmxexri = false;
      bool ySIKtnCNUu = false;
      bool QcZLXyPdyF = false;
      bool FddGHiYCPy = false;
      bool SGJQwjSYyn = false;
      bool tBekPhEKlo = false;
      bool gAzmcUoBDL = false;
      bool PAyWbKTIOm = false;
      bool eqeJLDyrqY = false;
      bool dNQZQRHVmb = false;
      bool uQVlYsDIIE = false;
      bool OuuyKpUobZ = false;
      bool VwKdgBiXFJ = false;
      bool DxVmrNUrjR = false;
      bool ApilKhSRPa = false;
      bool xRtNGzGqFX = false;
      bool KeDzBQZHGw = false;
      bool VbFJZfMXTz = false;
      bool hRIeMlCIgs = false;
      bool GcmLHUljks = false;
      string igVhDhVBIK;
      string jKiSYTYdoT;
      string CAAhAEcLAF;
      string XTligyUqpO;
      string SbNuWzLVMY;
      string nAGKSigngy;
      string EKADlQYzla;
      string zgnZAZlFKA;
      string rHybRXNbQx;
      string RgJqKIaKIG;
      string aLjlZGLfJr;
      string wacfGxLYlN;
      string CxinIRXxPr;
      string zbwDZfXMBK;
      string LgRYBpDcjM;
      string wBXtEdcucI;
      string hOQQrUeYGa;
      string ZzWaCVeKmS;
      string NtMCwxDBzu;
      string CbiscwPAff;
      if(igVhDhVBIK == aLjlZGLfJr){LeVJmxexri = true;}
      else if(aLjlZGLfJr == igVhDhVBIK){uQVlYsDIIE = true;}
      if(jKiSYTYdoT == wacfGxLYlN){ySIKtnCNUu = true;}
      else if(wacfGxLYlN == jKiSYTYdoT){OuuyKpUobZ = true;}
      if(CAAhAEcLAF == CxinIRXxPr){QcZLXyPdyF = true;}
      else if(CxinIRXxPr == CAAhAEcLAF){VwKdgBiXFJ = true;}
      if(XTligyUqpO == zbwDZfXMBK){FddGHiYCPy = true;}
      else if(zbwDZfXMBK == XTligyUqpO){DxVmrNUrjR = true;}
      if(SbNuWzLVMY == LgRYBpDcjM){SGJQwjSYyn = true;}
      else if(LgRYBpDcjM == SbNuWzLVMY){ApilKhSRPa = true;}
      if(nAGKSigngy == wBXtEdcucI){tBekPhEKlo = true;}
      else if(wBXtEdcucI == nAGKSigngy){xRtNGzGqFX = true;}
      if(EKADlQYzla == hOQQrUeYGa){gAzmcUoBDL = true;}
      else if(hOQQrUeYGa == EKADlQYzla){KeDzBQZHGw = true;}
      if(zgnZAZlFKA == ZzWaCVeKmS){PAyWbKTIOm = true;}
      if(rHybRXNbQx == NtMCwxDBzu){eqeJLDyrqY = true;}
      if(RgJqKIaKIG == CbiscwPAff){dNQZQRHVmb = true;}
      while(ZzWaCVeKmS == zgnZAZlFKA){VbFJZfMXTz = true;}
      while(NtMCwxDBzu == NtMCwxDBzu){hRIeMlCIgs = true;}
      while(CbiscwPAff == CbiscwPAff){GcmLHUljks = true;}
      if(LeVJmxexri == true){LeVJmxexri = false;}
      if(ySIKtnCNUu == true){ySIKtnCNUu = false;}
      if(QcZLXyPdyF == true){QcZLXyPdyF = false;}
      if(FddGHiYCPy == true){FddGHiYCPy = false;}
      if(SGJQwjSYyn == true){SGJQwjSYyn = false;}
      if(tBekPhEKlo == true){tBekPhEKlo = false;}
      if(gAzmcUoBDL == true){gAzmcUoBDL = false;}
      if(PAyWbKTIOm == true){PAyWbKTIOm = false;}
      if(eqeJLDyrqY == true){eqeJLDyrqY = false;}
      if(dNQZQRHVmb == true){dNQZQRHVmb = false;}
      if(uQVlYsDIIE == true){uQVlYsDIIE = false;}
      if(OuuyKpUobZ == true){OuuyKpUobZ = false;}
      if(VwKdgBiXFJ == true){VwKdgBiXFJ = false;}
      if(DxVmrNUrjR == true){DxVmrNUrjR = false;}
      if(ApilKhSRPa == true){ApilKhSRPa = false;}
      if(xRtNGzGqFX == true){xRtNGzGqFX = false;}
      if(KeDzBQZHGw == true){KeDzBQZHGw = false;}
      if(VbFJZfMXTz == true){VbFJZfMXTz = false;}
      if(hRIeMlCIgs == true){hRIeMlCIgs = false;}
      if(GcmLHUljks == true){GcmLHUljks = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KNKCEXXHYL
{ 
  void YrLdQfgjaN()
  { 
      bool bejSClIuoJ = false;
      bool tHqGKYAJcq = false;
      bool WJVHLgqAIV = false;
      bool gMDlJuewhC = false;
      bool LGYKtLqHOD = false;
      bool eAwdpRJbHU = false;
      bool tImTcgEExP = false;
      bool sEaZeQkLNA = false;
      bool pikEpfPZwq = false;
      bool kCoNHDSFgM = false;
      bool GmKyKaZWyZ = false;
      bool fxOGeLXicw = false;
      bool ReIFdqONRb = false;
      bool xfdBsDYltO = false;
      bool GsaWLnnhhw = false;
      bool fibkubdFUT = false;
      bool LLEWbzSNMb = false;
      bool mloNoXDlsp = false;
      bool CfeUuNDrFQ = false;
      bool YSRZCtnEtL = false;
      string agRGkKUdeQ;
      string unlHRWqdEf;
      string YAIxIWSayc;
      string rCVjWCywox;
      string xAiNkcmOrF;
      string dSDgnZYUyN;
      string rteBXrRBjO;
      string YtoHiYQuLf;
      string jDirzZzoww;
      string ddybFCUhor;
      string KkecgidwxP;
      string IdIRdnLlBy;
      string scFmkuuNBf;
      string DiFSVyYFLQ;
      string wDqgmAUfyQ;
      string qigqzgfqqk;
      string nPtRLZXowe;
      string OFZkBplXAk;
      string ZOPkoUCNTn;
      string ShNZcfJYUN;
      if(agRGkKUdeQ == KkecgidwxP){bejSClIuoJ = true;}
      else if(KkecgidwxP == agRGkKUdeQ){GmKyKaZWyZ = true;}
      if(unlHRWqdEf == IdIRdnLlBy){tHqGKYAJcq = true;}
      else if(IdIRdnLlBy == unlHRWqdEf){fxOGeLXicw = true;}
      if(YAIxIWSayc == scFmkuuNBf){WJVHLgqAIV = true;}
      else if(scFmkuuNBf == YAIxIWSayc){ReIFdqONRb = true;}
      if(rCVjWCywox == DiFSVyYFLQ){gMDlJuewhC = true;}
      else if(DiFSVyYFLQ == rCVjWCywox){xfdBsDYltO = true;}
      if(xAiNkcmOrF == wDqgmAUfyQ){LGYKtLqHOD = true;}
      else if(wDqgmAUfyQ == xAiNkcmOrF){GsaWLnnhhw = true;}
      if(dSDgnZYUyN == qigqzgfqqk){eAwdpRJbHU = true;}
      else if(qigqzgfqqk == dSDgnZYUyN){fibkubdFUT = true;}
      if(rteBXrRBjO == nPtRLZXowe){tImTcgEExP = true;}
      else if(nPtRLZXowe == rteBXrRBjO){LLEWbzSNMb = true;}
      if(YtoHiYQuLf == OFZkBplXAk){sEaZeQkLNA = true;}
      if(jDirzZzoww == ZOPkoUCNTn){pikEpfPZwq = true;}
      if(ddybFCUhor == ShNZcfJYUN){kCoNHDSFgM = true;}
      while(OFZkBplXAk == YtoHiYQuLf){mloNoXDlsp = true;}
      while(ZOPkoUCNTn == ZOPkoUCNTn){CfeUuNDrFQ = true;}
      while(ShNZcfJYUN == ShNZcfJYUN){YSRZCtnEtL = true;}
      if(bejSClIuoJ == true){bejSClIuoJ = false;}
      if(tHqGKYAJcq == true){tHqGKYAJcq = false;}
      if(WJVHLgqAIV == true){WJVHLgqAIV = false;}
      if(gMDlJuewhC == true){gMDlJuewhC = false;}
      if(LGYKtLqHOD == true){LGYKtLqHOD = false;}
      if(eAwdpRJbHU == true){eAwdpRJbHU = false;}
      if(tImTcgEExP == true){tImTcgEExP = false;}
      if(sEaZeQkLNA == true){sEaZeQkLNA = false;}
      if(pikEpfPZwq == true){pikEpfPZwq = false;}
      if(kCoNHDSFgM == true){kCoNHDSFgM = false;}
      if(GmKyKaZWyZ == true){GmKyKaZWyZ = false;}
      if(fxOGeLXicw == true){fxOGeLXicw = false;}
      if(ReIFdqONRb == true){ReIFdqONRb = false;}
      if(xfdBsDYltO == true){xfdBsDYltO = false;}
      if(GsaWLnnhhw == true){GsaWLnnhhw = false;}
      if(fibkubdFUT == true){fibkubdFUT = false;}
      if(LLEWbzSNMb == true){LLEWbzSNMb = false;}
      if(mloNoXDlsp == true){mloNoXDlsp = false;}
      if(CfeUuNDrFQ == true){CfeUuNDrFQ = false;}
      if(YSRZCtnEtL == true){YSRZCtnEtL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class INIBLLWDPH
{ 
  void fppUfzcObZ()
  { 
      bool UcwAiGEkRu = false;
      bool OACSMOuQFk = false;
      bool zPALlytrkY = false;
      bool yDOUplhyKL = false;
      bool oeIVYiHQWg = false;
      bool xOgUmEckOj = false;
      bool MMzswLlgca = false;
      bool fwFsuUHemC = false;
      bool qcVEajVsgk = false;
      bool IwjZzcfPYM = false;
      bool xamwAAImYS = false;
      bool xyyMsipuRL = false;
      bool przrRiMftR = false;
      bool oqFZFQpZDa = false;
      bool bhUOmJyXfP = false;
      bool mkyNWEaudQ = false;
      bool nbsGdFsXGc = false;
      bool sQqqiHXjEI = false;
      bool knzdeDsqDA = false;
      bool otwmlAoNkh = false;
      string AAblJmHbUI;
      string AgoaxDyduP;
      string ujAksCUAsq;
      string wrsrQMrhHm;
      string iBPBBsWfkw;
      string joFHAQAprH;
      string RGhcMHmwKj;
      string LPWHfsTzjz;
      string eoOwHGmlqz;
      string MBozUcSlos;
      string GZUBkTSxaG;
      string xAGsbcrlUF;
      string TAFKWJaWlD;
      string agJpsFTNKn;
      string npKkCLtwXr;
      string jfRiYFUKdb;
      string suKXePqQzW;
      string IdbgRdpTHK;
      string PdztStPaQn;
      string jJjiuremFm;
      if(AAblJmHbUI == GZUBkTSxaG){UcwAiGEkRu = true;}
      else if(GZUBkTSxaG == AAblJmHbUI){xamwAAImYS = true;}
      if(AgoaxDyduP == xAGsbcrlUF){OACSMOuQFk = true;}
      else if(xAGsbcrlUF == AgoaxDyduP){xyyMsipuRL = true;}
      if(ujAksCUAsq == TAFKWJaWlD){zPALlytrkY = true;}
      else if(TAFKWJaWlD == ujAksCUAsq){przrRiMftR = true;}
      if(wrsrQMrhHm == agJpsFTNKn){yDOUplhyKL = true;}
      else if(agJpsFTNKn == wrsrQMrhHm){oqFZFQpZDa = true;}
      if(iBPBBsWfkw == npKkCLtwXr){oeIVYiHQWg = true;}
      else if(npKkCLtwXr == iBPBBsWfkw){bhUOmJyXfP = true;}
      if(joFHAQAprH == jfRiYFUKdb){xOgUmEckOj = true;}
      else if(jfRiYFUKdb == joFHAQAprH){mkyNWEaudQ = true;}
      if(RGhcMHmwKj == suKXePqQzW){MMzswLlgca = true;}
      else if(suKXePqQzW == RGhcMHmwKj){nbsGdFsXGc = true;}
      if(LPWHfsTzjz == IdbgRdpTHK){fwFsuUHemC = true;}
      if(eoOwHGmlqz == PdztStPaQn){qcVEajVsgk = true;}
      if(MBozUcSlos == jJjiuremFm){IwjZzcfPYM = true;}
      while(IdbgRdpTHK == LPWHfsTzjz){sQqqiHXjEI = true;}
      while(PdztStPaQn == PdztStPaQn){knzdeDsqDA = true;}
      while(jJjiuremFm == jJjiuremFm){otwmlAoNkh = true;}
      if(UcwAiGEkRu == true){UcwAiGEkRu = false;}
      if(OACSMOuQFk == true){OACSMOuQFk = false;}
      if(zPALlytrkY == true){zPALlytrkY = false;}
      if(yDOUplhyKL == true){yDOUplhyKL = false;}
      if(oeIVYiHQWg == true){oeIVYiHQWg = false;}
      if(xOgUmEckOj == true){xOgUmEckOj = false;}
      if(MMzswLlgca == true){MMzswLlgca = false;}
      if(fwFsuUHemC == true){fwFsuUHemC = false;}
      if(qcVEajVsgk == true){qcVEajVsgk = false;}
      if(IwjZzcfPYM == true){IwjZzcfPYM = false;}
      if(xamwAAImYS == true){xamwAAImYS = false;}
      if(xyyMsipuRL == true){xyyMsipuRL = false;}
      if(przrRiMftR == true){przrRiMftR = false;}
      if(oqFZFQpZDa == true){oqFZFQpZDa = false;}
      if(bhUOmJyXfP == true){bhUOmJyXfP = false;}
      if(mkyNWEaudQ == true){mkyNWEaudQ = false;}
      if(nbsGdFsXGc == true){nbsGdFsXGc = false;}
      if(sQqqiHXjEI == true){sQqqiHXjEI = false;}
      if(knzdeDsqDA == true){knzdeDsqDA = false;}
      if(otwmlAoNkh == true){otwmlAoNkh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KJHSAWJEAJ
{ 
  void OpOdbDZtaj()
  { 
      bool biKZQyMkaQ = false;
      bool sYgWQxfRId = false;
      bool mJqYTBYATV = false;
      bool VaKzAWXZVH = false;
      bool deSyIdfHXL = false;
      bool drDOaSrkGP = false;
      bool YMQEuSOgVt = false;
      bool YFwAAHprPp = false;
      bool URbIrdWwYh = false;
      bool elTFhQNODp = false;
      bool PzkdlXwDNS = false;
      bool bzVPuHjIMR = false;
      bool uRHnPmlpRi = false;
      bool saoYaVgyWT = false;
      bool gTiGFePDKa = false;
      bool fFCktxqxNy = false;
      bool lFkdYBJxoY = false;
      bool allrjbiBhj = false;
      bool WieqCnYDdc = false;
      bool djjXhbhEgE = false;
      string sqJXrqFoHn;
      string JRfdRuzbWq;
      string hyEhiOnmHC;
      string wbHlwXmjdD;
      string BklWaFnGBb;
      string OboKKumbnd;
      string CSgFrhKsgs;
      string HOxSowLgGd;
      string aglfnUlwbB;
      string kLuAAhDfes;
      string ttZHTzdWVN;
      string iFUKDVgBRX;
      string uNRfHwTxRH;
      string seCNifHdMn;
      string pOaNRepkxe;
      string BCOHxgctzW;
      string ffEOUDDHaW;
      string NDocgfmDNc;
      string FhAmFpQtPC;
      string DMdoIKKMrN;
      if(sqJXrqFoHn == ttZHTzdWVN){biKZQyMkaQ = true;}
      else if(ttZHTzdWVN == sqJXrqFoHn){PzkdlXwDNS = true;}
      if(JRfdRuzbWq == iFUKDVgBRX){sYgWQxfRId = true;}
      else if(iFUKDVgBRX == JRfdRuzbWq){bzVPuHjIMR = true;}
      if(hyEhiOnmHC == uNRfHwTxRH){mJqYTBYATV = true;}
      else if(uNRfHwTxRH == hyEhiOnmHC){uRHnPmlpRi = true;}
      if(wbHlwXmjdD == seCNifHdMn){VaKzAWXZVH = true;}
      else if(seCNifHdMn == wbHlwXmjdD){saoYaVgyWT = true;}
      if(BklWaFnGBb == pOaNRepkxe){deSyIdfHXL = true;}
      else if(pOaNRepkxe == BklWaFnGBb){gTiGFePDKa = true;}
      if(OboKKumbnd == BCOHxgctzW){drDOaSrkGP = true;}
      else if(BCOHxgctzW == OboKKumbnd){fFCktxqxNy = true;}
      if(CSgFrhKsgs == ffEOUDDHaW){YMQEuSOgVt = true;}
      else if(ffEOUDDHaW == CSgFrhKsgs){lFkdYBJxoY = true;}
      if(HOxSowLgGd == NDocgfmDNc){YFwAAHprPp = true;}
      if(aglfnUlwbB == FhAmFpQtPC){URbIrdWwYh = true;}
      if(kLuAAhDfes == DMdoIKKMrN){elTFhQNODp = true;}
      while(NDocgfmDNc == HOxSowLgGd){allrjbiBhj = true;}
      while(FhAmFpQtPC == FhAmFpQtPC){WieqCnYDdc = true;}
      while(DMdoIKKMrN == DMdoIKKMrN){djjXhbhEgE = true;}
      if(biKZQyMkaQ == true){biKZQyMkaQ = false;}
      if(sYgWQxfRId == true){sYgWQxfRId = false;}
      if(mJqYTBYATV == true){mJqYTBYATV = false;}
      if(VaKzAWXZVH == true){VaKzAWXZVH = false;}
      if(deSyIdfHXL == true){deSyIdfHXL = false;}
      if(drDOaSrkGP == true){drDOaSrkGP = false;}
      if(YMQEuSOgVt == true){YMQEuSOgVt = false;}
      if(YFwAAHprPp == true){YFwAAHprPp = false;}
      if(URbIrdWwYh == true){URbIrdWwYh = false;}
      if(elTFhQNODp == true){elTFhQNODp = false;}
      if(PzkdlXwDNS == true){PzkdlXwDNS = false;}
      if(bzVPuHjIMR == true){bzVPuHjIMR = false;}
      if(uRHnPmlpRi == true){uRHnPmlpRi = false;}
      if(saoYaVgyWT == true){saoYaVgyWT = false;}
      if(gTiGFePDKa == true){gTiGFePDKa = false;}
      if(fFCktxqxNy == true){fFCktxqxNy = false;}
      if(lFkdYBJxoY == true){lFkdYBJxoY = false;}
      if(allrjbiBhj == true){allrjbiBhj = false;}
      if(WieqCnYDdc == true){WieqCnYDdc = false;}
      if(djjXhbhEgE == true){djjXhbhEgE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LSJLWKODKP
{ 
  void FHVGHnjuXj()
  { 
      bool FJhnQsZFxE = false;
      bool XDKhGQookm = false;
      bool LMoTXJWoXd = false;
      bool GAbtdmqIdQ = false;
      bool KkaFlJuhra = false;
      bool HsBVmrUOQG = false;
      bool VkgTxXaQES = false;
      bool YfLUcalihN = false;
      bool OowwWNyGHk = false;
      bool ScrWtZDGIN = false;
      bool hecYCahgsL = false;
      bool GkfCrgnjxj = false;
      bool ZUISAunnZD = false;
      bool XeaTfjLXtu = false;
      bool fQNuegwoeN = false;
      bool kSJYnMVKYG = false;
      bool msHonbJMWH = false;
      bool UQpCDIHNnS = false;
      bool yyIMREVCfS = false;
      bool OBymIlJXGD = false;
      string JNLSirFtsU;
      string LdAThwFhRf;
      string DNDzSnMNsh;
      string YXqbLiaJDk;
      string pDfaqRfFpf;
      string XkrYWwJMzt;
      string fqBKwnlXIy;
      string VuegoAhbHA;
      string IGXMKneREO;
      string YOKJFzCVeD;
      string fHxqKbGLjW;
      string OhEykYTozH;
      string JUSsZHkUao;
      string yMwEUBJgBi;
      string hSghAqJLXf;
      string fMVXtKnDQD;
      string pkjtYBkzcj;
      string PlmQZpmksi;
      string xZwtJkpYKJ;
      string MFPyrwimJy;
      if(JNLSirFtsU == fHxqKbGLjW){FJhnQsZFxE = true;}
      else if(fHxqKbGLjW == JNLSirFtsU){hecYCahgsL = true;}
      if(LdAThwFhRf == OhEykYTozH){XDKhGQookm = true;}
      else if(OhEykYTozH == LdAThwFhRf){GkfCrgnjxj = true;}
      if(DNDzSnMNsh == JUSsZHkUao){LMoTXJWoXd = true;}
      else if(JUSsZHkUao == DNDzSnMNsh){ZUISAunnZD = true;}
      if(YXqbLiaJDk == yMwEUBJgBi){GAbtdmqIdQ = true;}
      else if(yMwEUBJgBi == YXqbLiaJDk){XeaTfjLXtu = true;}
      if(pDfaqRfFpf == hSghAqJLXf){KkaFlJuhra = true;}
      else if(hSghAqJLXf == pDfaqRfFpf){fQNuegwoeN = true;}
      if(XkrYWwJMzt == fMVXtKnDQD){HsBVmrUOQG = true;}
      else if(fMVXtKnDQD == XkrYWwJMzt){kSJYnMVKYG = true;}
      if(fqBKwnlXIy == pkjtYBkzcj){VkgTxXaQES = true;}
      else if(pkjtYBkzcj == fqBKwnlXIy){msHonbJMWH = true;}
      if(VuegoAhbHA == PlmQZpmksi){YfLUcalihN = true;}
      if(IGXMKneREO == xZwtJkpYKJ){OowwWNyGHk = true;}
      if(YOKJFzCVeD == MFPyrwimJy){ScrWtZDGIN = true;}
      while(PlmQZpmksi == VuegoAhbHA){UQpCDIHNnS = true;}
      while(xZwtJkpYKJ == xZwtJkpYKJ){yyIMREVCfS = true;}
      while(MFPyrwimJy == MFPyrwimJy){OBymIlJXGD = true;}
      if(FJhnQsZFxE == true){FJhnQsZFxE = false;}
      if(XDKhGQookm == true){XDKhGQookm = false;}
      if(LMoTXJWoXd == true){LMoTXJWoXd = false;}
      if(GAbtdmqIdQ == true){GAbtdmqIdQ = false;}
      if(KkaFlJuhra == true){KkaFlJuhra = false;}
      if(HsBVmrUOQG == true){HsBVmrUOQG = false;}
      if(VkgTxXaQES == true){VkgTxXaQES = false;}
      if(YfLUcalihN == true){YfLUcalihN = false;}
      if(OowwWNyGHk == true){OowwWNyGHk = false;}
      if(ScrWtZDGIN == true){ScrWtZDGIN = false;}
      if(hecYCahgsL == true){hecYCahgsL = false;}
      if(GkfCrgnjxj == true){GkfCrgnjxj = false;}
      if(ZUISAunnZD == true){ZUISAunnZD = false;}
      if(XeaTfjLXtu == true){XeaTfjLXtu = false;}
      if(fQNuegwoeN == true){fQNuegwoeN = false;}
      if(kSJYnMVKYG == true){kSJYnMVKYG = false;}
      if(msHonbJMWH == true){msHonbJMWH = false;}
      if(UQpCDIHNnS == true){UQpCDIHNnS = false;}
      if(yyIMREVCfS == true){yyIMREVCfS = false;}
      if(OBymIlJXGD == true){OBymIlJXGD = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TXZVGUYXST
{ 
  void IWuzRkAMjS()
  { 
      bool oHVnNqqGzd = false;
      bool IPcwdtaADm = false;
      bool rbdaiPeqbq = false;
      bool AGJcORNKoe = false;
      bool uoebpaNAyQ = false;
      bool NEKeTMctFx = false;
      bool MzeHdBGAgM = false;
      bool gPLXqDwBjI = false;
      bool CdrLduwdIC = false;
      bool nTFqSuJXyn = false;
      bool UMTlixrlmP = false;
      bool PbJxthGTdh = false;
      bool FbqeYNKUFw = false;
      bool sYDumoKGbf = false;
      bool DJqslLJIwk = false;
      bool npsQisVnMV = false;
      bool mXgqWqXNJF = false;
      bool WArxeujysM = false;
      bool DCXUHDemMt = false;
      bool TVtFreczVt = false;
      string KMKxbSmObU;
      string ITksicLzPy;
      string gRIoYfpPKx;
      string YLDCYSpbQL;
      string PTCWxEqaQH;
      string uWcymsDUYu;
      string cXEewxPhTP;
      string ljhEhNOkrT;
      string oOyCBChFPe;
      string OUCUhrOXZA;
      string oIGBULnYQY;
      string eScFGOYCct;
      string PKRoARgNVp;
      string zpKqdgHxgp;
      string OIoqAwrxOT;
      string SMtfkepRAD;
      string eZDZZnHkaZ;
      string GbUmfGVanE;
      string YjzCWmweGz;
      string ppwcLnFORL;
      if(KMKxbSmObU == oIGBULnYQY){oHVnNqqGzd = true;}
      else if(oIGBULnYQY == KMKxbSmObU){UMTlixrlmP = true;}
      if(ITksicLzPy == eScFGOYCct){IPcwdtaADm = true;}
      else if(eScFGOYCct == ITksicLzPy){PbJxthGTdh = true;}
      if(gRIoYfpPKx == PKRoARgNVp){rbdaiPeqbq = true;}
      else if(PKRoARgNVp == gRIoYfpPKx){FbqeYNKUFw = true;}
      if(YLDCYSpbQL == zpKqdgHxgp){AGJcORNKoe = true;}
      else if(zpKqdgHxgp == YLDCYSpbQL){sYDumoKGbf = true;}
      if(PTCWxEqaQH == OIoqAwrxOT){uoebpaNAyQ = true;}
      else if(OIoqAwrxOT == PTCWxEqaQH){DJqslLJIwk = true;}
      if(uWcymsDUYu == SMtfkepRAD){NEKeTMctFx = true;}
      else if(SMtfkepRAD == uWcymsDUYu){npsQisVnMV = true;}
      if(cXEewxPhTP == eZDZZnHkaZ){MzeHdBGAgM = true;}
      else if(eZDZZnHkaZ == cXEewxPhTP){mXgqWqXNJF = true;}
      if(ljhEhNOkrT == GbUmfGVanE){gPLXqDwBjI = true;}
      if(oOyCBChFPe == YjzCWmweGz){CdrLduwdIC = true;}
      if(OUCUhrOXZA == ppwcLnFORL){nTFqSuJXyn = true;}
      while(GbUmfGVanE == ljhEhNOkrT){WArxeujysM = true;}
      while(YjzCWmweGz == YjzCWmweGz){DCXUHDemMt = true;}
      while(ppwcLnFORL == ppwcLnFORL){TVtFreczVt = true;}
      if(oHVnNqqGzd == true){oHVnNqqGzd = false;}
      if(IPcwdtaADm == true){IPcwdtaADm = false;}
      if(rbdaiPeqbq == true){rbdaiPeqbq = false;}
      if(AGJcORNKoe == true){AGJcORNKoe = false;}
      if(uoebpaNAyQ == true){uoebpaNAyQ = false;}
      if(NEKeTMctFx == true){NEKeTMctFx = false;}
      if(MzeHdBGAgM == true){MzeHdBGAgM = false;}
      if(gPLXqDwBjI == true){gPLXqDwBjI = false;}
      if(CdrLduwdIC == true){CdrLduwdIC = false;}
      if(nTFqSuJXyn == true){nTFqSuJXyn = false;}
      if(UMTlixrlmP == true){UMTlixrlmP = false;}
      if(PbJxthGTdh == true){PbJxthGTdh = false;}
      if(FbqeYNKUFw == true){FbqeYNKUFw = false;}
      if(sYDumoKGbf == true){sYDumoKGbf = false;}
      if(DJqslLJIwk == true){DJqslLJIwk = false;}
      if(npsQisVnMV == true){npsQisVnMV = false;}
      if(mXgqWqXNJF == true){mXgqWqXNJF = false;}
      if(WArxeujysM == true){WArxeujysM = false;}
      if(DCXUHDemMt == true){DCXUHDemMt = false;}
      if(TVtFreczVt == true){TVtFreczVt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OTEXPVNAQM
{ 
  void uIoskklfya()
  { 
      bool VLtgDqtdbt = false;
      bool eQnUSzZcjA = false;
      bool uUmwYtWhxd = false;
      bool HplbqfOJNl = false;
      bool ufZIDtJKaw = false;
      bool ImiSDAYxHI = false;
      bool TLgxmneLdl = false;
      bool kQOyFOtese = false;
      bool jLGAVTHSYp = false;
      bool HQBFRizbVO = false;
      bool PygnLrYjNn = false;
      bool FsDcXIEgbS = false;
      bool gMmneTPJgW = false;
      bool PSIdOleRnd = false;
      bool uWFjjNNXgJ = false;
      bool dSzChCLsJq = false;
      bool bODpYmkYNl = false;
      bool dlryWJHCDQ = false;
      bool GqcWrxSFwM = false;
      bool akIxBgTOKw = false;
      string QpwCdsMXlk;
      string ruIOZErbjH;
      string PVhQumdxds;
      string yskfoOtieS;
      string ZyPoxpQUxR;
      string yXZSSuHeHq;
      string OPtsmzJcZL;
      string KJxRhQFsSE;
      string WIzMtZSRMs;
      string GObmDKEWBy;
      string gmulGoaIkC;
      string oOLCUGQhyM;
      string gqPfEugJOW;
      string LiqUXiNNFE;
      string zJgKErDYSw;
      string tWXoRoWOgL;
      string IQtNpFWqek;
      string sDbtMWKFaM;
      string rIpRcoKCDh;
      string LyftwiPTTy;
      if(QpwCdsMXlk == gmulGoaIkC){VLtgDqtdbt = true;}
      else if(gmulGoaIkC == QpwCdsMXlk){PygnLrYjNn = true;}
      if(ruIOZErbjH == oOLCUGQhyM){eQnUSzZcjA = true;}
      else if(oOLCUGQhyM == ruIOZErbjH){FsDcXIEgbS = true;}
      if(PVhQumdxds == gqPfEugJOW){uUmwYtWhxd = true;}
      else if(gqPfEugJOW == PVhQumdxds){gMmneTPJgW = true;}
      if(yskfoOtieS == LiqUXiNNFE){HplbqfOJNl = true;}
      else if(LiqUXiNNFE == yskfoOtieS){PSIdOleRnd = true;}
      if(ZyPoxpQUxR == zJgKErDYSw){ufZIDtJKaw = true;}
      else if(zJgKErDYSw == ZyPoxpQUxR){uWFjjNNXgJ = true;}
      if(yXZSSuHeHq == tWXoRoWOgL){ImiSDAYxHI = true;}
      else if(tWXoRoWOgL == yXZSSuHeHq){dSzChCLsJq = true;}
      if(OPtsmzJcZL == IQtNpFWqek){TLgxmneLdl = true;}
      else if(IQtNpFWqek == OPtsmzJcZL){bODpYmkYNl = true;}
      if(KJxRhQFsSE == sDbtMWKFaM){kQOyFOtese = true;}
      if(WIzMtZSRMs == rIpRcoKCDh){jLGAVTHSYp = true;}
      if(GObmDKEWBy == LyftwiPTTy){HQBFRizbVO = true;}
      while(sDbtMWKFaM == KJxRhQFsSE){dlryWJHCDQ = true;}
      while(rIpRcoKCDh == rIpRcoKCDh){GqcWrxSFwM = true;}
      while(LyftwiPTTy == LyftwiPTTy){akIxBgTOKw = true;}
      if(VLtgDqtdbt == true){VLtgDqtdbt = false;}
      if(eQnUSzZcjA == true){eQnUSzZcjA = false;}
      if(uUmwYtWhxd == true){uUmwYtWhxd = false;}
      if(HplbqfOJNl == true){HplbqfOJNl = false;}
      if(ufZIDtJKaw == true){ufZIDtJKaw = false;}
      if(ImiSDAYxHI == true){ImiSDAYxHI = false;}
      if(TLgxmneLdl == true){TLgxmneLdl = false;}
      if(kQOyFOtese == true){kQOyFOtese = false;}
      if(jLGAVTHSYp == true){jLGAVTHSYp = false;}
      if(HQBFRizbVO == true){HQBFRizbVO = false;}
      if(PygnLrYjNn == true){PygnLrYjNn = false;}
      if(FsDcXIEgbS == true){FsDcXIEgbS = false;}
      if(gMmneTPJgW == true){gMmneTPJgW = false;}
      if(PSIdOleRnd == true){PSIdOleRnd = false;}
      if(uWFjjNNXgJ == true){uWFjjNNXgJ = false;}
      if(dSzChCLsJq == true){dSzChCLsJq = false;}
      if(bODpYmkYNl == true){bODpYmkYNl = false;}
      if(dlryWJHCDQ == true){dlryWJHCDQ = false;}
      if(GqcWrxSFwM == true){GqcWrxSFwM = false;}
      if(akIxBgTOKw == true){akIxBgTOKw = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CGXYPRFINY
{ 
  void imAoYHNNYj()
  { 
      bool iejrnNpTNL = false;
      bool bciBmkmaKi = false;
      bool VSDGYFadaI = false;
      bool hRogmMAhOL = false;
      bool ADARTJIcyG = false;
      bool mqYpgQadTi = false;
      bool lMsFcJnCNa = false;
      bool jFLyhRRllO = false;
      bool wyfUVqsyAV = false;
      bool kWNfYnKunK = false;
      bool VFOXWgfaHO = false;
      bool yorFlnfXil = false;
      bool QoCroNqfYt = false;
      bool HWxarpiIyn = false;
      bool KSqkWwdIsk = false;
      bool HlLBtgDWhI = false;
      bool pABMGbqQMu = false;
      bool FXZubxxQVs = false;
      bool ihlnmVLugL = false;
      bool mMZoyKQeet = false;
      string YQlynwiEfs;
      string cUMPWrIlmP;
      string DouRuRhPXq;
      string rQAdjbDTPF;
      string tSJSUUkbEX;
      string iOBAHjUUDV;
      string NSfoAMUhet;
      string noWgVjecxZ;
      string gGLAIdPzVA;
      string HiQsramTSI;
      string zCwJitqTeW;
      string EqWcoGHfgV;
      string wVMelEXdot;
      string xLKoZdJBQN;
      string mwTyuPMmDk;
      string TGmwLdpRWe;
      string VMdkZsggCL;
      string wFPShABNiV;
      string qkZDZqXNdw;
      string mqdkqZRiYC;
      if(YQlynwiEfs == zCwJitqTeW){iejrnNpTNL = true;}
      else if(zCwJitqTeW == YQlynwiEfs){VFOXWgfaHO = true;}
      if(cUMPWrIlmP == EqWcoGHfgV){bciBmkmaKi = true;}
      else if(EqWcoGHfgV == cUMPWrIlmP){yorFlnfXil = true;}
      if(DouRuRhPXq == wVMelEXdot){VSDGYFadaI = true;}
      else if(wVMelEXdot == DouRuRhPXq){QoCroNqfYt = true;}
      if(rQAdjbDTPF == xLKoZdJBQN){hRogmMAhOL = true;}
      else if(xLKoZdJBQN == rQAdjbDTPF){HWxarpiIyn = true;}
      if(tSJSUUkbEX == mwTyuPMmDk){ADARTJIcyG = true;}
      else if(mwTyuPMmDk == tSJSUUkbEX){KSqkWwdIsk = true;}
      if(iOBAHjUUDV == TGmwLdpRWe){mqYpgQadTi = true;}
      else if(TGmwLdpRWe == iOBAHjUUDV){HlLBtgDWhI = true;}
      if(NSfoAMUhet == VMdkZsggCL){lMsFcJnCNa = true;}
      else if(VMdkZsggCL == NSfoAMUhet){pABMGbqQMu = true;}
      if(noWgVjecxZ == wFPShABNiV){jFLyhRRllO = true;}
      if(gGLAIdPzVA == qkZDZqXNdw){wyfUVqsyAV = true;}
      if(HiQsramTSI == mqdkqZRiYC){kWNfYnKunK = true;}
      while(wFPShABNiV == noWgVjecxZ){FXZubxxQVs = true;}
      while(qkZDZqXNdw == qkZDZqXNdw){ihlnmVLugL = true;}
      while(mqdkqZRiYC == mqdkqZRiYC){mMZoyKQeet = true;}
      if(iejrnNpTNL == true){iejrnNpTNL = false;}
      if(bciBmkmaKi == true){bciBmkmaKi = false;}
      if(VSDGYFadaI == true){VSDGYFadaI = false;}
      if(hRogmMAhOL == true){hRogmMAhOL = false;}
      if(ADARTJIcyG == true){ADARTJIcyG = false;}
      if(mqYpgQadTi == true){mqYpgQadTi = false;}
      if(lMsFcJnCNa == true){lMsFcJnCNa = false;}
      if(jFLyhRRllO == true){jFLyhRRllO = false;}
      if(wyfUVqsyAV == true){wyfUVqsyAV = false;}
      if(kWNfYnKunK == true){kWNfYnKunK = false;}
      if(VFOXWgfaHO == true){VFOXWgfaHO = false;}
      if(yorFlnfXil == true){yorFlnfXil = false;}
      if(QoCroNqfYt == true){QoCroNqfYt = false;}
      if(HWxarpiIyn == true){HWxarpiIyn = false;}
      if(KSqkWwdIsk == true){KSqkWwdIsk = false;}
      if(HlLBtgDWhI == true){HlLBtgDWhI = false;}
      if(pABMGbqQMu == true){pABMGbqQMu = false;}
      if(FXZubxxQVs == true){FXZubxxQVs = false;}
      if(ihlnmVLugL == true){ihlnmVLugL = false;}
      if(mMZoyKQeet == true){mMZoyKQeet = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QLFYBJZSRL
{ 
  void CXrkpHUzVp()
  { 
      bool QwZoQJsNFi = false;
      bool lAbCdTKuOK = false;
      bool OrcuxsLTGw = false;
      bool aeVTTAlKOB = false;
      bool RYRNSyjFcO = false;
      bool yDUDWYTdrX = false;
      bool yZGIIGeHhc = false;
      bool gAgNroZOcz = false;
      bool JGqrxWKSxu = false;
      bool gIRqDrRIFL = false;
      bool TbXzgyWGLg = false;
      bool zEBsNrhOLk = false;
      bool wLJmuYhoJz = false;
      bool WQJReghqKz = false;
      bool AEGPubGYcL = false;
      bool JDxTPHRjEI = false;
      bool QgxLKhujNf = false;
      bool JqELkqFsqS = false;
      bool RILypTToCX = false;
      bool OymChSGuFN = false;
      string YLdCajVeIN;
      string OgHmnUswNA;
      string YmwgLDQCsW;
      string VIezQsyXpo;
      string OOqcQcNxAQ;
      string nDCDMlPwpW;
      string SSllrGBHWu;
      string VUFYlJuSQh;
      string pTCchRFbmd;
      string cChSVucFkK;
      string bFZFtTxdRC;
      string qwfHgzQIDZ;
      string BZUrRTmlCp;
      string fohGGjswhO;
      string TaKjmfVZlW;
      string dUCKJBkhna;
      string fKbaVBOZAw;
      string FTaoFZOjFk;
      string xMAfLLmWxF;
      string zLIGhJziNk;
      if(YLdCajVeIN == bFZFtTxdRC){QwZoQJsNFi = true;}
      else if(bFZFtTxdRC == YLdCajVeIN){TbXzgyWGLg = true;}
      if(OgHmnUswNA == qwfHgzQIDZ){lAbCdTKuOK = true;}
      else if(qwfHgzQIDZ == OgHmnUswNA){zEBsNrhOLk = true;}
      if(YmwgLDQCsW == BZUrRTmlCp){OrcuxsLTGw = true;}
      else if(BZUrRTmlCp == YmwgLDQCsW){wLJmuYhoJz = true;}
      if(VIezQsyXpo == fohGGjswhO){aeVTTAlKOB = true;}
      else if(fohGGjswhO == VIezQsyXpo){WQJReghqKz = true;}
      if(OOqcQcNxAQ == TaKjmfVZlW){RYRNSyjFcO = true;}
      else if(TaKjmfVZlW == OOqcQcNxAQ){AEGPubGYcL = true;}
      if(nDCDMlPwpW == dUCKJBkhna){yDUDWYTdrX = true;}
      else if(dUCKJBkhna == nDCDMlPwpW){JDxTPHRjEI = true;}
      if(SSllrGBHWu == fKbaVBOZAw){yZGIIGeHhc = true;}
      else if(fKbaVBOZAw == SSllrGBHWu){QgxLKhujNf = true;}
      if(VUFYlJuSQh == FTaoFZOjFk){gAgNroZOcz = true;}
      if(pTCchRFbmd == xMAfLLmWxF){JGqrxWKSxu = true;}
      if(cChSVucFkK == zLIGhJziNk){gIRqDrRIFL = true;}
      while(FTaoFZOjFk == VUFYlJuSQh){JqELkqFsqS = true;}
      while(xMAfLLmWxF == xMAfLLmWxF){RILypTToCX = true;}
      while(zLIGhJziNk == zLIGhJziNk){OymChSGuFN = true;}
      if(QwZoQJsNFi == true){QwZoQJsNFi = false;}
      if(lAbCdTKuOK == true){lAbCdTKuOK = false;}
      if(OrcuxsLTGw == true){OrcuxsLTGw = false;}
      if(aeVTTAlKOB == true){aeVTTAlKOB = false;}
      if(RYRNSyjFcO == true){RYRNSyjFcO = false;}
      if(yDUDWYTdrX == true){yDUDWYTdrX = false;}
      if(yZGIIGeHhc == true){yZGIIGeHhc = false;}
      if(gAgNroZOcz == true){gAgNroZOcz = false;}
      if(JGqrxWKSxu == true){JGqrxWKSxu = false;}
      if(gIRqDrRIFL == true){gIRqDrRIFL = false;}
      if(TbXzgyWGLg == true){TbXzgyWGLg = false;}
      if(zEBsNrhOLk == true){zEBsNrhOLk = false;}
      if(wLJmuYhoJz == true){wLJmuYhoJz = false;}
      if(WQJReghqKz == true){WQJReghqKz = false;}
      if(AEGPubGYcL == true){AEGPubGYcL = false;}
      if(JDxTPHRjEI == true){JDxTPHRjEI = false;}
      if(QgxLKhujNf == true){QgxLKhujNf = false;}
      if(JqELkqFsqS == true){JqELkqFsqS = false;}
      if(RILypTToCX == true){RILypTToCX = false;}
      if(OymChSGuFN == true){OymChSGuFN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UPJBVITHIB
{ 
  void rbTgVtpUXR()
  { 
      bool AeMTXMyixr = false;
      bool YyIuwoZWPy = false;
      bool XAjFtzeEHj = false;
      bool LDDfFLznNC = false;
      bool RNzSwGdWQP = false;
      bool xAaESAuGYX = false;
      bool htaMfVucoj = false;
      bool pkJNfJAQIn = false;
      bool UHmrmKbyNF = false;
      bool QrOpTXlmma = false;
      bool LBhfYLqNYI = false;
      bool zdykDaGiZU = false;
      bool swOjSyWTsI = false;
      bool WNdhbSZHSa = false;
      bool TOZEGmtcls = false;
      bool QDMRgPmxMa = false;
      bool nXUPlkzPJj = false;
      bool BzQyBeWKdf = false;
      bool EtgArTBMBM = false;
      bool fbtrlwsJSU = false;
      string HTomydTVYD;
      string GZHpLBlgLP;
      string apFaAAsesC;
      string dDgWnKXmaz;
      string PLkoucNOGj;
      string xPQFKRONlx;
      string jobWZCrjCA;
      string cyFLAPFPeX;
      string meYIEOlbMV;
      string wJAdPZiNPr;
      string jcLFHyJFFu;
      string LyiOJFPSSL;
      string QQXgESxbwZ;
      string frAmnKSSBF;
      string EgWRKyalTK;
      string JPuDbSEeBn;
      string AhIeOTxuZz;
      string mwRMGitGVG;
      string qLHkPQwwLU;
      string FmRMEWXqDp;
      if(HTomydTVYD == jcLFHyJFFu){AeMTXMyixr = true;}
      else if(jcLFHyJFFu == HTomydTVYD){LBhfYLqNYI = true;}
      if(GZHpLBlgLP == LyiOJFPSSL){YyIuwoZWPy = true;}
      else if(LyiOJFPSSL == GZHpLBlgLP){zdykDaGiZU = true;}
      if(apFaAAsesC == QQXgESxbwZ){XAjFtzeEHj = true;}
      else if(QQXgESxbwZ == apFaAAsesC){swOjSyWTsI = true;}
      if(dDgWnKXmaz == frAmnKSSBF){LDDfFLznNC = true;}
      else if(frAmnKSSBF == dDgWnKXmaz){WNdhbSZHSa = true;}
      if(PLkoucNOGj == EgWRKyalTK){RNzSwGdWQP = true;}
      else if(EgWRKyalTK == PLkoucNOGj){TOZEGmtcls = true;}
      if(xPQFKRONlx == JPuDbSEeBn){xAaESAuGYX = true;}
      else if(JPuDbSEeBn == xPQFKRONlx){QDMRgPmxMa = true;}
      if(jobWZCrjCA == AhIeOTxuZz){htaMfVucoj = true;}
      else if(AhIeOTxuZz == jobWZCrjCA){nXUPlkzPJj = true;}
      if(cyFLAPFPeX == mwRMGitGVG){pkJNfJAQIn = true;}
      if(meYIEOlbMV == qLHkPQwwLU){UHmrmKbyNF = true;}
      if(wJAdPZiNPr == FmRMEWXqDp){QrOpTXlmma = true;}
      while(mwRMGitGVG == cyFLAPFPeX){BzQyBeWKdf = true;}
      while(qLHkPQwwLU == qLHkPQwwLU){EtgArTBMBM = true;}
      while(FmRMEWXqDp == FmRMEWXqDp){fbtrlwsJSU = true;}
      if(AeMTXMyixr == true){AeMTXMyixr = false;}
      if(YyIuwoZWPy == true){YyIuwoZWPy = false;}
      if(XAjFtzeEHj == true){XAjFtzeEHj = false;}
      if(LDDfFLznNC == true){LDDfFLznNC = false;}
      if(RNzSwGdWQP == true){RNzSwGdWQP = false;}
      if(xAaESAuGYX == true){xAaESAuGYX = false;}
      if(htaMfVucoj == true){htaMfVucoj = false;}
      if(pkJNfJAQIn == true){pkJNfJAQIn = false;}
      if(UHmrmKbyNF == true){UHmrmKbyNF = false;}
      if(QrOpTXlmma == true){QrOpTXlmma = false;}
      if(LBhfYLqNYI == true){LBhfYLqNYI = false;}
      if(zdykDaGiZU == true){zdykDaGiZU = false;}
      if(swOjSyWTsI == true){swOjSyWTsI = false;}
      if(WNdhbSZHSa == true){WNdhbSZHSa = false;}
      if(TOZEGmtcls == true){TOZEGmtcls = false;}
      if(QDMRgPmxMa == true){QDMRgPmxMa = false;}
      if(nXUPlkzPJj == true){nXUPlkzPJj = false;}
      if(BzQyBeWKdf == true){BzQyBeWKdf = false;}
      if(EtgArTBMBM == true){EtgArTBMBM = false;}
      if(fbtrlwsJSU == true){fbtrlwsJSU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EYTJZZYGXG
{ 
  void VLlUBOyOCJ()
  { 
      bool ingQzwzwgq = false;
      bool XVHdLPDuuj = false;
      bool pkKWagWGkz = false;
      bool uJZHXLJxUW = false;
      bool KINXpDkGNx = false;
      bool bmxYhydJoR = false;
      bool YsKhuNuzGz = false;
      bool GFaXYgWlgX = false;
      bool uQsmWHMBZZ = false;
      bool hzHDyRyfbc = false;
      bool HlANqVMYzB = false;
      bool aqcRVHzXKn = false;
      bool BtZfRGyXZH = false;
      bool ipKDPJQeuB = false;
      bool umCYypWogp = false;
      bool SZVnaNMTwk = false;
      bool WozrRjgYCM = false;
      bool qTcIrOrBgb = false;
      bool iIqjoIUWcB = false;
      bool wxiJdFmJGO = false;
      string llcVDoPzKD;
      string gkrgHKGmdr;
      string JdFdQxQJXH;
      string drhOUhXAGE;
      string nWQMpoXyfM;
      string WgipXHfyEG;
      string YGaDYnCdLY;
      string mGgZElMsmb;
      string YRcKkxJrpM;
      string CYwzfgwNmB;
      string lslHZQzJkl;
      string tmKcgGcQSb;
      string eKnyWNJgsW;
      string pzGlpffVkT;
      string xzHESPNTSY;
      string DetLWaFmkL;
      string GSPNutfYlB;
      string ZHfydeDifG;
      string ApOTSKEuOO;
      string ZsjYNeisIX;
      if(llcVDoPzKD == lslHZQzJkl){ingQzwzwgq = true;}
      else if(lslHZQzJkl == llcVDoPzKD){HlANqVMYzB = true;}
      if(gkrgHKGmdr == tmKcgGcQSb){XVHdLPDuuj = true;}
      else if(tmKcgGcQSb == gkrgHKGmdr){aqcRVHzXKn = true;}
      if(JdFdQxQJXH == eKnyWNJgsW){pkKWagWGkz = true;}
      else if(eKnyWNJgsW == JdFdQxQJXH){BtZfRGyXZH = true;}
      if(drhOUhXAGE == pzGlpffVkT){uJZHXLJxUW = true;}
      else if(pzGlpffVkT == drhOUhXAGE){ipKDPJQeuB = true;}
      if(nWQMpoXyfM == xzHESPNTSY){KINXpDkGNx = true;}
      else if(xzHESPNTSY == nWQMpoXyfM){umCYypWogp = true;}
      if(WgipXHfyEG == DetLWaFmkL){bmxYhydJoR = true;}
      else if(DetLWaFmkL == WgipXHfyEG){SZVnaNMTwk = true;}
      if(YGaDYnCdLY == GSPNutfYlB){YsKhuNuzGz = true;}
      else if(GSPNutfYlB == YGaDYnCdLY){WozrRjgYCM = true;}
      if(mGgZElMsmb == ZHfydeDifG){GFaXYgWlgX = true;}
      if(YRcKkxJrpM == ApOTSKEuOO){uQsmWHMBZZ = true;}
      if(CYwzfgwNmB == ZsjYNeisIX){hzHDyRyfbc = true;}
      while(ZHfydeDifG == mGgZElMsmb){qTcIrOrBgb = true;}
      while(ApOTSKEuOO == ApOTSKEuOO){iIqjoIUWcB = true;}
      while(ZsjYNeisIX == ZsjYNeisIX){wxiJdFmJGO = true;}
      if(ingQzwzwgq == true){ingQzwzwgq = false;}
      if(XVHdLPDuuj == true){XVHdLPDuuj = false;}
      if(pkKWagWGkz == true){pkKWagWGkz = false;}
      if(uJZHXLJxUW == true){uJZHXLJxUW = false;}
      if(KINXpDkGNx == true){KINXpDkGNx = false;}
      if(bmxYhydJoR == true){bmxYhydJoR = false;}
      if(YsKhuNuzGz == true){YsKhuNuzGz = false;}
      if(GFaXYgWlgX == true){GFaXYgWlgX = false;}
      if(uQsmWHMBZZ == true){uQsmWHMBZZ = false;}
      if(hzHDyRyfbc == true){hzHDyRyfbc = false;}
      if(HlANqVMYzB == true){HlANqVMYzB = false;}
      if(aqcRVHzXKn == true){aqcRVHzXKn = false;}
      if(BtZfRGyXZH == true){BtZfRGyXZH = false;}
      if(ipKDPJQeuB == true){ipKDPJQeuB = false;}
      if(umCYypWogp == true){umCYypWogp = false;}
      if(SZVnaNMTwk == true){SZVnaNMTwk = false;}
      if(WozrRjgYCM == true){WozrRjgYCM = false;}
      if(qTcIrOrBgb == true){qTcIrOrBgb = false;}
      if(iIqjoIUWcB == true){iIqjoIUWcB = false;}
      if(wxiJdFmJGO == true){wxiJdFmJGO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NIJIGMICYW
{ 
  void jpXeLJnzQl()
  { 
      bool pIDstjCcmx = false;
      bool kFYAhLhGoV = false;
      bool qaGiVYUBEp = false;
      bool KOyFjCDTul = false;
      bool nnOGMabOYf = false;
      bool mWAZzOCUWV = false;
      bool OBXxmWGTQf = false;
      bool HOLChDMzVu = false;
      bool xGbXCmOYGU = false;
      bool ziwYWmPQTp = false;
      bool THxsYXKxru = false;
      bool CCybYIYztz = false;
      bool HhNwfYRlft = false;
      bool mwfeaoIizd = false;
      bool cRfKSxjUAN = false;
      bool NQVUilsBNz = false;
      bool GYQxIFVSAi = false;
      bool AozQWpBDdh = false;
      bool nxkRfVZLwo = false;
      bool uGVzXDAiBM = false;
      string IJSrlhRNJE;
      string NFsUytNdac;
      string UiChmUzbjg;
      string HRwNZFBNtz;
      string mxKYuDjMOO;
      string LmirmYHkFR;
      string ipdcxwIeur;
      string sAjboWYcmi;
      string jlNUVThDZA;
      string yPHbQxTgby;
      string tEQVVKDHDQ;
      string AdxVgwbIHc;
      string NEEutcrHKq;
      string CqAnnfVqzK;
      string QZAJzTRktU;
      string ULZzkpzUar;
      string SuxLNIcJhB;
      string ZkIxLmEtua;
      string KHMpVqRBDy;
      string fAPGrVcowp;
      if(IJSrlhRNJE == tEQVVKDHDQ){pIDstjCcmx = true;}
      else if(tEQVVKDHDQ == IJSrlhRNJE){THxsYXKxru = true;}
      if(NFsUytNdac == AdxVgwbIHc){kFYAhLhGoV = true;}
      else if(AdxVgwbIHc == NFsUytNdac){CCybYIYztz = true;}
      if(UiChmUzbjg == NEEutcrHKq){qaGiVYUBEp = true;}
      else if(NEEutcrHKq == UiChmUzbjg){HhNwfYRlft = true;}
      if(HRwNZFBNtz == CqAnnfVqzK){KOyFjCDTul = true;}
      else if(CqAnnfVqzK == HRwNZFBNtz){mwfeaoIizd = true;}
      if(mxKYuDjMOO == QZAJzTRktU){nnOGMabOYf = true;}
      else if(QZAJzTRktU == mxKYuDjMOO){cRfKSxjUAN = true;}
      if(LmirmYHkFR == ULZzkpzUar){mWAZzOCUWV = true;}
      else if(ULZzkpzUar == LmirmYHkFR){NQVUilsBNz = true;}
      if(ipdcxwIeur == SuxLNIcJhB){OBXxmWGTQf = true;}
      else if(SuxLNIcJhB == ipdcxwIeur){GYQxIFVSAi = true;}
      if(sAjboWYcmi == ZkIxLmEtua){HOLChDMzVu = true;}
      if(jlNUVThDZA == KHMpVqRBDy){xGbXCmOYGU = true;}
      if(yPHbQxTgby == fAPGrVcowp){ziwYWmPQTp = true;}
      while(ZkIxLmEtua == sAjboWYcmi){AozQWpBDdh = true;}
      while(KHMpVqRBDy == KHMpVqRBDy){nxkRfVZLwo = true;}
      while(fAPGrVcowp == fAPGrVcowp){uGVzXDAiBM = true;}
      if(pIDstjCcmx == true){pIDstjCcmx = false;}
      if(kFYAhLhGoV == true){kFYAhLhGoV = false;}
      if(qaGiVYUBEp == true){qaGiVYUBEp = false;}
      if(KOyFjCDTul == true){KOyFjCDTul = false;}
      if(nnOGMabOYf == true){nnOGMabOYf = false;}
      if(mWAZzOCUWV == true){mWAZzOCUWV = false;}
      if(OBXxmWGTQf == true){OBXxmWGTQf = false;}
      if(HOLChDMzVu == true){HOLChDMzVu = false;}
      if(xGbXCmOYGU == true){xGbXCmOYGU = false;}
      if(ziwYWmPQTp == true){ziwYWmPQTp = false;}
      if(THxsYXKxru == true){THxsYXKxru = false;}
      if(CCybYIYztz == true){CCybYIYztz = false;}
      if(HhNwfYRlft == true){HhNwfYRlft = false;}
      if(mwfeaoIizd == true){mwfeaoIizd = false;}
      if(cRfKSxjUAN == true){cRfKSxjUAN = false;}
      if(NQVUilsBNz == true){NQVUilsBNz = false;}
      if(GYQxIFVSAi == true){GYQxIFVSAi = false;}
      if(AozQWpBDdh == true){AozQWpBDdh = false;}
      if(nxkRfVZLwo == true){nxkRfVZLwo = false;}
      if(uGVzXDAiBM == true){uGVzXDAiBM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RMPWADNPYS
{ 
  void ZCDhhbZdBe()
  { 
      bool MPJluRLzrl = false;
      bool BryPdwYROU = false;
      bool rHfTixkpPW = false;
      bool QEGHkpmziR = false;
      bool wfRdLZgOJB = false;
      bool EBYhxFjoCy = false;
      bool gUiVUwlpdm = false;
      bool JYhwXLpCHI = false;
      bool PVgNSPfXKf = false;
      bool GxwkkawwIO = false;
      bool npjWreSbJk = false;
      bool PhhshKnVCE = false;
      bool QNfyfLcxLx = false;
      bool PssrUYkJVn = false;
      bool xDNYQYODCZ = false;
      bool lZsEyZJlpf = false;
      bool BUPMUVnoYI = false;
      bool YezOGMAHhH = false;
      bool xZulLozott = false;
      bool PzSplcnBDt = false;
      string GbsmXCEXir;
      string SDUYoyCTrs;
      string RoVBZsSKrM;
      string jzZZpHkomL;
      string jZkmqzjlZP;
      string rPgXTRFrMd;
      string jteWHTdthg;
      string MNgfIrEiJa;
      string JfkbTURBGY;
      string CsbIbptNeX;
      string UoErDdhBPa;
      string MWhlUDIDCl;
      string BcnIdSlXDR;
      string ypDlFTarYo;
      string lQkeLfLpmX;
      string hpLrWrBuNk;
      string rJxybWSufB;
      string cPeMzjqIxy;
      string GwkejHeKPM;
      string SfenxcSmCo;
      if(GbsmXCEXir == UoErDdhBPa){MPJluRLzrl = true;}
      else if(UoErDdhBPa == GbsmXCEXir){npjWreSbJk = true;}
      if(SDUYoyCTrs == MWhlUDIDCl){BryPdwYROU = true;}
      else if(MWhlUDIDCl == SDUYoyCTrs){PhhshKnVCE = true;}
      if(RoVBZsSKrM == BcnIdSlXDR){rHfTixkpPW = true;}
      else if(BcnIdSlXDR == RoVBZsSKrM){QNfyfLcxLx = true;}
      if(jzZZpHkomL == ypDlFTarYo){QEGHkpmziR = true;}
      else if(ypDlFTarYo == jzZZpHkomL){PssrUYkJVn = true;}
      if(jZkmqzjlZP == lQkeLfLpmX){wfRdLZgOJB = true;}
      else if(lQkeLfLpmX == jZkmqzjlZP){xDNYQYODCZ = true;}
      if(rPgXTRFrMd == hpLrWrBuNk){EBYhxFjoCy = true;}
      else if(hpLrWrBuNk == rPgXTRFrMd){lZsEyZJlpf = true;}
      if(jteWHTdthg == rJxybWSufB){gUiVUwlpdm = true;}
      else if(rJxybWSufB == jteWHTdthg){BUPMUVnoYI = true;}
      if(MNgfIrEiJa == cPeMzjqIxy){JYhwXLpCHI = true;}
      if(JfkbTURBGY == GwkejHeKPM){PVgNSPfXKf = true;}
      if(CsbIbptNeX == SfenxcSmCo){GxwkkawwIO = true;}
      while(cPeMzjqIxy == MNgfIrEiJa){YezOGMAHhH = true;}
      while(GwkejHeKPM == GwkejHeKPM){xZulLozott = true;}
      while(SfenxcSmCo == SfenxcSmCo){PzSplcnBDt = true;}
      if(MPJluRLzrl == true){MPJluRLzrl = false;}
      if(BryPdwYROU == true){BryPdwYROU = false;}
      if(rHfTixkpPW == true){rHfTixkpPW = false;}
      if(QEGHkpmziR == true){QEGHkpmziR = false;}
      if(wfRdLZgOJB == true){wfRdLZgOJB = false;}
      if(EBYhxFjoCy == true){EBYhxFjoCy = false;}
      if(gUiVUwlpdm == true){gUiVUwlpdm = false;}
      if(JYhwXLpCHI == true){JYhwXLpCHI = false;}
      if(PVgNSPfXKf == true){PVgNSPfXKf = false;}
      if(GxwkkawwIO == true){GxwkkawwIO = false;}
      if(npjWreSbJk == true){npjWreSbJk = false;}
      if(PhhshKnVCE == true){PhhshKnVCE = false;}
      if(QNfyfLcxLx == true){QNfyfLcxLx = false;}
      if(PssrUYkJVn == true){PssrUYkJVn = false;}
      if(xDNYQYODCZ == true){xDNYQYODCZ = false;}
      if(lZsEyZJlpf == true){lZsEyZJlpf = false;}
      if(BUPMUVnoYI == true){BUPMUVnoYI = false;}
      if(YezOGMAHhH == true){YezOGMAHhH = false;}
      if(xZulLozott == true){xZulLozott = false;}
      if(PzSplcnBDt == true){PzSplcnBDt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PAGOGFEERE
{ 
  void jEsuaftrku()
  { 
      bool qFpTfGLcWR = false;
      bool zjaaqdWszQ = false;
      bool HSEeaOcUiW = false;
      bool fTZPriIQVS = false;
      bool iOpNYdzjdy = false;
      bool awDEZYBKrQ = false;
      bool cqtAleKKiI = false;
      bool UGWggrbqGx = false;
      bool PspjwjPUoS = false;
      bool hxXForTpas = false;
      bool SnBVcTZWWg = false;
      bool wJEuBjAJEB = false;
      bool OpVPZXhRrn = false;
      bool XlPMearTHb = false;
      bool KsxHWYscDE = false;
      bool qKlZecSkwF = false;
      bool kHehJKsqQa = false;
      bool rfCbqVrNrB = false;
      bool dFNmxNzpgC = false;
      bool IGXCpCNOyM = false;
      string zZccJoXKMR;
      string HPOblinKff;
      string AeQNNazZBY;
      string nAruhSJGZz;
      string EGstBuZxkl;
      string AiyUETIPPi;
      string pyJndHWSsS;
      string YgjfatlVMS;
      string ACnPXEBWqO;
      string KspqJFwEQs;
      string XHDSMTzRtJ;
      string glDxhnKtPY;
      string FAbeHfqrWU;
      string ygFStsyVDE;
      string EjcggXprIU;
      string uCoqmWeXKg;
      string bGtYLzgjCo;
      string onqLUVXtxi;
      string oyjBZurNeX;
      string NZKVbxoDUh;
      if(zZccJoXKMR == XHDSMTzRtJ){qFpTfGLcWR = true;}
      else if(XHDSMTzRtJ == zZccJoXKMR){SnBVcTZWWg = true;}
      if(HPOblinKff == glDxhnKtPY){zjaaqdWszQ = true;}
      else if(glDxhnKtPY == HPOblinKff){wJEuBjAJEB = true;}
      if(AeQNNazZBY == FAbeHfqrWU){HSEeaOcUiW = true;}
      else if(FAbeHfqrWU == AeQNNazZBY){OpVPZXhRrn = true;}
      if(nAruhSJGZz == ygFStsyVDE){fTZPriIQVS = true;}
      else if(ygFStsyVDE == nAruhSJGZz){XlPMearTHb = true;}
      if(EGstBuZxkl == EjcggXprIU){iOpNYdzjdy = true;}
      else if(EjcggXprIU == EGstBuZxkl){KsxHWYscDE = true;}
      if(AiyUETIPPi == uCoqmWeXKg){awDEZYBKrQ = true;}
      else if(uCoqmWeXKg == AiyUETIPPi){qKlZecSkwF = true;}
      if(pyJndHWSsS == bGtYLzgjCo){cqtAleKKiI = true;}
      else if(bGtYLzgjCo == pyJndHWSsS){kHehJKsqQa = true;}
      if(YgjfatlVMS == onqLUVXtxi){UGWggrbqGx = true;}
      if(ACnPXEBWqO == oyjBZurNeX){PspjwjPUoS = true;}
      if(KspqJFwEQs == NZKVbxoDUh){hxXForTpas = true;}
      while(onqLUVXtxi == YgjfatlVMS){rfCbqVrNrB = true;}
      while(oyjBZurNeX == oyjBZurNeX){dFNmxNzpgC = true;}
      while(NZKVbxoDUh == NZKVbxoDUh){IGXCpCNOyM = true;}
      if(qFpTfGLcWR == true){qFpTfGLcWR = false;}
      if(zjaaqdWszQ == true){zjaaqdWszQ = false;}
      if(HSEeaOcUiW == true){HSEeaOcUiW = false;}
      if(fTZPriIQVS == true){fTZPriIQVS = false;}
      if(iOpNYdzjdy == true){iOpNYdzjdy = false;}
      if(awDEZYBKrQ == true){awDEZYBKrQ = false;}
      if(cqtAleKKiI == true){cqtAleKKiI = false;}
      if(UGWggrbqGx == true){UGWggrbqGx = false;}
      if(PspjwjPUoS == true){PspjwjPUoS = false;}
      if(hxXForTpas == true){hxXForTpas = false;}
      if(SnBVcTZWWg == true){SnBVcTZWWg = false;}
      if(wJEuBjAJEB == true){wJEuBjAJEB = false;}
      if(OpVPZXhRrn == true){OpVPZXhRrn = false;}
      if(XlPMearTHb == true){XlPMearTHb = false;}
      if(KsxHWYscDE == true){KsxHWYscDE = false;}
      if(qKlZecSkwF == true){qKlZecSkwF = false;}
      if(kHehJKsqQa == true){kHehJKsqQa = false;}
      if(rfCbqVrNrB == true){rfCbqVrNrB = false;}
      if(dFNmxNzpgC == true){dFNmxNzpgC = false;}
      if(IGXCpCNOyM == true){IGXCpCNOyM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CLNZEWNPQA
{ 
  void ATAMmfdECs()
  { 
      bool uKsQtoViQj = false;
      bool DAUWugeHAT = false;
      bool KDNgcEMKXd = false;
      bool aPNItyLLUZ = false;
      bool fVylWjeLVA = false;
      bool PYTaWrxLnp = false;
      bool cLLmInlMbw = false;
      bool UTmamoFaJr = false;
      bool YAoXNCumkC = false;
      bool BAXdwUJLdi = false;
      bool ySiYewksmh = false;
      bool lVBuRcVnYG = false;
      bool ryaZuUuJdR = false;
      bool esxatFQQrQ = false;
      bool QCzDPeybLT = false;
      bool yVmWqzrJcJ = false;
      bool AbOVCsYaBM = false;
      bool LFuHopzXJQ = false;
      bool WTxKSJGZiu = false;
      bool NHmhPomdAW = false;
      string llEawsjJZZ;
      string OHOdcPlIcO;
      string PQohnQNbik;
      string cKpDejRIPw;
      string lUFQLVqALG;
      string WISrQVxWme;
      string eIZoxEQKys;
      string ycrfctKYWR;
      string YNPcOTqylJ;
      string mtVjQctPSG;
      string FfBSHsPlOV;
      string KYAUFRrwac;
      string NSXoFDGHOz;
      string AtFPMrMwUU;
      string DEiPyOtBez;
      string kxopslILNE;
      string mIXOVPSjHN;
      string EubFYIiHqp;
      string IFcPOCFPWZ;
      string jAiQhugTth;
      if(llEawsjJZZ == FfBSHsPlOV){uKsQtoViQj = true;}
      else if(FfBSHsPlOV == llEawsjJZZ){ySiYewksmh = true;}
      if(OHOdcPlIcO == KYAUFRrwac){DAUWugeHAT = true;}
      else if(KYAUFRrwac == OHOdcPlIcO){lVBuRcVnYG = true;}
      if(PQohnQNbik == NSXoFDGHOz){KDNgcEMKXd = true;}
      else if(NSXoFDGHOz == PQohnQNbik){ryaZuUuJdR = true;}
      if(cKpDejRIPw == AtFPMrMwUU){aPNItyLLUZ = true;}
      else if(AtFPMrMwUU == cKpDejRIPw){esxatFQQrQ = true;}
      if(lUFQLVqALG == DEiPyOtBez){fVylWjeLVA = true;}
      else if(DEiPyOtBez == lUFQLVqALG){QCzDPeybLT = true;}
      if(WISrQVxWme == kxopslILNE){PYTaWrxLnp = true;}
      else if(kxopslILNE == WISrQVxWme){yVmWqzrJcJ = true;}
      if(eIZoxEQKys == mIXOVPSjHN){cLLmInlMbw = true;}
      else if(mIXOVPSjHN == eIZoxEQKys){AbOVCsYaBM = true;}
      if(ycrfctKYWR == EubFYIiHqp){UTmamoFaJr = true;}
      if(YNPcOTqylJ == IFcPOCFPWZ){YAoXNCumkC = true;}
      if(mtVjQctPSG == jAiQhugTth){BAXdwUJLdi = true;}
      while(EubFYIiHqp == ycrfctKYWR){LFuHopzXJQ = true;}
      while(IFcPOCFPWZ == IFcPOCFPWZ){WTxKSJGZiu = true;}
      while(jAiQhugTth == jAiQhugTth){NHmhPomdAW = true;}
      if(uKsQtoViQj == true){uKsQtoViQj = false;}
      if(DAUWugeHAT == true){DAUWugeHAT = false;}
      if(KDNgcEMKXd == true){KDNgcEMKXd = false;}
      if(aPNItyLLUZ == true){aPNItyLLUZ = false;}
      if(fVylWjeLVA == true){fVylWjeLVA = false;}
      if(PYTaWrxLnp == true){PYTaWrxLnp = false;}
      if(cLLmInlMbw == true){cLLmInlMbw = false;}
      if(UTmamoFaJr == true){UTmamoFaJr = false;}
      if(YAoXNCumkC == true){YAoXNCumkC = false;}
      if(BAXdwUJLdi == true){BAXdwUJLdi = false;}
      if(ySiYewksmh == true){ySiYewksmh = false;}
      if(lVBuRcVnYG == true){lVBuRcVnYG = false;}
      if(ryaZuUuJdR == true){ryaZuUuJdR = false;}
      if(esxatFQQrQ == true){esxatFQQrQ = false;}
      if(QCzDPeybLT == true){QCzDPeybLT = false;}
      if(yVmWqzrJcJ == true){yVmWqzrJcJ = false;}
      if(AbOVCsYaBM == true){AbOVCsYaBM = false;}
      if(LFuHopzXJQ == true){LFuHopzXJQ = false;}
      if(WTxKSJGZiu == true){WTxKSJGZiu = false;}
      if(NHmhPomdAW == true){NHmhPomdAW = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZYVBAAJNWB
{ 
  void qSZDcekzqz()
  { 
      bool UjfsRuxGXH = false;
      bool WMIKJPpnRK = false;
      bool dQrkGeNPJH = false;
      bool WgBOURDKwk = false;
      bool wssZdVddje = false;
      bool EVhqoDruml = false;
      bool hgxQjpKspr = false;
      bool MfrgOkXRGj = false;
      bool AifdCByKZQ = false;
      bool YIXJDOucon = false;
      bool KjnrSYdBbD = false;
      bool faoCkyBTOi = false;
      bool qATYSEHhcz = false;
      bool pubSYQunLc = false;
      bool WxhVjxTUXE = false;
      bool NaNFwKQOYO = false;
      bool RuXcNkyVdU = false;
      bool DMHlLsSxoV = false;
      bool klKXkOgVVO = false;
      bool zfPxLbZEHz = false;
      string noFGTceOEe;
      string tgCPufBlto;
      string jcAAaKkDVJ;
      string NlajyVModU;
      string SNGlsmomTr;
      string NoYzaLoEeI;
      string rbruRIFRRZ;
      string TLIZVVVNHm;
      string UyidwXMaJl;
      string fHJVcqVKUj;
      string hWUtXBgzcE;
      string obxqmtrOzs;
      string udHesMBlwe;
      string ALsWFycoFt;
      string pNxFxEKShi;
      string yGLaoDOpAL;
      string ztCyDdiZlo;
      string QDJEChfCho;
      string CiENfszRhB;
      string ilosuuKzBS;
      if(noFGTceOEe == hWUtXBgzcE){UjfsRuxGXH = true;}
      else if(hWUtXBgzcE == noFGTceOEe){KjnrSYdBbD = true;}
      if(tgCPufBlto == obxqmtrOzs){WMIKJPpnRK = true;}
      else if(obxqmtrOzs == tgCPufBlto){faoCkyBTOi = true;}
      if(jcAAaKkDVJ == udHesMBlwe){dQrkGeNPJH = true;}
      else if(udHesMBlwe == jcAAaKkDVJ){qATYSEHhcz = true;}
      if(NlajyVModU == ALsWFycoFt){WgBOURDKwk = true;}
      else if(ALsWFycoFt == NlajyVModU){pubSYQunLc = true;}
      if(SNGlsmomTr == pNxFxEKShi){wssZdVddje = true;}
      else if(pNxFxEKShi == SNGlsmomTr){WxhVjxTUXE = true;}
      if(NoYzaLoEeI == yGLaoDOpAL){EVhqoDruml = true;}
      else if(yGLaoDOpAL == NoYzaLoEeI){NaNFwKQOYO = true;}
      if(rbruRIFRRZ == ztCyDdiZlo){hgxQjpKspr = true;}
      else if(ztCyDdiZlo == rbruRIFRRZ){RuXcNkyVdU = true;}
      if(TLIZVVVNHm == QDJEChfCho){MfrgOkXRGj = true;}
      if(UyidwXMaJl == CiENfszRhB){AifdCByKZQ = true;}
      if(fHJVcqVKUj == ilosuuKzBS){YIXJDOucon = true;}
      while(QDJEChfCho == TLIZVVVNHm){DMHlLsSxoV = true;}
      while(CiENfszRhB == CiENfszRhB){klKXkOgVVO = true;}
      while(ilosuuKzBS == ilosuuKzBS){zfPxLbZEHz = true;}
      if(UjfsRuxGXH == true){UjfsRuxGXH = false;}
      if(WMIKJPpnRK == true){WMIKJPpnRK = false;}
      if(dQrkGeNPJH == true){dQrkGeNPJH = false;}
      if(WgBOURDKwk == true){WgBOURDKwk = false;}
      if(wssZdVddje == true){wssZdVddje = false;}
      if(EVhqoDruml == true){EVhqoDruml = false;}
      if(hgxQjpKspr == true){hgxQjpKspr = false;}
      if(MfrgOkXRGj == true){MfrgOkXRGj = false;}
      if(AifdCByKZQ == true){AifdCByKZQ = false;}
      if(YIXJDOucon == true){YIXJDOucon = false;}
      if(KjnrSYdBbD == true){KjnrSYdBbD = false;}
      if(faoCkyBTOi == true){faoCkyBTOi = false;}
      if(qATYSEHhcz == true){qATYSEHhcz = false;}
      if(pubSYQunLc == true){pubSYQunLc = false;}
      if(WxhVjxTUXE == true){WxhVjxTUXE = false;}
      if(NaNFwKQOYO == true){NaNFwKQOYO = false;}
      if(RuXcNkyVdU == true){RuXcNkyVdU = false;}
      if(DMHlLsSxoV == true){DMHlLsSxoV = false;}
      if(klKXkOgVVO == true){klKXkOgVVO = false;}
      if(zfPxLbZEHz == true){zfPxLbZEHz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TCTIREWJNN
{ 
  void ITwpebZuuL()
  { 
      bool QWCpBBtVDB = false;
      bool STMYMOHlBg = false;
      bool AncZYGMmXr = false;
      bool yXluuwcktB = false;
      bool xUVpCmgplx = false;
      bool RYcfHctcRt = false;
      bool nGxLCthRcP = false;
      bool lWrzjfRlWk = false;
      bool YVAaZZuFmR = false;
      bool CzTzMpXKmH = false;
      bool EPMVKufcwb = false;
      bool LHFfVSFWkc = false;
      bool zcdLNmCrms = false;
      bool IGzPgIIePe = false;
      bool sRJLFutmds = false;
      bool kAZKHblBIX = false;
      bool rnoDSFIpzM = false;
      bool KaGGigSRFS = false;
      bool sPOsHHgnHE = false;
      bool WGVqmFcFrA = false;
      string JExXpwxdkQ;
      string unUWRaSpYE;
      string LWEIwDAnnM;
      string ySmBblCIPL;
      string QoYNqDXQIQ;
      string gHsWSmznbl;
      string wJpYRcATte;
      string YJaMmWxqQr;
      string fieXepYQho;
      string mNgcPHzAwO;
      string DEWdqXgfiw;
      string oIWiGNdSsa;
      string DikEqKcxxf;
      string XxuCoaMBfH;
      string JsglAeqxVC;
      string cfGUKTTRlg;
      string tihDYqNTtf;
      string BblkRJBzEM;
      string TebuCqpZqU;
      string HtPzMZLdNI;
      if(JExXpwxdkQ == DEWdqXgfiw){QWCpBBtVDB = true;}
      else if(DEWdqXgfiw == JExXpwxdkQ){EPMVKufcwb = true;}
      if(unUWRaSpYE == oIWiGNdSsa){STMYMOHlBg = true;}
      else if(oIWiGNdSsa == unUWRaSpYE){LHFfVSFWkc = true;}
      if(LWEIwDAnnM == DikEqKcxxf){AncZYGMmXr = true;}
      else if(DikEqKcxxf == LWEIwDAnnM){zcdLNmCrms = true;}
      if(ySmBblCIPL == XxuCoaMBfH){yXluuwcktB = true;}
      else if(XxuCoaMBfH == ySmBblCIPL){IGzPgIIePe = true;}
      if(QoYNqDXQIQ == JsglAeqxVC){xUVpCmgplx = true;}
      else if(JsglAeqxVC == QoYNqDXQIQ){sRJLFutmds = true;}
      if(gHsWSmznbl == cfGUKTTRlg){RYcfHctcRt = true;}
      else if(cfGUKTTRlg == gHsWSmznbl){kAZKHblBIX = true;}
      if(wJpYRcATte == tihDYqNTtf){nGxLCthRcP = true;}
      else if(tihDYqNTtf == wJpYRcATte){rnoDSFIpzM = true;}
      if(YJaMmWxqQr == BblkRJBzEM){lWrzjfRlWk = true;}
      if(fieXepYQho == TebuCqpZqU){YVAaZZuFmR = true;}
      if(mNgcPHzAwO == HtPzMZLdNI){CzTzMpXKmH = true;}
      while(BblkRJBzEM == YJaMmWxqQr){KaGGigSRFS = true;}
      while(TebuCqpZqU == TebuCqpZqU){sPOsHHgnHE = true;}
      while(HtPzMZLdNI == HtPzMZLdNI){WGVqmFcFrA = true;}
      if(QWCpBBtVDB == true){QWCpBBtVDB = false;}
      if(STMYMOHlBg == true){STMYMOHlBg = false;}
      if(AncZYGMmXr == true){AncZYGMmXr = false;}
      if(yXluuwcktB == true){yXluuwcktB = false;}
      if(xUVpCmgplx == true){xUVpCmgplx = false;}
      if(RYcfHctcRt == true){RYcfHctcRt = false;}
      if(nGxLCthRcP == true){nGxLCthRcP = false;}
      if(lWrzjfRlWk == true){lWrzjfRlWk = false;}
      if(YVAaZZuFmR == true){YVAaZZuFmR = false;}
      if(CzTzMpXKmH == true){CzTzMpXKmH = false;}
      if(EPMVKufcwb == true){EPMVKufcwb = false;}
      if(LHFfVSFWkc == true){LHFfVSFWkc = false;}
      if(zcdLNmCrms == true){zcdLNmCrms = false;}
      if(IGzPgIIePe == true){IGzPgIIePe = false;}
      if(sRJLFutmds == true){sRJLFutmds = false;}
      if(kAZKHblBIX == true){kAZKHblBIX = false;}
      if(rnoDSFIpzM == true){rnoDSFIpzM = false;}
      if(KaGGigSRFS == true){KaGGigSRFS = false;}
      if(sPOsHHgnHE == true){sPOsHHgnHE = false;}
      if(WGVqmFcFrA == true){WGVqmFcFrA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZPOFTHOIGN
{ 
  void GDuaKVzrTd()
  { 
      bool QWkpyDslyq = false;
      bool jszVGTMPIT = false;
      bool QqhlpoOSWr = false;
      bool zlEFAwqBjK = false;
      bool nMdGSuCqpw = false;
      bool IloPiYnzAI = false;
      bool oZOGqxotVJ = false;
      bool cLfxeDNbTt = false;
      bool lWcaiMxnVa = false;
      bool IrAaVOyXOU = false;
      bool FIVfTxGqke = false;
      bool LLfGWRxnWd = false;
      bool ABXtBjKFbQ = false;
      bool FpNyndAwHl = false;
      bool GFDqOwMtDD = false;
      bool QJAjUEjaei = false;
      bool lBPTZVxgRs = false;
      bool QAgjXtygeE = false;
      bool XQBXzuoddj = false;
      bool sZRXAFpiRs = false;
      string TyHWMUtFGT;
      string ezBPYWgkoj;
      string chIBOikYEu;
      string qGHBpgctAZ;
      string ftjYRQGBhn;
      string kBkVZlpFId;
      string jVBKbXqAZr;
      string smjOEixijx;
      string ciqQKpVlbE;
      string pXkWpKsYwt;
      string ZFaotxQVQq;
      string nGVLBAdkAl;
      string BFYVBVBMXt;
      string oXmoWWIBVM;
      string YAUkPjCZVp;
      string fIYKrUgFmY;
      string wExWBPTfne;
      string rcOmMBjnFR;
      string SbqhNgybxC;
      string oCNwQUCxiA;
      if(TyHWMUtFGT == ZFaotxQVQq){QWkpyDslyq = true;}
      else if(ZFaotxQVQq == TyHWMUtFGT){FIVfTxGqke = true;}
      if(ezBPYWgkoj == nGVLBAdkAl){jszVGTMPIT = true;}
      else if(nGVLBAdkAl == ezBPYWgkoj){LLfGWRxnWd = true;}
      if(chIBOikYEu == BFYVBVBMXt){QqhlpoOSWr = true;}
      else if(BFYVBVBMXt == chIBOikYEu){ABXtBjKFbQ = true;}
      if(qGHBpgctAZ == oXmoWWIBVM){zlEFAwqBjK = true;}
      else if(oXmoWWIBVM == qGHBpgctAZ){FpNyndAwHl = true;}
      if(ftjYRQGBhn == YAUkPjCZVp){nMdGSuCqpw = true;}
      else if(YAUkPjCZVp == ftjYRQGBhn){GFDqOwMtDD = true;}
      if(kBkVZlpFId == fIYKrUgFmY){IloPiYnzAI = true;}
      else if(fIYKrUgFmY == kBkVZlpFId){QJAjUEjaei = true;}
      if(jVBKbXqAZr == wExWBPTfne){oZOGqxotVJ = true;}
      else if(wExWBPTfne == jVBKbXqAZr){lBPTZVxgRs = true;}
      if(smjOEixijx == rcOmMBjnFR){cLfxeDNbTt = true;}
      if(ciqQKpVlbE == SbqhNgybxC){lWcaiMxnVa = true;}
      if(pXkWpKsYwt == oCNwQUCxiA){IrAaVOyXOU = true;}
      while(rcOmMBjnFR == smjOEixijx){QAgjXtygeE = true;}
      while(SbqhNgybxC == SbqhNgybxC){XQBXzuoddj = true;}
      while(oCNwQUCxiA == oCNwQUCxiA){sZRXAFpiRs = true;}
      if(QWkpyDslyq == true){QWkpyDslyq = false;}
      if(jszVGTMPIT == true){jszVGTMPIT = false;}
      if(QqhlpoOSWr == true){QqhlpoOSWr = false;}
      if(zlEFAwqBjK == true){zlEFAwqBjK = false;}
      if(nMdGSuCqpw == true){nMdGSuCqpw = false;}
      if(IloPiYnzAI == true){IloPiYnzAI = false;}
      if(oZOGqxotVJ == true){oZOGqxotVJ = false;}
      if(cLfxeDNbTt == true){cLfxeDNbTt = false;}
      if(lWcaiMxnVa == true){lWcaiMxnVa = false;}
      if(IrAaVOyXOU == true){IrAaVOyXOU = false;}
      if(FIVfTxGqke == true){FIVfTxGqke = false;}
      if(LLfGWRxnWd == true){LLfGWRxnWd = false;}
      if(ABXtBjKFbQ == true){ABXtBjKFbQ = false;}
      if(FpNyndAwHl == true){FpNyndAwHl = false;}
      if(GFDqOwMtDD == true){GFDqOwMtDD = false;}
      if(QJAjUEjaei == true){QJAjUEjaei = false;}
      if(lBPTZVxgRs == true){lBPTZVxgRs = false;}
      if(QAgjXtygeE == true){QAgjXtygeE = false;}
      if(XQBXzuoddj == true){XQBXzuoddj = false;}
      if(sZRXAFpiRs == true){sZRXAFpiRs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HLTMZRTIMP
{ 
  void ufHLCQQUYK()
  { 
      bool goIPTCKmzb = false;
      bool RdKBnCKRXz = false;
      bool nPZCRDinby = false;
      bool hrzUIkmSku = false;
      bool OihzTepuwb = false;
      bool nFFgOQOGxQ = false;
      bool TVLAtYeosD = false;
      bool PqfckEgZhm = false;
      bool MmqzKclRGq = false;
      bool VfVTarXhFu = false;
      bool AACrMwYcwJ = false;
      bool inXKDgeLoR = false;
      bool lMhBgiVLBr = false;
      bool fzUmlLjylO = false;
      bool DIUwIhILQt = false;
      bool NMrmImRBKd = false;
      bool aMbDdiSkXr = false;
      bool wfYaNcqlsz = false;
      bool aKuGdSHgey = false;
      bool sinImpgRSY = false;
      string BCLFCjFdOq;
      string YEKidWFKsG;
      string VdGKKpslcK;
      string phjiKCCEzD;
      string WHhnVcYqly;
      string qeojuWuAuM;
      string qTBDBpUaXb;
      string TBLpiOuSoh;
      string RCzdTLrznn;
      string HCJlMDGeme;
      string IxSKepSzBg;
      string acLMunUXjp;
      string yUmjTQrZcf;
      string HQeJYTjnyc;
      string rsyGMeqwqX;
      string iitxgnKhqN;
      string ysVUYKxCma;
      string EfYGxEVfXo;
      string ZuiExYWhbk;
      string JBxkeyWdgC;
      if(BCLFCjFdOq == IxSKepSzBg){goIPTCKmzb = true;}
      else if(IxSKepSzBg == BCLFCjFdOq){AACrMwYcwJ = true;}
      if(YEKidWFKsG == acLMunUXjp){RdKBnCKRXz = true;}
      else if(acLMunUXjp == YEKidWFKsG){inXKDgeLoR = true;}
      if(VdGKKpslcK == yUmjTQrZcf){nPZCRDinby = true;}
      else if(yUmjTQrZcf == VdGKKpslcK){lMhBgiVLBr = true;}
      if(phjiKCCEzD == HQeJYTjnyc){hrzUIkmSku = true;}
      else if(HQeJYTjnyc == phjiKCCEzD){fzUmlLjylO = true;}
      if(WHhnVcYqly == rsyGMeqwqX){OihzTepuwb = true;}
      else if(rsyGMeqwqX == WHhnVcYqly){DIUwIhILQt = true;}
      if(qeojuWuAuM == iitxgnKhqN){nFFgOQOGxQ = true;}
      else if(iitxgnKhqN == qeojuWuAuM){NMrmImRBKd = true;}
      if(qTBDBpUaXb == ysVUYKxCma){TVLAtYeosD = true;}
      else if(ysVUYKxCma == qTBDBpUaXb){aMbDdiSkXr = true;}
      if(TBLpiOuSoh == EfYGxEVfXo){PqfckEgZhm = true;}
      if(RCzdTLrznn == ZuiExYWhbk){MmqzKclRGq = true;}
      if(HCJlMDGeme == JBxkeyWdgC){VfVTarXhFu = true;}
      while(EfYGxEVfXo == TBLpiOuSoh){wfYaNcqlsz = true;}
      while(ZuiExYWhbk == ZuiExYWhbk){aKuGdSHgey = true;}
      while(JBxkeyWdgC == JBxkeyWdgC){sinImpgRSY = true;}
      if(goIPTCKmzb == true){goIPTCKmzb = false;}
      if(RdKBnCKRXz == true){RdKBnCKRXz = false;}
      if(nPZCRDinby == true){nPZCRDinby = false;}
      if(hrzUIkmSku == true){hrzUIkmSku = false;}
      if(OihzTepuwb == true){OihzTepuwb = false;}
      if(nFFgOQOGxQ == true){nFFgOQOGxQ = false;}
      if(TVLAtYeosD == true){TVLAtYeosD = false;}
      if(PqfckEgZhm == true){PqfckEgZhm = false;}
      if(MmqzKclRGq == true){MmqzKclRGq = false;}
      if(VfVTarXhFu == true){VfVTarXhFu = false;}
      if(AACrMwYcwJ == true){AACrMwYcwJ = false;}
      if(inXKDgeLoR == true){inXKDgeLoR = false;}
      if(lMhBgiVLBr == true){lMhBgiVLBr = false;}
      if(fzUmlLjylO == true){fzUmlLjylO = false;}
      if(DIUwIhILQt == true){DIUwIhILQt = false;}
      if(NMrmImRBKd == true){NMrmImRBKd = false;}
      if(aMbDdiSkXr == true){aMbDdiSkXr = false;}
      if(wfYaNcqlsz == true){wfYaNcqlsz = false;}
      if(aKuGdSHgey == true){aKuGdSHgey = false;}
      if(sinImpgRSY == true){sinImpgRSY = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DPZDZWMPWP
{ 
  void naeKJCJPRh()
  { 
      bool rKRexwEJtw = false;
      bool wmZwkaDLKg = false;
      bool lqfiqFXQhW = false;
      bool ZLtOVGaZfl = false;
      bool hMVGggtbbN = false;
      bool DNbZNBeinq = false;
      bool YmbXWjsLZe = false;
      bool zceXyZUZBJ = false;
      bool LuoTSLjUBU = false;
      bool arlnMujyix = false;
      bool nhCwOahDam = false;
      bool gFhdedxIFK = false;
      bool zXKFDHpqLD = false;
      bool pVpDUdsrFq = false;
      bool dgtNmVKHIi = false;
      bool xbsYxQXxrG = false;
      bool kqrXxKOdJB = false;
      bool jFDNfScwRh = false;
      bool xhhbFOcRFE = false;
      bool FwzyGmnWQc = false;
      string fuRZochuEH;
      string StynPuecSH;
      string zFqetoYuYa;
      string qCRFuCLsPK;
      string JApiYzLDmw;
      string bTySnaEleA;
      string EmWdQQcsrM;
      string ikNIyyOHGt;
      string cUIcyWEzQU;
      string xZOPGWcLSH;
      string WRLZMpGpUo;
      string LxfjWidtet;
      string MccWOJWxBa;
      string VDoyXKIiPI;
      string PNeaIAsdrX;
      string rigYutonKC;
      string TIosTIHhMM;
      string ZudWdyxBOO;
      string QrFcimVQCd;
      string UTXNoGYlGz;
      if(fuRZochuEH == WRLZMpGpUo){rKRexwEJtw = true;}
      else if(WRLZMpGpUo == fuRZochuEH){nhCwOahDam = true;}
      if(StynPuecSH == LxfjWidtet){wmZwkaDLKg = true;}
      else if(LxfjWidtet == StynPuecSH){gFhdedxIFK = true;}
      if(zFqetoYuYa == MccWOJWxBa){lqfiqFXQhW = true;}
      else if(MccWOJWxBa == zFqetoYuYa){zXKFDHpqLD = true;}
      if(qCRFuCLsPK == VDoyXKIiPI){ZLtOVGaZfl = true;}
      else if(VDoyXKIiPI == qCRFuCLsPK){pVpDUdsrFq = true;}
      if(JApiYzLDmw == PNeaIAsdrX){hMVGggtbbN = true;}
      else if(PNeaIAsdrX == JApiYzLDmw){dgtNmVKHIi = true;}
      if(bTySnaEleA == rigYutonKC){DNbZNBeinq = true;}
      else if(rigYutonKC == bTySnaEleA){xbsYxQXxrG = true;}
      if(EmWdQQcsrM == TIosTIHhMM){YmbXWjsLZe = true;}
      else if(TIosTIHhMM == EmWdQQcsrM){kqrXxKOdJB = true;}
      if(ikNIyyOHGt == ZudWdyxBOO){zceXyZUZBJ = true;}
      if(cUIcyWEzQU == QrFcimVQCd){LuoTSLjUBU = true;}
      if(xZOPGWcLSH == UTXNoGYlGz){arlnMujyix = true;}
      while(ZudWdyxBOO == ikNIyyOHGt){jFDNfScwRh = true;}
      while(QrFcimVQCd == QrFcimVQCd){xhhbFOcRFE = true;}
      while(UTXNoGYlGz == UTXNoGYlGz){FwzyGmnWQc = true;}
      if(rKRexwEJtw == true){rKRexwEJtw = false;}
      if(wmZwkaDLKg == true){wmZwkaDLKg = false;}
      if(lqfiqFXQhW == true){lqfiqFXQhW = false;}
      if(ZLtOVGaZfl == true){ZLtOVGaZfl = false;}
      if(hMVGggtbbN == true){hMVGggtbbN = false;}
      if(DNbZNBeinq == true){DNbZNBeinq = false;}
      if(YmbXWjsLZe == true){YmbXWjsLZe = false;}
      if(zceXyZUZBJ == true){zceXyZUZBJ = false;}
      if(LuoTSLjUBU == true){LuoTSLjUBU = false;}
      if(arlnMujyix == true){arlnMujyix = false;}
      if(nhCwOahDam == true){nhCwOahDam = false;}
      if(gFhdedxIFK == true){gFhdedxIFK = false;}
      if(zXKFDHpqLD == true){zXKFDHpqLD = false;}
      if(pVpDUdsrFq == true){pVpDUdsrFq = false;}
      if(dgtNmVKHIi == true){dgtNmVKHIi = false;}
      if(xbsYxQXxrG == true){xbsYxQXxrG = false;}
      if(kqrXxKOdJB == true){kqrXxKOdJB = false;}
      if(jFDNfScwRh == true){jFDNfScwRh = false;}
      if(xhhbFOcRFE == true){xhhbFOcRFE = false;}
      if(FwzyGmnWQc == true){FwzyGmnWQc = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XFSEXAKXIW
{ 
  void aqTrHaXAdC()
  { 
      bool AYCGdyGfTx = false;
      bool PhwTogrmej = false;
      bool hJhSgGVmRJ = false;
      bool aHkddSKfCC = false;
      bool ntkqBpQtdO = false;
      bool aSJSlpqjYX = false;
      bool cSNJWHxSpV = false;
      bool VMjbwnYpQp = false;
      bool paIDiHyRrm = false;
      bool bZGeZUfbUs = false;
      bool syEHrOigWy = false;
      bool RBSigBKKaO = false;
      bool JfZCtwFECb = false;
      bool IwUWxrWBKU = false;
      bool YRtsUZHXxY = false;
      bool KUBGjMXBMw = false;
      bool IedupUfulo = false;
      bool dIAFpqkfhT = false;
      bool ohGdQmuOmj = false;
      bool TUnNLtmyDz = false;
      string DxbJSBPFVY;
      string VMXmxOcUym;
      string dFFdlZoSXh;
      string TlLhEXlZGo;
      string aCldWdjHPz;
      string xqKbNxryWr;
      string LxgnFiApiY;
      string cDNJMbTaHH;
      string phCYjcbjTG;
      string fmCOccUyYm;
      string csXqWtdBPk;
      string dQqOCduIDb;
      string HQWbsoAyfi;
      string GUBTdwJLBo;
      string yaUhcOwmMz;
      string hMtxeMWwxh;
      string seUfdIOWjF;
      string UjGNcgwnzd;
      string rMlgBtZEoR;
      string YNNTuTGnDU;
      if(DxbJSBPFVY == csXqWtdBPk){AYCGdyGfTx = true;}
      else if(csXqWtdBPk == DxbJSBPFVY){syEHrOigWy = true;}
      if(VMXmxOcUym == dQqOCduIDb){PhwTogrmej = true;}
      else if(dQqOCduIDb == VMXmxOcUym){RBSigBKKaO = true;}
      if(dFFdlZoSXh == HQWbsoAyfi){hJhSgGVmRJ = true;}
      else if(HQWbsoAyfi == dFFdlZoSXh){JfZCtwFECb = true;}
      if(TlLhEXlZGo == GUBTdwJLBo){aHkddSKfCC = true;}
      else if(GUBTdwJLBo == TlLhEXlZGo){IwUWxrWBKU = true;}
      if(aCldWdjHPz == yaUhcOwmMz){ntkqBpQtdO = true;}
      else if(yaUhcOwmMz == aCldWdjHPz){YRtsUZHXxY = true;}
      if(xqKbNxryWr == hMtxeMWwxh){aSJSlpqjYX = true;}
      else if(hMtxeMWwxh == xqKbNxryWr){KUBGjMXBMw = true;}
      if(LxgnFiApiY == seUfdIOWjF){cSNJWHxSpV = true;}
      else if(seUfdIOWjF == LxgnFiApiY){IedupUfulo = true;}
      if(cDNJMbTaHH == UjGNcgwnzd){VMjbwnYpQp = true;}
      if(phCYjcbjTG == rMlgBtZEoR){paIDiHyRrm = true;}
      if(fmCOccUyYm == YNNTuTGnDU){bZGeZUfbUs = true;}
      while(UjGNcgwnzd == cDNJMbTaHH){dIAFpqkfhT = true;}
      while(rMlgBtZEoR == rMlgBtZEoR){ohGdQmuOmj = true;}
      while(YNNTuTGnDU == YNNTuTGnDU){TUnNLtmyDz = true;}
      if(AYCGdyGfTx == true){AYCGdyGfTx = false;}
      if(PhwTogrmej == true){PhwTogrmej = false;}
      if(hJhSgGVmRJ == true){hJhSgGVmRJ = false;}
      if(aHkddSKfCC == true){aHkddSKfCC = false;}
      if(ntkqBpQtdO == true){ntkqBpQtdO = false;}
      if(aSJSlpqjYX == true){aSJSlpqjYX = false;}
      if(cSNJWHxSpV == true){cSNJWHxSpV = false;}
      if(VMjbwnYpQp == true){VMjbwnYpQp = false;}
      if(paIDiHyRrm == true){paIDiHyRrm = false;}
      if(bZGeZUfbUs == true){bZGeZUfbUs = false;}
      if(syEHrOigWy == true){syEHrOigWy = false;}
      if(RBSigBKKaO == true){RBSigBKKaO = false;}
      if(JfZCtwFECb == true){JfZCtwFECb = false;}
      if(IwUWxrWBKU == true){IwUWxrWBKU = false;}
      if(YRtsUZHXxY == true){YRtsUZHXxY = false;}
      if(KUBGjMXBMw == true){KUBGjMXBMw = false;}
      if(IedupUfulo == true){IedupUfulo = false;}
      if(dIAFpqkfhT == true){dIAFpqkfhT = false;}
      if(ohGdQmuOmj == true){ohGdQmuOmj = false;}
      if(TUnNLtmyDz == true){TUnNLtmyDz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YPYOMFSSXN
{ 
  void kkMFGzkqDm()
  { 
      bool CJkQqdUmBs = false;
      bool xsTTHnOJyd = false;
      bool gfyNMfOUql = false;
      bool XLyRwgihcO = false;
      bool UotECNcpkz = false;
      bool OmimgWLXEL = false;
      bool OAuOUktCXa = false;
      bool EgfiIqlsKy = false;
      bool DYjrWkbLjZ = false;
      bool VLdRATtfiQ = false;
      bool TaSKuhGzdo = false;
      bool jRMFIpQDri = false;
      bool JhRxEsDOin = false;
      bool xGJtdbFYxn = false;
      bool ebhtoEyXIJ = false;
      bool CqmzHkIUFB = false;
      bool eqQordHSaf = false;
      bool kqFDEgAWIA = false;
      bool ueFYkFSJls = false;
      bool TjTuJmLrBK = false;
      string bjuhcCzkUj;
      string ZeSAsGUeqk;
      string niIQiTgVuU;
      string mmBkUutAcM;
      string ictVukriuI;
      string wywoFrbezJ;
      string VdpAwldhfy;
      string MTCeBLnlVx;
      string ayalYnqLfb;
      string TPASqFltZI;
      string HKCxubwRms;
      string EoICiXCDbH;
      string tRBDdxSinw;
      string YtMMoPucqw;
      string qZYlXeGyjT;
      string DhRROpIzSy;
      string INdqUJbwMT;
      string bYMeQUsPfz;
      string qUyTspbuyz;
      string GwKrmdwgQO;
      if(bjuhcCzkUj == HKCxubwRms){CJkQqdUmBs = true;}
      else if(HKCxubwRms == bjuhcCzkUj){TaSKuhGzdo = true;}
      if(ZeSAsGUeqk == EoICiXCDbH){xsTTHnOJyd = true;}
      else if(EoICiXCDbH == ZeSAsGUeqk){jRMFIpQDri = true;}
      if(niIQiTgVuU == tRBDdxSinw){gfyNMfOUql = true;}
      else if(tRBDdxSinw == niIQiTgVuU){JhRxEsDOin = true;}
      if(mmBkUutAcM == YtMMoPucqw){XLyRwgihcO = true;}
      else if(YtMMoPucqw == mmBkUutAcM){xGJtdbFYxn = true;}
      if(ictVukriuI == qZYlXeGyjT){UotECNcpkz = true;}
      else if(qZYlXeGyjT == ictVukriuI){ebhtoEyXIJ = true;}
      if(wywoFrbezJ == DhRROpIzSy){OmimgWLXEL = true;}
      else if(DhRROpIzSy == wywoFrbezJ){CqmzHkIUFB = true;}
      if(VdpAwldhfy == INdqUJbwMT){OAuOUktCXa = true;}
      else if(INdqUJbwMT == VdpAwldhfy){eqQordHSaf = true;}
      if(MTCeBLnlVx == bYMeQUsPfz){EgfiIqlsKy = true;}
      if(ayalYnqLfb == qUyTspbuyz){DYjrWkbLjZ = true;}
      if(TPASqFltZI == GwKrmdwgQO){VLdRATtfiQ = true;}
      while(bYMeQUsPfz == MTCeBLnlVx){kqFDEgAWIA = true;}
      while(qUyTspbuyz == qUyTspbuyz){ueFYkFSJls = true;}
      while(GwKrmdwgQO == GwKrmdwgQO){TjTuJmLrBK = true;}
      if(CJkQqdUmBs == true){CJkQqdUmBs = false;}
      if(xsTTHnOJyd == true){xsTTHnOJyd = false;}
      if(gfyNMfOUql == true){gfyNMfOUql = false;}
      if(XLyRwgihcO == true){XLyRwgihcO = false;}
      if(UotECNcpkz == true){UotECNcpkz = false;}
      if(OmimgWLXEL == true){OmimgWLXEL = false;}
      if(OAuOUktCXa == true){OAuOUktCXa = false;}
      if(EgfiIqlsKy == true){EgfiIqlsKy = false;}
      if(DYjrWkbLjZ == true){DYjrWkbLjZ = false;}
      if(VLdRATtfiQ == true){VLdRATtfiQ = false;}
      if(TaSKuhGzdo == true){TaSKuhGzdo = false;}
      if(jRMFIpQDri == true){jRMFIpQDri = false;}
      if(JhRxEsDOin == true){JhRxEsDOin = false;}
      if(xGJtdbFYxn == true){xGJtdbFYxn = false;}
      if(ebhtoEyXIJ == true){ebhtoEyXIJ = false;}
      if(CqmzHkIUFB == true){CqmzHkIUFB = false;}
      if(eqQordHSaf == true){eqQordHSaf = false;}
      if(kqFDEgAWIA == true){kqFDEgAWIA = false;}
      if(ueFYkFSJls == true){ueFYkFSJls = false;}
      if(TjTuJmLrBK == true){TjTuJmLrBK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class AZOARLXDKS
{ 
  void JFtAxTDUje()
  { 
      bool rTHYqFRaTF = false;
      bool NWhfOGmDtP = false;
      bool wyckPVMkpS = false;
      bool EnSZPZYSKJ = false;
      bool XqZwjpezZM = false;
      bool juHEhspWJU = false;
      bool sgjBAYDIPn = false;
      bool hTsjqFXgAZ = false;
      bool qMKdiJgOrV = false;
      bool iBJoQqGYuK = false;
      bool lWildPBWMb = false;
      bool UJVidfVFSq = false;
      bool qoQtFfKESz = false;
      bool zzkpmUdYzS = false;
      bool HKBPAYfZKj = false;
      bool UBGESxFnfy = false;
      bool ExDNcVXfwJ = false;
      bool CxFzigmAtf = false;
      bool AkshdxsgWp = false;
      bool OorBNKGnmd = false;
      string yRYkXpOODQ;
      string FEMgDGSqMk;
      string ffTysgYEqX;
      string zIWqaeSTsa;
      string QkeQRlppFp;
      string IXUpgRxZrq;
      string RJXeRxhVGY;
      string myxlTtBCgG;
      string qizAPangKJ;
      string gsbCJHNzzT;
      string EUifDJZhVl;
      string PurkziKWPp;
      string JAKxrtWucM;
      string gBCMCWzSxF;
      string BFKYskApCa;
      string xqjIKDJlmI;
      string meFJFOrfic;
      string iqZJoEplEH;
      string tfrBPoRtCy;
      string NMeoSWzBTd;
      if(yRYkXpOODQ == EUifDJZhVl){rTHYqFRaTF = true;}
      else if(EUifDJZhVl == yRYkXpOODQ){lWildPBWMb = true;}
      if(FEMgDGSqMk == PurkziKWPp){NWhfOGmDtP = true;}
      else if(PurkziKWPp == FEMgDGSqMk){UJVidfVFSq = true;}
      if(ffTysgYEqX == JAKxrtWucM){wyckPVMkpS = true;}
      else if(JAKxrtWucM == ffTysgYEqX){qoQtFfKESz = true;}
      if(zIWqaeSTsa == gBCMCWzSxF){EnSZPZYSKJ = true;}
      else if(gBCMCWzSxF == zIWqaeSTsa){zzkpmUdYzS = true;}
      if(QkeQRlppFp == BFKYskApCa){XqZwjpezZM = true;}
      else if(BFKYskApCa == QkeQRlppFp){HKBPAYfZKj = true;}
      if(IXUpgRxZrq == xqjIKDJlmI){juHEhspWJU = true;}
      else if(xqjIKDJlmI == IXUpgRxZrq){UBGESxFnfy = true;}
      if(RJXeRxhVGY == meFJFOrfic){sgjBAYDIPn = true;}
      else if(meFJFOrfic == RJXeRxhVGY){ExDNcVXfwJ = true;}
      if(myxlTtBCgG == iqZJoEplEH){hTsjqFXgAZ = true;}
      if(qizAPangKJ == tfrBPoRtCy){qMKdiJgOrV = true;}
      if(gsbCJHNzzT == NMeoSWzBTd){iBJoQqGYuK = true;}
      while(iqZJoEplEH == myxlTtBCgG){CxFzigmAtf = true;}
      while(tfrBPoRtCy == tfrBPoRtCy){AkshdxsgWp = true;}
      while(NMeoSWzBTd == NMeoSWzBTd){OorBNKGnmd = true;}
      if(rTHYqFRaTF == true){rTHYqFRaTF = false;}
      if(NWhfOGmDtP == true){NWhfOGmDtP = false;}
      if(wyckPVMkpS == true){wyckPVMkpS = false;}
      if(EnSZPZYSKJ == true){EnSZPZYSKJ = false;}
      if(XqZwjpezZM == true){XqZwjpezZM = false;}
      if(juHEhspWJU == true){juHEhspWJU = false;}
      if(sgjBAYDIPn == true){sgjBAYDIPn = false;}
      if(hTsjqFXgAZ == true){hTsjqFXgAZ = false;}
      if(qMKdiJgOrV == true){qMKdiJgOrV = false;}
      if(iBJoQqGYuK == true){iBJoQqGYuK = false;}
      if(lWildPBWMb == true){lWildPBWMb = false;}
      if(UJVidfVFSq == true){UJVidfVFSq = false;}
      if(qoQtFfKESz == true){qoQtFfKESz = false;}
      if(zzkpmUdYzS == true){zzkpmUdYzS = false;}
      if(HKBPAYfZKj == true){HKBPAYfZKj = false;}
      if(UBGESxFnfy == true){UBGESxFnfy = false;}
      if(ExDNcVXfwJ == true){ExDNcVXfwJ = false;}
      if(CxFzigmAtf == true){CxFzigmAtf = false;}
      if(AkshdxsgWp == true){AkshdxsgWp = false;}
      if(OorBNKGnmd == true){OorBNKGnmd = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SGXEKLJKKJ
{ 
  void ETpgCPezgx()
  { 
      bool MIKdoRZMJL = false;
      bool otNqxqQQVe = false;
      bool nTrzrbrVEp = false;
      bool UYiOhVhjOT = false;
      bool fTWnMrZpKm = false;
      bool AosnSrQKkN = false;
      bool ZDISpPLyIa = false;
      bool JfBBzUxDlA = false;
      bool CBLRpfftwp = false;
      bool EWyBtcKDxx = false;
      bool gagicTmqox = false;
      bool uNYkKXqpcK = false;
      bool iQCSWlhzHW = false;
      bool JsGnTbUbnV = false;
      bool roFyuqyMqW = false;
      bool kUamNPXOMq = false;
      bool knjSmBGjRi = false;
      bool COCGRtLDXe = false;
      bool nUIjkoRTaD = false;
      bool DGPxkEElLs = false;
      string oCYfIwkLqr;
      string pbWUAstcdH;
      string tTkUcBIcHp;
      string tiIQweiwfe;
      string rUuStILFPR;
      string mxdASVRECG;
      string nHDByGTFXV;
      string KywzOZXRZU;
      string pDNpDwFIxZ;
      string NesYpkQtTn;
      string oFgtHnBeks;
      string yzdgPksNyC;
      string BLHxIeEWJd;
      string IcylfkVTiO;
      string uaWEbNALJE;
      string qDwRldyilb;
      string jdUHjUBqBe;
      string OZdiHanDVt;
      string erNElFfgzC;
      string mmhHeQFOTG;
      if(oCYfIwkLqr == oFgtHnBeks){MIKdoRZMJL = true;}
      else if(oFgtHnBeks == oCYfIwkLqr){gagicTmqox = true;}
      if(pbWUAstcdH == yzdgPksNyC){otNqxqQQVe = true;}
      else if(yzdgPksNyC == pbWUAstcdH){uNYkKXqpcK = true;}
      if(tTkUcBIcHp == BLHxIeEWJd){nTrzrbrVEp = true;}
      else if(BLHxIeEWJd == tTkUcBIcHp){iQCSWlhzHW = true;}
      if(tiIQweiwfe == IcylfkVTiO){UYiOhVhjOT = true;}
      else if(IcylfkVTiO == tiIQweiwfe){JsGnTbUbnV = true;}
      if(rUuStILFPR == uaWEbNALJE){fTWnMrZpKm = true;}
      else if(uaWEbNALJE == rUuStILFPR){roFyuqyMqW = true;}
      if(mxdASVRECG == qDwRldyilb){AosnSrQKkN = true;}
      else if(qDwRldyilb == mxdASVRECG){kUamNPXOMq = true;}
      if(nHDByGTFXV == jdUHjUBqBe){ZDISpPLyIa = true;}
      else if(jdUHjUBqBe == nHDByGTFXV){knjSmBGjRi = true;}
      if(KywzOZXRZU == OZdiHanDVt){JfBBzUxDlA = true;}
      if(pDNpDwFIxZ == erNElFfgzC){CBLRpfftwp = true;}
      if(NesYpkQtTn == mmhHeQFOTG){EWyBtcKDxx = true;}
      while(OZdiHanDVt == KywzOZXRZU){COCGRtLDXe = true;}
      while(erNElFfgzC == erNElFfgzC){nUIjkoRTaD = true;}
      while(mmhHeQFOTG == mmhHeQFOTG){DGPxkEElLs = true;}
      if(MIKdoRZMJL == true){MIKdoRZMJL = false;}
      if(otNqxqQQVe == true){otNqxqQQVe = false;}
      if(nTrzrbrVEp == true){nTrzrbrVEp = false;}
      if(UYiOhVhjOT == true){UYiOhVhjOT = false;}
      if(fTWnMrZpKm == true){fTWnMrZpKm = false;}
      if(AosnSrQKkN == true){AosnSrQKkN = false;}
      if(ZDISpPLyIa == true){ZDISpPLyIa = false;}
      if(JfBBzUxDlA == true){JfBBzUxDlA = false;}
      if(CBLRpfftwp == true){CBLRpfftwp = false;}
      if(EWyBtcKDxx == true){EWyBtcKDxx = false;}
      if(gagicTmqox == true){gagicTmqox = false;}
      if(uNYkKXqpcK == true){uNYkKXqpcK = false;}
      if(iQCSWlhzHW == true){iQCSWlhzHW = false;}
      if(JsGnTbUbnV == true){JsGnTbUbnV = false;}
      if(roFyuqyMqW == true){roFyuqyMqW = false;}
      if(kUamNPXOMq == true){kUamNPXOMq = false;}
      if(knjSmBGjRi == true){knjSmBGjRi = false;}
      if(COCGRtLDXe == true){COCGRtLDXe = false;}
      if(nUIjkoRTaD == true){nUIjkoRTaD = false;}
      if(DGPxkEElLs == true){DGPxkEElLs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XCUKDXZLKX
{ 
  void ZEHimaKuII()
  { 
      bool fEzInUZICl = false;
      bool RErsrtcFTJ = false;
      bool zHBKdlxlhF = false;
      bool UpWmYUmTOL = false;
      bool RLhiFCmMiy = false;
      bool lEpjEYDodC = false;
      bool WnZwQbgFTW = false;
      bool HJqHqmYREa = false;
      bool ykCjTQAlhA = false;
      bool RyoLndklSd = false;
      bool VhsMqcNhsQ = false;
      bool zkjDDllocz = false;
      bool atWSEITeTN = false;
      bool BlzaFQrorq = false;
      bool DSsQAfNlHB = false;
      bool quISCaKZHh = false;
      bool IhNeOMlJxm = false;
      bool twxlwIBGRX = false;
      bool tgXRVGcgXe = false;
      bool FRgKhVEOBm = false;
      string PdKHyAqdib;
      string chULngWBRG;
      string kELzRcMEMF;
      string tuIgEnwuaY;
      string TLPsxATCZC;
      string cAKsFLRLoV;
      string nlRdhhYmGz;
      string jaasmYoSCQ;
      string IJeKbsjPas;
      string WjGFjgtkKO;
      string agwaRCZRFf;
      string WzJXMeHGBn;
      string xJLNcAUUIF;
      string eDHmobNEsZ;
      string TSJuWXahsF;
      string YmjWRRUhyR;
      string gEIABSrWhk;
      string QDNaUezQpE;
      string YmbDgqTjVX;
      string GeFbsOJJtD;
      if(PdKHyAqdib == agwaRCZRFf){fEzInUZICl = true;}
      else if(agwaRCZRFf == PdKHyAqdib){VhsMqcNhsQ = true;}
      if(chULngWBRG == WzJXMeHGBn){RErsrtcFTJ = true;}
      else if(WzJXMeHGBn == chULngWBRG){zkjDDllocz = true;}
      if(kELzRcMEMF == xJLNcAUUIF){zHBKdlxlhF = true;}
      else if(xJLNcAUUIF == kELzRcMEMF){atWSEITeTN = true;}
      if(tuIgEnwuaY == eDHmobNEsZ){UpWmYUmTOL = true;}
      else if(eDHmobNEsZ == tuIgEnwuaY){BlzaFQrorq = true;}
      if(TLPsxATCZC == TSJuWXahsF){RLhiFCmMiy = true;}
      else if(TSJuWXahsF == TLPsxATCZC){DSsQAfNlHB = true;}
      if(cAKsFLRLoV == YmjWRRUhyR){lEpjEYDodC = true;}
      else if(YmjWRRUhyR == cAKsFLRLoV){quISCaKZHh = true;}
      if(nlRdhhYmGz == gEIABSrWhk){WnZwQbgFTW = true;}
      else if(gEIABSrWhk == nlRdhhYmGz){IhNeOMlJxm = true;}
      if(jaasmYoSCQ == QDNaUezQpE){HJqHqmYREa = true;}
      if(IJeKbsjPas == YmbDgqTjVX){ykCjTQAlhA = true;}
      if(WjGFjgtkKO == GeFbsOJJtD){RyoLndklSd = true;}
      while(QDNaUezQpE == jaasmYoSCQ){twxlwIBGRX = true;}
      while(YmbDgqTjVX == YmbDgqTjVX){tgXRVGcgXe = true;}
      while(GeFbsOJJtD == GeFbsOJJtD){FRgKhVEOBm = true;}
      if(fEzInUZICl == true){fEzInUZICl = false;}
      if(RErsrtcFTJ == true){RErsrtcFTJ = false;}
      if(zHBKdlxlhF == true){zHBKdlxlhF = false;}
      if(UpWmYUmTOL == true){UpWmYUmTOL = false;}
      if(RLhiFCmMiy == true){RLhiFCmMiy = false;}
      if(lEpjEYDodC == true){lEpjEYDodC = false;}
      if(WnZwQbgFTW == true){WnZwQbgFTW = false;}
      if(HJqHqmYREa == true){HJqHqmYREa = false;}
      if(ykCjTQAlhA == true){ykCjTQAlhA = false;}
      if(RyoLndklSd == true){RyoLndklSd = false;}
      if(VhsMqcNhsQ == true){VhsMqcNhsQ = false;}
      if(zkjDDllocz == true){zkjDDllocz = false;}
      if(atWSEITeTN == true){atWSEITeTN = false;}
      if(BlzaFQrorq == true){BlzaFQrorq = false;}
      if(DSsQAfNlHB == true){DSsQAfNlHB = false;}
      if(quISCaKZHh == true){quISCaKZHh = false;}
      if(IhNeOMlJxm == true){IhNeOMlJxm = false;}
      if(twxlwIBGRX == true){twxlwIBGRX = false;}
      if(tgXRVGcgXe == true){tgXRVGcgXe = false;}
      if(FRgKhVEOBm == true){FRgKhVEOBm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JCUYXBAFFB
{ 
  void KJdRwZdjZh()
  { 
      bool TzHKEGDVJJ = false;
      bool rVbnyYucXJ = false;
      bool sqDqMZqMrk = false;
      bool tjaWlaBJFr = false;
      bool DbqzVpLhjy = false;
      bool alsTScuQxe = false;
      bool OpcXPIuEBX = false;
      bool WlWeGQzEVW = false;
      bool NJctJHpGEJ = false;
      bool fkLZdMzSjM = false;
      bool tGfMaoPONB = false;
      bool cptZRFNhNH = false;
      bool SnArrkZwBs = false;
      bool RgUIcGcQSU = false;
      bool rbsCgoplIX = false;
      bool zQEbtFGnUp = false;
      bool OSNTiuVlxK = false;
      bool cwPsPpuxKC = false;
      bool tnnctBcrNW = false;
      bool znwcGYOYSk = false;
      string nhYKpYrnmX;
      string dUVPzjfbsl;
      string mXraCMPfQK;
      string XtxCwLyial;
      string ObiMRIwRAx;
      string BSXPiQpyHd;
      string JLXczjXTSn;
      string lxAGWLofJs;
      string QxeSIEVgSe;
      string VAaxcCEFMg;
      string kbzMKESRrX;
      string aKnNNLgETt;
      string OpAmRlVDcU;
      string GiSbInyRIE;
      string lEouELLMjR;
      string VzkRStbLuY;
      string hPnWQqqMFY;
      string MOwPIJBFiG;
      string ZJTKzGyAIO;
      string MmWiWWPRdM;
      if(nhYKpYrnmX == kbzMKESRrX){TzHKEGDVJJ = true;}
      else if(kbzMKESRrX == nhYKpYrnmX){tGfMaoPONB = true;}
      if(dUVPzjfbsl == aKnNNLgETt){rVbnyYucXJ = true;}
      else if(aKnNNLgETt == dUVPzjfbsl){cptZRFNhNH = true;}
      if(mXraCMPfQK == OpAmRlVDcU){sqDqMZqMrk = true;}
      else if(OpAmRlVDcU == mXraCMPfQK){SnArrkZwBs = true;}
      if(XtxCwLyial == GiSbInyRIE){tjaWlaBJFr = true;}
      else if(GiSbInyRIE == XtxCwLyial){RgUIcGcQSU = true;}
      if(ObiMRIwRAx == lEouELLMjR){DbqzVpLhjy = true;}
      else if(lEouELLMjR == ObiMRIwRAx){rbsCgoplIX = true;}
      if(BSXPiQpyHd == VzkRStbLuY){alsTScuQxe = true;}
      else if(VzkRStbLuY == BSXPiQpyHd){zQEbtFGnUp = true;}
      if(JLXczjXTSn == hPnWQqqMFY){OpcXPIuEBX = true;}
      else if(hPnWQqqMFY == JLXczjXTSn){OSNTiuVlxK = true;}
      if(lxAGWLofJs == MOwPIJBFiG){WlWeGQzEVW = true;}
      if(QxeSIEVgSe == ZJTKzGyAIO){NJctJHpGEJ = true;}
      if(VAaxcCEFMg == MmWiWWPRdM){fkLZdMzSjM = true;}
      while(MOwPIJBFiG == lxAGWLofJs){cwPsPpuxKC = true;}
      while(ZJTKzGyAIO == ZJTKzGyAIO){tnnctBcrNW = true;}
      while(MmWiWWPRdM == MmWiWWPRdM){znwcGYOYSk = true;}
      if(TzHKEGDVJJ == true){TzHKEGDVJJ = false;}
      if(rVbnyYucXJ == true){rVbnyYucXJ = false;}
      if(sqDqMZqMrk == true){sqDqMZqMrk = false;}
      if(tjaWlaBJFr == true){tjaWlaBJFr = false;}
      if(DbqzVpLhjy == true){DbqzVpLhjy = false;}
      if(alsTScuQxe == true){alsTScuQxe = false;}
      if(OpcXPIuEBX == true){OpcXPIuEBX = false;}
      if(WlWeGQzEVW == true){WlWeGQzEVW = false;}
      if(NJctJHpGEJ == true){NJctJHpGEJ = false;}
      if(fkLZdMzSjM == true){fkLZdMzSjM = false;}
      if(tGfMaoPONB == true){tGfMaoPONB = false;}
      if(cptZRFNhNH == true){cptZRFNhNH = false;}
      if(SnArrkZwBs == true){SnArrkZwBs = false;}
      if(RgUIcGcQSU == true){RgUIcGcQSU = false;}
      if(rbsCgoplIX == true){rbsCgoplIX = false;}
      if(zQEbtFGnUp == true){zQEbtFGnUp = false;}
      if(OSNTiuVlxK == true){OSNTiuVlxK = false;}
      if(cwPsPpuxKC == true){cwPsPpuxKC = false;}
      if(tnnctBcrNW == true){tnnctBcrNW = false;}
      if(znwcGYOYSk == true){znwcGYOYSk = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FFYKWXULKB
{ 
  void AzpUBeQPhF()
  { 
      bool PcaAOeZgdR = false;
      bool hHtYKdtgsF = false;
      bool yInixIdBxs = false;
      bool CknkflbyMt = false;
      bool MgwKjNotFu = false;
      bool zsycwGnqDF = false;
      bool BYjjPxTJsX = false;
      bool CuUysRLdyi = false;
      bool TjuBtJlKnY = false;
      bool yjuGsmxMum = false;
      bool sfAgsDYzje = false;
      bool tKsTlRdhoF = false;
      bool LJUUZRANFF = false;
      bool TzhLoniNIq = false;
      bool klZJdmqaTb = false;
      bool JOWobjKfHa = false;
      bool DKtdKtbgdy = false;
      bool hPYyUYjLpl = false;
      bool ZYsdwEcriF = false;
      bool cDGgmZruyi = false;
      string LBmAutftPi;
      string uiprDAFnkV;
      string NfsTrzPoJT;
      string MKQwRhEQip;
      string EjeLoQWokg;
      string APlkzeITHw;
      string dpjwVjStZU;
      string unEtQcAgdb;
      string cZEbSxYewl;
      string ZkIlLOzhoF;
      string xbOPkLKqUV;
      string eapfTElHdY;
      string KmxntKeRiC;
      string dSqXdALnqG;
      string lhKaHzucSH;
      string bqEQtKeaUa;
      string gsHSrMiRCS;
      string sxQWmcbWfu;
      string PCyaZlNacS;
      string VjTPZsZqiu;
      if(LBmAutftPi == xbOPkLKqUV){PcaAOeZgdR = true;}
      else if(xbOPkLKqUV == LBmAutftPi){sfAgsDYzje = true;}
      if(uiprDAFnkV == eapfTElHdY){hHtYKdtgsF = true;}
      else if(eapfTElHdY == uiprDAFnkV){tKsTlRdhoF = true;}
      if(NfsTrzPoJT == KmxntKeRiC){yInixIdBxs = true;}
      else if(KmxntKeRiC == NfsTrzPoJT){LJUUZRANFF = true;}
      if(MKQwRhEQip == dSqXdALnqG){CknkflbyMt = true;}
      else if(dSqXdALnqG == MKQwRhEQip){TzhLoniNIq = true;}
      if(EjeLoQWokg == lhKaHzucSH){MgwKjNotFu = true;}
      else if(lhKaHzucSH == EjeLoQWokg){klZJdmqaTb = true;}
      if(APlkzeITHw == bqEQtKeaUa){zsycwGnqDF = true;}
      else if(bqEQtKeaUa == APlkzeITHw){JOWobjKfHa = true;}
      if(dpjwVjStZU == gsHSrMiRCS){BYjjPxTJsX = true;}
      else if(gsHSrMiRCS == dpjwVjStZU){DKtdKtbgdy = true;}
      if(unEtQcAgdb == sxQWmcbWfu){CuUysRLdyi = true;}
      if(cZEbSxYewl == PCyaZlNacS){TjuBtJlKnY = true;}
      if(ZkIlLOzhoF == VjTPZsZqiu){yjuGsmxMum = true;}
      while(sxQWmcbWfu == unEtQcAgdb){hPYyUYjLpl = true;}
      while(PCyaZlNacS == PCyaZlNacS){ZYsdwEcriF = true;}
      while(VjTPZsZqiu == VjTPZsZqiu){cDGgmZruyi = true;}
      if(PcaAOeZgdR == true){PcaAOeZgdR = false;}
      if(hHtYKdtgsF == true){hHtYKdtgsF = false;}
      if(yInixIdBxs == true){yInixIdBxs = false;}
      if(CknkflbyMt == true){CknkflbyMt = false;}
      if(MgwKjNotFu == true){MgwKjNotFu = false;}
      if(zsycwGnqDF == true){zsycwGnqDF = false;}
      if(BYjjPxTJsX == true){BYjjPxTJsX = false;}
      if(CuUysRLdyi == true){CuUysRLdyi = false;}
      if(TjuBtJlKnY == true){TjuBtJlKnY = false;}
      if(yjuGsmxMum == true){yjuGsmxMum = false;}
      if(sfAgsDYzje == true){sfAgsDYzje = false;}
      if(tKsTlRdhoF == true){tKsTlRdhoF = false;}
      if(LJUUZRANFF == true){LJUUZRANFF = false;}
      if(TzhLoniNIq == true){TzhLoniNIq = false;}
      if(klZJdmqaTb == true){klZJdmqaTb = false;}
      if(JOWobjKfHa == true){JOWobjKfHa = false;}
      if(DKtdKtbgdy == true){DKtdKtbgdy = false;}
      if(hPYyUYjLpl == true){hPYyUYjLpl = false;}
      if(ZYsdwEcriF == true){ZYsdwEcriF = false;}
      if(cDGgmZruyi == true){cDGgmZruyi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class IBCIVYCJCS
{ 
  void VMwrYUiPxS()
  { 
      bool FtytZDbYgU = false;
      bool mnAVkuMgGM = false;
      bool tnOsLQeqxY = false;
      bool lRxULwSqaB = false;
      bool braclocxJU = false;
      bool TWlgVRfFiI = false;
      bool XkaIiVpfKn = false;
      bool cUGFjBMkGV = false;
      bool mghbEOfQyt = false;
      bool PMIRcqRSsQ = false;
      bool zxmUryQgwT = false;
      bool EuzNVpDyDT = false;
      bool oYgQYPMnbs = false;
      bool jtqoWptMjG = false;
      bool yzUdoUNuwm = false;
      bool nSPZaZPWwb = false;
      bool lbECjJDJpe = false;
      bool wpOxVpECaD = false;
      bool CRroFQeROx = false;
      bool ZWPHTVtRte = false;
      string egNVDxpzeH;
      string LxMBGPgTGn;
      string MeZyqRXYuN;
      string WtIQsxpnRF;
      string YYuIDDbINz;
      string nVFiWzGPel;
      string ZQDQkQouII;
      string YrBBPniQtm;
      string oPLwYAoYts;
      string paJfeHVeWl;
      string OgAEZuUDTi;
      string QoUqwKFHqk;
      string bPBORyiVVH;
      string REtIYIPGVc;
      string fkEKcBGRiU;
      string CNmlKyekmb;
      string MzhLQMuLSn;
      string tykOFpKqkQ;
      string qQlAFERHxZ;
      string cdTtQfDssQ;
      if(egNVDxpzeH == OgAEZuUDTi){FtytZDbYgU = true;}
      else if(OgAEZuUDTi == egNVDxpzeH){zxmUryQgwT = true;}
      if(LxMBGPgTGn == QoUqwKFHqk){mnAVkuMgGM = true;}
      else if(QoUqwKFHqk == LxMBGPgTGn){EuzNVpDyDT = true;}
      if(MeZyqRXYuN == bPBORyiVVH){tnOsLQeqxY = true;}
      else if(bPBORyiVVH == MeZyqRXYuN){oYgQYPMnbs = true;}
      if(WtIQsxpnRF == REtIYIPGVc){lRxULwSqaB = true;}
      else if(REtIYIPGVc == WtIQsxpnRF){jtqoWptMjG = true;}
      if(YYuIDDbINz == fkEKcBGRiU){braclocxJU = true;}
      else if(fkEKcBGRiU == YYuIDDbINz){yzUdoUNuwm = true;}
      if(nVFiWzGPel == CNmlKyekmb){TWlgVRfFiI = true;}
      else if(CNmlKyekmb == nVFiWzGPel){nSPZaZPWwb = true;}
      if(ZQDQkQouII == MzhLQMuLSn){XkaIiVpfKn = true;}
      else if(MzhLQMuLSn == ZQDQkQouII){lbECjJDJpe = true;}
      if(YrBBPniQtm == tykOFpKqkQ){cUGFjBMkGV = true;}
      if(oPLwYAoYts == qQlAFERHxZ){mghbEOfQyt = true;}
      if(paJfeHVeWl == cdTtQfDssQ){PMIRcqRSsQ = true;}
      while(tykOFpKqkQ == YrBBPniQtm){wpOxVpECaD = true;}
      while(qQlAFERHxZ == qQlAFERHxZ){CRroFQeROx = true;}
      while(cdTtQfDssQ == cdTtQfDssQ){ZWPHTVtRte = true;}
      if(FtytZDbYgU == true){FtytZDbYgU = false;}
      if(mnAVkuMgGM == true){mnAVkuMgGM = false;}
      if(tnOsLQeqxY == true){tnOsLQeqxY = false;}
      if(lRxULwSqaB == true){lRxULwSqaB = false;}
      if(braclocxJU == true){braclocxJU = false;}
      if(TWlgVRfFiI == true){TWlgVRfFiI = false;}
      if(XkaIiVpfKn == true){XkaIiVpfKn = false;}
      if(cUGFjBMkGV == true){cUGFjBMkGV = false;}
      if(mghbEOfQyt == true){mghbEOfQyt = false;}
      if(PMIRcqRSsQ == true){PMIRcqRSsQ = false;}
      if(zxmUryQgwT == true){zxmUryQgwT = false;}
      if(EuzNVpDyDT == true){EuzNVpDyDT = false;}
      if(oYgQYPMnbs == true){oYgQYPMnbs = false;}
      if(jtqoWptMjG == true){jtqoWptMjG = false;}
      if(yzUdoUNuwm == true){yzUdoUNuwm = false;}
      if(nSPZaZPWwb == true){nSPZaZPWwb = false;}
      if(lbECjJDJpe == true){lbECjJDJpe = false;}
      if(wpOxVpECaD == true){wpOxVpECaD = false;}
      if(CRroFQeROx == true){CRroFQeROx = false;}
      if(ZWPHTVtRte == true){ZWPHTVtRte = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NMKASMVXWR
{ 
  void baMOfpiVns()
  { 
      bool LlwFhtsNVF = false;
      bool XPDdoipNDe = false;
      bool AxCJOBVsbT = false;
      bool gDBzKWCxIk = false;
      bool AXRSpcrPHw = false;
      bool EnHkVOjmpg = false;
      bool YzlBXTNDTS = false;
      bool frERdbfjSl = false;
      bool IFlfOswPdu = false;
      bool lPGbLBlPtW = false;
      bool OounPMjoHs = false;
      bool iATEXSMFle = false;
      bool WyYXfPlbFc = false;
      bool pPowWTKxPC = false;
      bool JGIkMlwBEs = false;
      bool nfdOzQPXIz = false;
      bool aHidDSKWgD = false;
      bool IOpiVPonKt = false;
      bool TjXMoJlawF = false;
      bool EmoPbBXehs = false;
      string WQZtDzlTNZ;
      string IjpaxFXLQG;
      string ccytYEHKNb;
      string YjFruXgMwr;
      string ucdnCbmitu;
      string MZAjBsSeVz;
      string raFlBYeyeZ;
      string YKOXKzzJqx;
      string QYQSPziUpB;
      string FBqxAUWrld;
      string woDUWBKcZB;
      string sInXiTKmVC;
      string oJHdTWSRXp;
      string cHXMAaVuwN;
      string nGuoHxNTNf;
      string JiVjnmhYBz;
      string hsEiXlkaWH;
      string yPpHOiORVW;
      string JzDqfslmXN;
      string swtxetBLQV;
      if(WQZtDzlTNZ == woDUWBKcZB){LlwFhtsNVF = true;}
      else if(woDUWBKcZB == WQZtDzlTNZ){OounPMjoHs = true;}
      if(IjpaxFXLQG == sInXiTKmVC){XPDdoipNDe = true;}
      else if(sInXiTKmVC == IjpaxFXLQG){iATEXSMFle = true;}
      if(ccytYEHKNb == oJHdTWSRXp){AxCJOBVsbT = true;}
      else if(oJHdTWSRXp == ccytYEHKNb){WyYXfPlbFc = true;}
      if(YjFruXgMwr == cHXMAaVuwN){gDBzKWCxIk = true;}
      else if(cHXMAaVuwN == YjFruXgMwr){pPowWTKxPC = true;}
      if(ucdnCbmitu == nGuoHxNTNf){AXRSpcrPHw = true;}
      else if(nGuoHxNTNf == ucdnCbmitu){JGIkMlwBEs = true;}
      if(MZAjBsSeVz == JiVjnmhYBz){EnHkVOjmpg = true;}
      else if(JiVjnmhYBz == MZAjBsSeVz){nfdOzQPXIz = true;}
      if(raFlBYeyeZ == hsEiXlkaWH){YzlBXTNDTS = true;}
      else if(hsEiXlkaWH == raFlBYeyeZ){aHidDSKWgD = true;}
      if(YKOXKzzJqx == yPpHOiORVW){frERdbfjSl = true;}
      if(QYQSPziUpB == JzDqfslmXN){IFlfOswPdu = true;}
      if(FBqxAUWrld == swtxetBLQV){lPGbLBlPtW = true;}
      while(yPpHOiORVW == YKOXKzzJqx){IOpiVPonKt = true;}
      while(JzDqfslmXN == JzDqfslmXN){TjXMoJlawF = true;}
      while(swtxetBLQV == swtxetBLQV){EmoPbBXehs = true;}
      if(LlwFhtsNVF == true){LlwFhtsNVF = false;}
      if(XPDdoipNDe == true){XPDdoipNDe = false;}
      if(AxCJOBVsbT == true){AxCJOBVsbT = false;}
      if(gDBzKWCxIk == true){gDBzKWCxIk = false;}
      if(AXRSpcrPHw == true){AXRSpcrPHw = false;}
      if(EnHkVOjmpg == true){EnHkVOjmpg = false;}
      if(YzlBXTNDTS == true){YzlBXTNDTS = false;}
      if(frERdbfjSl == true){frERdbfjSl = false;}
      if(IFlfOswPdu == true){IFlfOswPdu = false;}
      if(lPGbLBlPtW == true){lPGbLBlPtW = false;}
      if(OounPMjoHs == true){OounPMjoHs = false;}
      if(iATEXSMFle == true){iATEXSMFle = false;}
      if(WyYXfPlbFc == true){WyYXfPlbFc = false;}
      if(pPowWTKxPC == true){pPowWTKxPC = false;}
      if(JGIkMlwBEs == true){JGIkMlwBEs = false;}
      if(nfdOzQPXIz == true){nfdOzQPXIz = false;}
      if(aHidDSKWgD == true){aHidDSKWgD = false;}
      if(IOpiVPonKt == true){IOpiVPonKt = false;}
      if(TjXMoJlawF == true){TjXMoJlawF = false;}
      if(EmoPbBXehs == true){EmoPbBXehs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SYASHHJOKI
{ 
  void NILQFPhbob()
  { 
      bool SDuQlEuGkl = false;
      bool WYpMjPKZLl = false;
      bool IBkYuiNhTw = false;
      bool NwrNeHDrIb = false;
      bool Iarajzlchc = false;
      bool aXeADRRWUC = false;
      bool pNmoYZfcgb = false;
      bool MRRjcgIKow = false;
      bool xjApVpRrJN = false;
      bool pgWFkunDCJ = false;
      bool SIuFQIFtWW = false;
      bool IGgYEFNxTi = false;
      bool HmTTleuLyh = false;
      bool sFIecontYM = false;
      bool jWVlEizxAe = false;
      bool hqjSBeoaTk = false;
      bool NtuodCyCwP = false;
      bool QSXGzkRgPD = false;
      bool fxLTzCVocy = false;
      bool zNZnzoagIC = false;
      string cjDEJmhoyC;
      string peqzTZIqgR;
      string DuHMMYzaEg;
      string BjtRbABNRn;
      string RtFGdQkPWg;
      string jkGezgcpqj;
      string mGjRBFkZWS;
      string YuZszJoBhw;
      string njDWwPdOSt;
      string XKVgALJnEX;
      string fLrEInLMfp;
      string raIuwDDNUl;
      string bVHctNZrrn;
      string zBwJPcqzNH;
      string EQibCxCajx;
      string HuSSwJEPJu;
      string OSTrppdyeT;
      string LDAMhKmeZx;
      string LzItfEhcMG;
      string YboRskxplM;
      if(cjDEJmhoyC == fLrEInLMfp){SDuQlEuGkl = true;}
      else if(fLrEInLMfp == cjDEJmhoyC){SIuFQIFtWW = true;}
      if(peqzTZIqgR == raIuwDDNUl){WYpMjPKZLl = true;}
      else if(raIuwDDNUl == peqzTZIqgR){IGgYEFNxTi = true;}
      if(DuHMMYzaEg == bVHctNZrrn){IBkYuiNhTw = true;}
      else if(bVHctNZrrn == DuHMMYzaEg){HmTTleuLyh = true;}
      if(BjtRbABNRn == zBwJPcqzNH){NwrNeHDrIb = true;}
      else if(zBwJPcqzNH == BjtRbABNRn){sFIecontYM = true;}
      if(RtFGdQkPWg == EQibCxCajx){Iarajzlchc = true;}
      else if(EQibCxCajx == RtFGdQkPWg){jWVlEizxAe = true;}
      if(jkGezgcpqj == HuSSwJEPJu){aXeADRRWUC = true;}
      else if(HuSSwJEPJu == jkGezgcpqj){hqjSBeoaTk = true;}
      if(mGjRBFkZWS == OSTrppdyeT){pNmoYZfcgb = true;}
      else if(OSTrppdyeT == mGjRBFkZWS){NtuodCyCwP = true;}
      if(YuZszJoBhw == LDAMhKmeZx){MRRjcgIKow = true;}
      if(njDWwPdOSt == LzItfEhcMG){xjApVpRrJN = true;}
      if(XKVgALJnEX == YboRskxplM){pgWFkunDCJ = true;}
      while(LDAMhKmeZx == YuZszJoBhw){QSXGzkRgPD = true;}
      while(LzItfEhcMG == LzItfEhcMG){fxLTzCVocy = true;}
      while(YboRskxplM == YboRskxplM){zNZnzoagIC = true;}
      if(SDuQlEuGkl == true){SDuQlEuGkl = false;}
      if(WYpMjPKZLl == true){WYpMjPKZLl = false;}
      if(IBkYuiNhTw == true){IBkYuiNhTw = false;}
      if(NwrNeHDrIb == true){NwrNeHDrIb = false;}
      if(Iarajzlchc == true){Iarajzlchc = false;}
      if(aXeADRRWUC == true){aXeADRRWUC = false;}
      if(pNmoYZfcgb == true){pNmoYZfcgb = false;}
      if(MRRjcgIKow == true){MRRjcgIKow = false;}
      if(xjApVpRrJN == true){xjApVpRrJN = false;}
      if(pgWFkunDCJ == true){pgWFkunDCJ = false;}
      if(SIuFQIFtWW == true){SIuFQIFtWW = false;}
      if(IGgYEFNxTi == true){IGgYEFNxTi = false;}
      if(HmTTleuLyh == true){HmTTleuLyh = false;}
      if(sFIecontYM == true){sFIecontYM = false;}
      if(jWVlEizxAe == true){jWVlEizxAe = false;}
      if(hqjSBeoaTk == true){hqjSBeoaTk = false;}
      if(NtuodCyCwP == true){NtuodCyCwP = false;}
      if(QSXGzkRgPD == true){QSXGzkRgPD = false;}
      if(fxLTzCVocy == true){fxLTzCVocy = false;}
      if(zNZnzoagIC == true){zNZnzoagIC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TMOHSFBZSQ
{ 
  void gUTZQUsXIU()
  { 
      bool HdxhUNIrlW = false;
      bool LXqTfcJTsy = false;
      bool GjIfDzBrWn = false;
      bool hZCtbGNZEI = false;
      bool EoZEQkeuMZ = false;
      bool tpxKrXVRdV = false;
      bool LQVynkVwqZ = false;
      bool PiPksRKDOS = false;
      bool TFfwsqcqCE = false;
      bool juwhwmSaPf = false;
      bool PhElKKaLQa = false;
      bool NRCKCGjZEx = false;
      bool HdtmoTmgoG = false;
      bool ziniDkBskh = false;
      bool PQZapRskfp = false;
      bool DrWSXknQxq = false;
      bool XtWKbedTHR = false;
      bool EqoWgpDSwX = false;
      bool aCByCGMVKR = false;
      bool wkutKVtOzL = false;
      string SrtWtEcqOH;
      string VnHKstfMMR;
      string xqWVAiziYw;
      string HLurCMKqKZ;
      string FFEpUtcZYz;
      string uTxZIaUYVK;
      string EsSSzKsOHg;
      string jnBlLrlgAG;
      string GZKYAmOUja;
      string rlVzVMllpK;
      string ANRrPHSKOP;
      string nmXOEdisSM;
      string wXefnySBQz;
      string FApoFrISJr;
      string UDGTmENGtq;
      string dsSdOcikDQ;
      string oQsPKnwiIM;
      string VQjZqrRLsj;
      string kLHZVMgMac;
      string IWOrDDKqIy;
      if(SrtWtEcqOH == ANRrPHSKOP){HdxhUNIrlW = true;}
      else if(ANRrPHSKOP == SrtWtEcqOH){PhElKKaLQa = true;}
      if(VnHKstfMMR == nmXOEdisSM){LXqTfcJTsy = true;}
      else if(nmXOEdisSM == VnHKstfMMR){NRCKCGjZEx = true;}
      if(xqWVAiziYw == wXefnySBQz){GjIfDzBrWn = true;}
      else if(wXefnySBQz == xqWVAiziYw){HdtmoTmgoG = true;}
      if(HLurCMKqKZ == FApoFrISJr){hZCtbGNZEI = true;}
      else if(FApoFrISJr == HLurCMKqKZ){ziniDkBskh = true;}
      if(FFEpUtcZYz == UDGTmENGtq){EoZEQkeuMZ = true;}
      else if(UDGTmENGtq == FFEpUtcZYz){PQZapRskfp = true;}
      if(uTxZIaUYVK == dsSdOcikDQ){tpxKrXVRdV = true;}
      else if(dsSdOcikDQ == uTxZIaUYVK){DrWSXknQxq = true;}
      if(EsSSzKsOHg == oQsPKnwiIM){LQVynkVwqZ = true;}
      else if(oQsPKnwiIM == EsSSzKsOHg){XtWKbedTHR = true;}
      if(jnBlLrlgAG == VQjZqrRLsj){PiPksRKDOS = true;}
      if(GZKYAmOUja == kLHZVMgMac){TFfwsqcqCE = true;}
      if(rlVzVMllpK == IWOrDDKqIy){juwhwmSaPf = true;}
      while(VQjZqrRLsj == jnBlLrlgAG){EqoWgpDSwX = true;}
      while(kLHZVMgMac == kLHZVMgMac){aCByCGMVKR = true;}
      while(IWOrDDKqIy == IWOrDDKqIy){wkutKVtOzL = true;}
      if(HdxhUNIrlW == true){HdxhUNIrlW = false;}
      if(LXqTfcJTsy == true){LXqTfcJTsy = false;}
      if(GjIfDzBrWn == true){GjIfDzBrWn = false;}
      if(hZCtbGNZEI == true){hZCtbGNZEI = false;}
      if(EoZEQkeuMZ == true){EoZEQkeuMZ = false;}
      if(tpxKrXVRdV == true){tpxKrXVRdV = false;}
      if(LQVynkVwqZ == true){LQVynkVwqZ = false;}
      if(PiPksRKDOS == true){PiPksRKDOS = false;}
      if(TFfwsqcqCE == true){TFfwsqcqCE = false;}
      if(juwhwmSaPf == true){juwhwmSaPf = false;}
      if(PhElKKaLQa == true){PhElKKaLQa = false;}
      if(NRCKCGjZEx == true){NRCKCGjZEx = false;}
      if(HdtmoTmgoG == true){HdtmoTmgoG = false;}
      if(ziniDkBskh == true){ziniDkBskh = false;}
      if(PQZapRskfp == true){PQZapRskfp = false;}
      if(DrWSXknQxq == true){DrWSXknQxq = false;}
      if(XtWKbedTHR == true){XtWKbedTHR = false;}
      if(EqoWgpDSwX == true){EqoWgpDSwX = false;}
      if(aCByCGMVKR == true){aCByCGMVKR = false;}
      if(wkutKVtOzL == true){wkutKVtOzL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WBQBAHPKFX
{ 
  void oAsfJILnOc()
  { 
      bool kGpDzZMpFu = false;
      bool IRzOAXXOHq = false;
      bool DopJgPnMaB = false;
      bool KromusVaEY = false;
      bool rkfbRAbmSj = false;
      bool ZaKkrwQfVi = false;
      bool kzOEjPcMSG = false;
      bool CwJiySINqf = false;
      bool nBspajhDtz = false;
      bool coFUmsVbrC = false;
      bool XOTMhcQalZ = false;
      bool mDnpujFYcA = false;
      bool gYRTcMqouF = false;
      bool wgVoIYggHy = false;
      bool TGQUMZBOZD = false;
      bool mxyqUyxAjV = false;
      bool kafgcGgzhV = false;
      bool DYQdgZIwHD = false;
      bool LcPqRzhdUJ = false;
      bool bgxlMrajrF = false;
      string eLzStWeaWw;
      string qcNHWaeUlH;
      string ZwsxmPPBje;
      string IRtDpxDkiz;
      string jistbnlLNs;
      string PWywjiRCee;
      string GcqqmZiKRY;
      string WslmXpCyUz;
      string cEnpPlSySH;
      string eBGErGrBGC;
      string EasoWWkLXs;
      string fSgeBBoAbB;
      string GkqKsVmhtQ;
      string xArStCKyTr;
      string BGZYaMUaCp;
      string QrZFuTjiwA;
      string jDRHweaNkz;
      string qqfizzKRig;
      string ALeDzEalXa;
      string SaCSIyoRcw;
      if(eLzStWeaWw == EasoWWkLXs){kGpDzZMpFu = true;}
      else if(EasoWWkLXs == eLzStWeaWw){XOTMhcQalZ = true;}
      if(qcNHWaeUlH == fSgeBBoAbB){IRzOAXXOHq = true;}
      else if(fSgeBBoAbB == qcNHWaeUlH){mDnpujFYcA = true;}
      if(ZwsxmPPBje == GkqKsVmhtQ){DopJgPnMaB = true;}
      else if(GkqKsVmhtQ == ZwsxmPPBje){gYRTcMqouF = true;}
      if(IRtDpxDkiz == xArStCKyTr){KromusVaEY = true;}
      else if(xArStCKyTr == IRtDpxDkiz){wgVoIYggHy = true;}
      if(jistbnlLNs == BGZYaMUaCp){rkfbRAbmSj = true;}
      else if(BGZYaMUaCp == jistbnlLNs){TGQUMZBOZD = true;}
      if(PWywjiRCee == QrZFuTjiwA){ZaKkrwQfVi = true;}
      else if(QrZFuTjiwA == PWywjiRCee){mxyqUyxAjV = true;}
      if(GcqqmZiKRY == jDRHweaNkz){kzOEjPcMSG = true;}
      else if(jDRHweaNkz == GcqqmZiKRY){kafgcGgzhV = true;}
      if(WslmXpCyUz == qqfizzKRig){CwJiySINqf = true;}
      if(cEnpPlSySH == ALeDzEalXa){nBspajhDtz = true;}
      if(eBGErGrBGC == SaCSIyoRcw){coFUmsVbrC = true;}
      while(qqfizzKRig == WslmXpCyUz){DYQdgZIwHD = true;}
      while(ALeDzEalXa == ALeDzEalXa){LcPqRzhdUJ = true;}
      while(SaCSIyoRcw == SaCSIyoRcw){bgxlMrajrF = true;}
      if(kGpDzZMpFu == true){kGpDzZMpFu = false;}
      if(IRzOAXXOHq == true){IRzOAXXOHq = false;}
      if(DopJgPnMaB == true){DopJgPnMaB = false;}
      if(KromusVaEY == true){KromusVaEY = false;}
      if(rkfbRAbmSj == true){rkfbRAbmSj = false;}
      if(ZaKkrwQfVi == true){ZaKkrwQfVi = false;}
      if(kzOEjPcMSG == true){kzOEjPcMSG = false;}
      if(CwJiySINqf == true){CwJiySINqf = false;}
      if(nBspajhDtz == true){nBspajhDtz = false;}
      if(coFUmsVbrC == true){coFUmsVbrC = false;}
      if(XOTMhcQalZ == true){XOTMhcQalZ = false;}
      if(mDnpujFYcA == true){mDnpujFYcA = false;}
      if(gYRTcMqouF == true){gYRTcMqouF = false;}
      if(wgVoIYggHy == true){wgVoIYggHy = false;}
      if(TGQUMZBOZD == true){TGQUMZBOZD = false;}
      if(mxyqUyxAjV == true){mxyqUyxAjV = false;}
      if(kafgcGgzhV == true){kafgcGgzhV = false;}
      if(DYQdgZIwHD == true){DYQdgZIwHD = false;}
      if(LcPqRzhdUJ == true){LcPqRzhdUJ = false;}
      if(bgxlMrajrF == true){bgxlMrajrF = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XSGUJHYJQW
{ 
  void STYNFEBFnJ()
  { 
      bool ZCxLXWMwYo = false;
      bool rLyogMYlrH = false;
      bool DPBOnrQGQd = false;
      bool BgklzxESqZ = false;
      bool cnLxsHAgYb = false;
      bool anUlSJAhXe = false;
      bool FGetmLaLpj = false;
      bool IFtRePDDVg = false;
      bool rignDNMySo = false;
      bool wVuMUWIRKh = false;
      bool foqoTUsCHD = false;
      bool GxLqmVZppY = false;
      bool ybrAmlKhxI = false;
      bool GksnACrqUh = false;
      bool UJqOizJDYy = false;
      bool xRakiEwqBg = false;
      bool KKEzxUEXqk = false;
      bool dhVHEokcRn = false;
      bool bGJzLnKHFw = false;
      bool FBpmXKEqRN = false;
      string zPccQjBXXj;
      string zwmGnZcQfH;
      string fdBLtWHZKz;
      string tjwJEbAFfo;
      string MohblktbZx;
      string jmYtDVNRit;
      string baNXpPAdCM;
      string YWjqupwZZJ;
      string bezMPGKCFi;
      string NhWlMlaJlV;
      string kQZNXNChyN;
      string YnrYSxaBIi;
      string FxXSypzyeI;
      string PlnDYLFlnC;
      string EcDrJZpySP;
      string LZBDQmrGOg;
      string NECpxmAnAH;
      string qnMFdbtfIY;
      string zBePxpjklk;
      string SyWGnzZNtW;
      if(zPccQjBXXj == kQZNXNChyN){ZCxLXWMwYo = true;}
      else if(kQZNXNChyN == zPccQjBXXj){foqoTUsCHD = true;}
      if(zwmGnZcQfH == YnrYSxaBIi){rLyogMYlrH = true;}
      else if(YnrYSxaBIi == zwmGnZcQfH){GxLqmVZppY = true;}
      if(fdBLtWHZKz == FxXSypzyeI){DPBOnrQGQd = true;}
      else if(FxXSypzyeI == fdBLtWHZKz){ybrAmlKhxI = true;}
      if(tjwJEbAFfo == PlnDYLFlnC){BgklzxESqZ = true;}
      else if(PlnDYLFlnC == tjwJEbAFfo){GksnACrqUh = true;}
      if(MohblktbZx == EcDrJZpySP){cnLxsHAgYb = true;}
      else if(EcDrJZpySP == MohblktbZx){UJqOizJDYy = true;}
      if(jmYtDVNRit == LZBDQmrGOg){anUlSJAhXe = true;}
      else if(LZBDQmrGOg == jmYtDVNRit){xRakiEwqBg = true;}
      if(baNXpPAdCM == NECpxmAnAH){FGetmLaLpj = true;}
      else if(NECpxmAnAH == baNXpPAdCM){KKEzxUEXqk = true;}
      if(YWjqupwZZJ == qnMFdbtfIY){IFtRePDDVg = true;}
      if(bezMPGKCFi == zBePxpjklk){rignDNMySo = true;}
      if(NhWlMlaJlV == SyWGnzZNtW){wVuMUWIRKh = true;}
      while(qnMFdbtfIY == YWjqupwZZJ){dhVHEokcRn = true;}
      while(zBePxpjklk == zBePxpjklk){bGJzLnKHFw = true;}
      while(SyWGnzZNtW == SyWGnzZNtW){FBpmXKEqRN = true;}
      if(ZCxLXWMwYo == true){ZCxLXWMwYo = false;}
      if(rLyogMYlrH == true){rLyogMYlrH = false;}
      if(DPBOnrQGQd == true){DPBOnrQGQd = false;}
      if(BgklzxESqZ == true){BgklzxESqZ = false;}
      if(cnLxsHAgYb == true){cnLxsHAgYb = false;}
      if(anUlSJAhXe == true){anUlSJAhXe = false;}
      if(FGetmLaLpj == true){FGetmLaLpj = false;}
      if(IFtRePDDVg == true){IFtRePDDVg = false;}
      if(rignDNMySo == true){rignDNMySo = false;}
      if(wVuMUWIRKh == true){wVuMUWIRKh = false;}
      if(foqoTUsCHD == true){foqoTUsCHD = false;}
      if(GxLqmVZppY == true){GxLqmVZppY = false;}
      if(ybrAmlKhxI == true){ybrAmlKhxI = false;}
      if(GksnACrqUh == true){GksnACrqUh = false;}
      if(UJqOizJDYy == true){UJqOizJDYy = false;}
      if(xRakiEwqBg == true){xRakiEwqBg = false;}
      if(KKEzxUEXqk == true){KKEzxUEXqk = false;}
      if(dhVHEokcRn == true){dhVHEokcRn = false;}
      if(bGJzLnKHFw == true){bGJzLnKHFw = false;}
      if(FBpmXKEqRN == true){FBpmXKEqRN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FJRKCDOXEJ
{ 
  void JEesiKXicJ()
  { 
      bool BZEpHrSPdz = false;
      bool ZkqkqEryxU = false;
      bool fcgkMESFRd = false;
      bool UXiDjtkiGN = false;
      bool qCsTfagjOA = false;
      bool KSOxMYsScD = false;
      bool ZNxAdCBnAx = false;
      bool TIqjcdjMFl = false;
      bool WipRSBIkpL = false;
      bool ASFDmdErod = false;
      bool ZsiRgBbeUi = false;
      bool nfNqfqWfgc = false;
      bool tQxnwACmwz = false;
      bool YqXbMdzXkQ = false;
      bool ZcQGXTEOWw = false;
      bool GXLMiIuuVf = false;
      bool zniUJxIleB = false;
      bool qzuPDQzpQb = false;
      bool wnPArAfKrD = false;
      bool iDBERktuwa = false;
      string LIgSFkVHNg;
      string nwCHlSZYgr;
      string FmfTltEnFG;
      string GfrhVNrjxG;
      string tJTlLNHOjg;
      string XgnmPYcMfK;
      string fhCHaIETlY;
      string ZpaaxuStHY;
      string mpDYhZqESx;
      string keDmaLHMLC;
      string JncLGhFMoT;
      string FiFHXpecYA;
      string QzAnntlnzl;
      string nGhEPyMqRA;
      string TePyrCZwOk;
      string fdnKlJmnWL;
      string uQMDeyAAqY;
      string cmLXgljCwl;
      string UpYOFKSWSF;
      string LYUGFCRMSs;
      if(LIgSFkVHNg == JncLGhFMoT){BZEpHrSPdz = true;}
      else if(JncLGhFMoT == LIgSFkVHNg){ZsiRgBbeUi = true;}
      if(nwCHlSZYgr == FiFHXpecYA){ZkqkqEryxU = true;}
      else if(FiFHXpecYA == nwCHlSZYgr){nfNqfqWfgc = true;}
      if(FmfTltEnFG == QzAnntlnzl){fcgkMESFRd = true;}
      else if(QzAnntlnzl == FmfTltEnFG){tQxnwACmwz = true;}
      if(GfrhVNrjxG == nGhEPyMqRA){UXiDjtkiGN = true;}
      else if(nGhEPyMqRA == GfrhVNrjxG){YqXbMdzXkQ = true;}
      if(tJTlLNHOjg == TePyrCZwOk){qCsTfagjOA = true;}
      else if(TePyrCZwOk == tJTlLNHOjg){ZcQGXTEOWw = true;}
      if(XgnmPYcMfK == fdnKlJmnWL){KSOxMYsScD = true;}
      else if(fdnKlJmnWL == XgnmPYcMfK){GXLMiIuuVf = true;}
      if(fhCHaIETlY == uQMDeyAAqY){ZNxAdCBnAx = true;}
      else if(uQMDeyAAqY == fhCHaIETlY){zniUJxIleB = true;}
      if(ZpaaxuStHY == cmLXgljCwl){TIqjcdjMFl = true;}
      if(mpDYhZqESx == UpYOFKSWSF){WipRSBIkpL = true;}
      if(keDmaLHMLC == LYUGFCRMSs){ASFDmdErod = true;}
      while(cmLXgljCwl == ZpaaxuStHY){qzuPDQzpQb = true;}
      while(UpYOFKSWSF == UpYOFKSWSF){wnPArAfKrD = true;}
      while(LYUGFCRMSs == LYUGFCRMSs){iDBERktuwa = true;}
      if(BZEpHrSPdz == true){BZEpHrSPdz = false;}
      if(ZkqkqEryxU == true){ZkqkqEryxU = false;}
      if(fcgkMESFRd == true){fcgkMESFRd = false;}
      if(UXiDjtkiGN == true){UXiDjtkiGN = false;}
      if(qCsTfagjOA == true){qCsTfagjOA = false;}
      if(KSOxMYsScD == true){KSOxMYsScD = false;}
      if(ZNxAdCBnAx == true){ZNxAdCBnAx = false;}
      if(TIqjcdjMFl == true){TIqjcdjMFl = false;}
      if(WipRSBIkpL == true){WipRSBIkpL = false;}
      if(ASFDmdErod == true){ASFDmdErod = false;}
      if(ZsiRgBbeUi == true){ZsiRgBbeUi = false;}
      if(nfNqfqWfgc == true){nfNqfqWfgc = false;}
      if(tQxnwACmwz == true){tQxnwACmwz = false;}
      if(YqXbMdzXkQ == true){YqXbMdzXkQ = false;}
      if(ZcQGXTEOWw == true){ZcQGXTEOWw = false;}
      if(GXLMiIuuVf == true){GXLMiIuuVf = false;}
      if(zniUJxIleB == true){zniUJxIleB = false;}
      if(qzuPDQzpQb == true){qzuPDQzpQb = false;}
      if(wnPArAfKrD == true){wnPArAfKrD = false;}
      if(iDBERktuwa == true){iDBERktuwa = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PNNGEKJNDR
{ 
  void SioLyqXhTm()
  { 
      bool sRTLmsGJqr = false;
      bool ABaTWdwCig = false;
      bool FYLWVGDNXA = false;
      bool YNCgVlXXof = false;
      bool rFsKoJlOeB = false;
      bool rwWxqTGxof = false;
      bool prlpSlysxk = false;
      bool FYMCnzfAhL = false;
      bool UniWIcqEpC = false;
      bool fahIpzTCrC = false;
      bool KsRtuTiTIr = false;
      bool mFPdLCJOho = false;
      bool utDuEdpEMo = false;
      bool edIfAbWobn = false;
      bool sWuxOaLExm = false;
      bool oFTCGQoVoQ = false;
      bool ceGTMJwjcj = false;
      bool eUmswXyFfd = false;
      bool SlGYzZJGYi = false;
      bool lAzKLOPbkE = false;
      string FQSDgZpqPQ;
      string cwwSDlEEVD;
      string lMjyYpYPGP;
      string dijuIJkeJs;
      string LARcLNYIuO;
      string TBqGksgUeq;
      string AZkAlsLAKt;
      string BYtcqktGlK;
      string QPTqwbrdJP;
      string bPASrlPRpR;
      string BXyBNtHOOA;
      string yZiUZkFsEJ;
      string BhptRkXbjF;
      string UcQTIsQSdB;
      string UcOJRCgfrH;
      string fWMfQThDib;
      string VfeIeUJOLg;
      string TlPDoXTgcP;
      string EribZyPjDo;
      string LMiCeoyFdw;
      if(FQSDgZpqPQ == BXyBNtHOOA){sRTLmsGJqr = true;}
      else if(BXyBNtHOOA == FQSDgZpqPQ){KsRtuTiTIr = true;}
      if(cwwSDlEEVD == yZiUZkFsEJ){ABaTWdwCig = true;}
      else if(yZiUZkFsEJ == cwwSDlEEVD){mFPdLCJOho = true;}
      if(lMjyYpYPGP == BhptRkXbjF){FYLWVGDNXA = true;}
      else if(BhptRkXbjF == lMjyYpYPGP){utDuEdpEMo = true;}
      if(dijuIJkeJs == UcQTIsQSdB){YNCgVlXXof = true;}
      else if(UcQTIsQSdB == dijuIJkeJs){edIfAbWobn = true;}
      if(LARcLNYIuO == UcOJRCgfrH){rFsKoJlOeB = true;}
      else if(UcOJRCgfrH == LARcLNYIuO){sWuxOaLExm = true;}
      if(TBqGksgUeq == fWMfQThDib){rwWxqTGxof = true;}
      else if(fWMfQThDib == TBqGksgUeq){oFTCGQoVoQ = true;}
      if(AZkAlsLAKt == VfeIeUJOLg){prlpSlysxk = true;}
      else if(VfeIeUJOLg == AZkAlsLAKt){ceGTMJwjcj = true;}
      if(BYtcqktGlK == TlPDoXTgcP){FYMCnzfAhL = true;}
      if(QPTqwbrdJP == EribZyPjDo){UniWIcqEpC = true;}
      if(bPASrlPRpR == LMiCeoyFdw){fahIpzTCrC = true;}
      while(TlPDoXTgcP == BYtcqktGlK){eUmswXyFfd = true;}
      while(EribZyPjDo == EribZyPjDo){SlGYzZJGYi = true;}
      while(LMiCeoyFdw == LMiCeoyFdw){lAzKLOPbkE = true;}
      if(sRTLmsGJqr == true){sRTLmsGJqr = false;}
      if(ABaTWdwCig == true){ABaTWdwCig = false;}
      if(FYLWVGDNXA == true){FYLWVGDNXA = false;}
      if(YNCgVlXXof == true){YNCgVlXXof = false;}
      if(rFsKoJlOeB == true){rFsKoJlOeB = false;}
      if(rwWxqTGxof == true){rwWxqTGxof = false;}
      if(prlpSlysxk == true){prlpSlysxk = false;}
      if(FYMCnzfAhL == true){FYMCnzfAhL = false;}
      if(UniWIcqEpC == true){UniWIcqEpC = false;}
      if(fahIpzTCrC == true){fahIpzTCrC = false;}
      if(KsRtuTiTIr == true){KsRtuTiTIr = false;}
      if(mFPdLCJOho == true){mFPdLCJOho = false;}
      if(utDuEdpEMo == true){utDuEdpEMo = false;}
      if(edIfAbWobn == true){edIfAbWobn = false;}
      if(sWuxOaLExm == true){sWuxOaLExm = false;}
      if(oFTCGQoVoQ == true){oFTCGQoVoQ = false;}
      if(ceGTMJwjcj == true){ceGTMJwjcj = false;}
      if(eUmswXyFfd == true){eUmswXyFfd = false;}
      if(SlGYzZJGYi == true){SlGYzZJGYi = false;}
      if(lAzKLOPbkE == true){lAzKLOPbkE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XQALFQIVRF
{ 
  void uVhcezeyiZ()
  { 
      bool ULpWXdBeNg = false;
      bool kooKtHkYIs = false;
      bool zBBBWKNRoJ = false;
      bool KzIKJWZHoT = false;
      bool DlJTDNXnkM = false;
      bool CWsxoTJoAM = false;
      bool PipDVkfdAX = false;
      bool hciooNqhxI = false;
      bool mZfbGnGLVp = false;
      bool MkHcEUwFxR = false;
      bool gsnMeZmEHa = false;
      bool baqqZpDNxh = false;
      bool eUIJKwVFjU = false;
      bool RljfbskSwf = false;
      bool ACDtOlsPDO = false;
      bool RhIPmLjdCh = false;
      bool EhTlJppLPf = false;
      bool EjQQgbQOuY = false;
      bool DFCVDidUrP = false;
      bool JHQObShRjX = false;
      string kGRPOouxrW;
      string SsklYSXiQi;
      string BLEJedVoJZ;
      string ZBgEDBPgum;
      string ArLJrIKclb;
      string LeYdLYBIkj;
      string SGmRxQMgiX;
      string DqmdiIAuQV;
      string TsDgOucDrd;
      string zGuLUtEsMb;
      string ucsNYUquuC;
      string XGrPPYCFDB;
      string nJoFXwwaJn;
      string DGhoZpAoIs;
      string EgHoFsOewu;
      string qyoKmlnnpc;
      string YPlGyRXDoY;
      string XNxKxGBzze;
      string hQeNxFRWBn;
      string CMzfUnuBpn;
      if(kGRPOouxrW == ucsNYUquuC){ULpWXdBeNg = true;}
      else if(ucsNYUquuC == kGRPOouxrW){gsnMeZmEHa = true;}
      if(SsklYSXiQi == XGrPPYCFDB){kooKtHkYIs = true;}
      else if(XGrPPYCFDB == SsklYSXiQi){baqqZpDNxh = true;}
      if(BLEJedVoJZ == nJoFXwwaJn){zBBBWKNRoJ = true;}
      else if(nJoFXwwaJn == BLEJedVoJZ){eUIJKwVFjU = true;}
      if(ZBgEDBPgum == DGhoZpAoIs){KzIKJWZHoT = true;}
      else if(DGhoZpAoIs == ZBgEDBPgum){RljfbskSwf = true;}
      if(ArLJrIKclb == EgHoFsOewu){DlJTDNXnkM = true;}
      else if(EgHoFsOewu == ArLJrIKclb){ACDtOlsPDO = true;}
      if(LeYdLYBIkj == qyoKmlnnpc){CWsxoTJoAM = true;}
      else if(qyoKmlnnpc == LeYdLYBIkj){RhIPmLjdCh = true;}
      if(SGmRxQMgiX == YPlGyRXDoY){PipDVkfdAX = true;}
      else if(YPlGyRXDoY == SGmRxQMgiX){EhTlJppLPf = true;}
      if(DqmdiIAuQV == XNxKxGBzze){hciooNqhxI = true;}
      if(TsDgOucDrd == hQeNxFRWBn){mZfbGnGLVp = true;}
      if(zGuLUtEsMb == CMzfUnuBpn){MkHcEUwFxR = true;}
      while(XNxKxGBzze == DqmdiIAuQV){EjQQgbQOuY = true;}
      while(hQeNxFRWBn == hQeNxFRWBn){DFCVDidUrP = true;}
      while(CMzfUnuBpn == CMzfUnuBpn){JHQObShRjX = true;}
      if(ULpWXdBeNg == true){ULpWXdBeNg = false;}
      if(kooKtHkYIs == true){kooKtHkYIs = false;}
      if(zBBBWKNRoJ == true){zBBBWKNRoJ = false;}
      if(KzIKJWZHoT == true){KzIKJWZHoT = false;}
      if(DlJTDNXnkM == true){DlJTDNXnkM = false;}
      if(CWsxoTJoAM == true){CWsxoTJoAM = false;}
      if(PipDVkfdAX == true){PipDVkfdAX = false;}
      if(hciooNqhxI == true){hciooNqhxI = false;}
      if(mZfbGnGLVp == true){mZfbGnGLVp = false;}
      if(MkHcEUwFxR == true){MkHcEUwFxR = false;}
      if(gsnMeZmEHa == true){gsnMeZmEHa = false;}
      if(baqqZpDNxh == true){baqqZpDNxh = false;}
      if(eUIJKwVFjU == true){eUIJKwVFjU = false;}
      if(RljfbskSwf == true){RljfbskSwf = false;}
      if(ACDtOlsPDO == true){ACDtOlsPDO = false;}
      if(RhIPmLjdCh == true){RhIPmLjdCh = false;}
      if(EhTlJppLPf == true){EhTlJppLPf = false;}
      if(EjQQgbQOuY == true){EjQQgbQOuY = false;}
      if(DFCVDidUrP == true){DFCVDidUrP = false;}
      if(JHQObShRjX == true){JHQObShRjX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JTEOOPPOMD
{ 
  void NgWtmOZxfn()
  { 
      bool lLRZLRoFoD = false;
      bool hWTqCWryjQ = false;
      bool MlOBLaedJu = false;
      bool DCZDcZnZnz = false;
      bool ufICWOMCZF = false;
      bool AMenTzwQOj = false;
      bool mKwtzexPOb = false;
      bool iSJjUfcMdx = false;
      bool NbEDtqXlic = false;
      bool cXLELkjOCr = false;
      bool RxmxWhCtTn = false;
      bool VjlRVLFrzI = false;
      bool GKfkyKGFYi = false;
      bool wOKlfQnGfM = false;
      bool ncRuGslthA = false;
      bool eqPskGQCzL = false;
      bool bxVjrHZzkF = false;
      bool WzLlTdrjeF = false;
      bool YMWYEEyQAZ = false;
      bool zlHUkqHhDz = false;
      string RVIjFDWPDl;
      string RTojFcDGsD;
      string FSXzipDGXq;
      string FVJQIiilXN;
      string fIznktNwOA;
      string ZAwhcyyIBO;
      string AQrLguKYaW;
      string tQhfXwTjOo;
      string cGrWQFYamK;
      string kakJMPQnGo;
      string inUrGJRQKg;
      string eNtLksYVtb;
      string WilQlSfYSI;
      string efcYFnkAPH;
      string mIQGfWgYXT;
      string ptseSACEGn;
      string dsxBBzkUGx;
      string HYFmoOSekO;
      string lGAzRklyGE;
      string kgsbcsKPEg;
      if(RVIjFDWPDl == inUrGJRQKg){lLRZLRoFoD = true;}
      else if(inUrGJRQKg == RVIjFDWPDl){RxmxWhCtTn = true;}
      if(RTojFcDGsD == eNtLksYVtb){hWTqCWryjQ = true;}
      else if(eNtLksYVtb == RTojFcDGsD){VjlRVLFrzI = true;}
      if(FSXzipDGXq == WilQlSfYSI){MlOBLaedJu = true;}
      else if(WilQlSfYSI == FSXzipDGXq){GKfkyKGFYi = true;}
      if(FVJQIiilXN == efcYFnkAPH){DCZDcZnZnz = true;}
      else if(efcYFnkAPH == FVJQIiilXN){wOKlfQnGfM = true;}
      if(fIznktNwOA == mIQGfWgYXT){ufICWOMCZF = true;}
      else if(mIQGfWgYXT == fIznktNwOA){ncRuGslthA = true;}
      if(ZAwhcyyIBO == ptseSACEGn){AMenTzwQOj = true;}
      else if(ptseSACEGn == ZAwhcyyIBO){eqPskGQCzL = true;}
      if(AQrLguKYaW == dsxBBzkUGx){mKwtzexPOb = true;}
      else if(dsxBBzkUGx == AQrLguKYaW){bxVjrHZzkF = true;}
      if(tQhfXwTjOo == HYFmoOSekO){iSJjUfcMdx = true;}
      if(cGrWQFYamK == lGAzRklyGE){NbEDtqXlic = true;}
      if(kakJMPQnGo == kgsbcsKPEg){cXLELkjOCr = true;}
      while(HYFmoOSekO == tQhfXwTjOo){WzLlTdrjeF = true;}
      while(lGAzRklyGE == lGAzRklyGE){YMWYEEyQAZ = true;}
      while(kgsbcsKPEg == kgsbcsKPEg){zlHUkqHhDz = true;}
      if(lLRZLRoFoD == true){lLRZLRoFoD = false;}
      if(hWTqCWryjQ == true){hWTqCWryjQ = false;}
      if(MlOBLaedJu == true){MlOBLaedJu = false;}
      if(DCZDcZnZnz == true){DCZDcZnZnz = false;}
      if(ufICWOMCZF == true){ufICWOMCZF = false;}
      if(AMenTzwQOj == true){AMenTzwQOj = false;}
      if(mKwtzexPOb == true){mKwtzexPOb = false;}
      if(iSJjUfcMdx == true){iSJjUfcMdx = false;}
      if(NbEDtqXlic == true){NbEDtqXlic = false;}
      if(cXLELkjOCr == true){cXLELkjOCr = false;}
      if(RxmxWhCtTn == true){RxmxWhCtTn = false;}
      if(VjlRVLFrzI == true){VjlRVLFrzI = false;}
      if(GKfkyKGFYi == true){GKfkyKGFYi = false;}
      if(wOKlfQnGfM == true){wOKlfQnGfM = false;}
      if(ncRuGslthA == true){ncRuGslthA = false;}
      if(eqPskGQCzL == true){eqPskGQCzL = false;}
      if(bxVjrHZzkF == true){bxVjrHZzkF = false;}
      if(WzLlTdrjeF == true){WzLlTdrjeF = false;}
      if(YMWYEEyQAZ == true){YMWYEEyQAZ = false;}
      if(zlHUkqHhDz == true){zlHUkqHhDz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DEJSUFPECH
{ 
  void wYAncwhCuO()
  { 
      bool ciGjcQQgfV = false;
      bool rUwRVlFJdJ = false;
      bool prAMoZxFHI = false;
      bool DDKuXFOOOq = false;
      bool hVVRZikhSp = false;
      bool CDxdqkhsuD = false;
      bool TKZXSnJAsP = false;
      bool WpulaUWhmF = false;
      bool nOlRFzFfEm = false;
      bool WUTKXXwyzk = false;
      bool ZjnCSMHLCW = false;
      bool OAGpOEXIRS = false;
      bool icwGsCWkOD = false;
      bool XXiBfEajys = false;
      bool rtdgYWNduT = false;
      bool znffCwnhJQ = false;
      bool oWxzypQFJK = false;
      bool dnJYanzOgV = false;
      bool nsouDjqDyj = false;
      bool PrZhDVwdhh = false;
      string WByHFZTJmc;
      string YgEKfAtOdu;
      string hCfmgGVtsX;
      string BzUuLnzgrC;
      string zTyDrbWgjX;
      string aEFBgTVKZz;
      string DYPOaZLJzq;
      string ANVauNEbtF;
      string joKVwwsTOr;
      string RnGszaEibS;
      string PNgrWNhdGS;
      string PtQUPONsSe;
      string frtGZMGcbU;
      string JUzboWHRzi;
      string NPMwcXXlcj;
      string MKQxmWSZSK;
      string GFBkIttoes;
      string GwtiQQeydB;
      string nUzrHyDCnR;
      string DgzobINbmh;
      if(WByHFZTJmc == PNgrWNhdGS){ciGjcQQgfV = true;}
      else if(PNgrWNhdGS == WByHFZTJmc){ZjnCSMHLCW = true;}
      if(YgEKfAtOdu == PtQUPONsSe){rUwRVlFJdJ = true;}
      else if(PtQUPONsSe == YgEKfAtOdu){OAGpOEXIRS = true;}
      if(hCfmgGVtsX == frtGZMGcbU){prAMoZxFHI = true;}
      else if(frtGZMGcbU == hCfmgGVtsX){icwGsCWkOD = true;}
      if(BzUuLnzgrC == JUzboWHRzi){DDKuXFOOOq = true;}
      else if(JUzboWHRzi == BzUuLnzgrC){XXiBfEajys = true;}
      if(zTyDrbWgjX == NPMwcXXlcj){hVVRZikhSp = true;}
      else if(NPMwcXXlcj == zTyDrbWgjX){rtdgYWNduT = true;}
      if(aEFBgTVKZz == MKQxmWSZSK){CDxdqkhsuD = true;}
      else if(MKQxmWSZSK == aEFBgTVKZz){znffCwnhJQ = true;}
      if(DYPOaZLJzq == GFBkIttoes){TKZXSnJAsP = true;}
      else if(GFBkIttoes == DYPOaZLJzq){oWxzypQFJK = true;}
      if(ANVauNEbtF == GwtiQQeydB){WpulaUWhmF = true;}
      if(joKVwwsTOr == nUzrHyDCnR){nOlRFzFfEm = true;}
      if(RnGszaEibS == DgzobINbmh){WUTKXXwyzk = true;}
      while(GwtiQQeydB == ANVauNEbtF){dnJYanzOgV = true;}
      while(nUzrHyDCnR == nUzrHyDCnR){nsouDjqDyj = true;}
      while(DgzobINbmh == DgzobINbmh){PrZhDVwdhh = true;}
      if(ciGjcQQgfV == true){ciGjcQQgfV = false;}
      if(rUwRVlFJdJ == true){rUwRVlFJdJ = false;}
      if(prAMoZxFHI == true){prAMoZxFHI = false;}
      if(DDKuXFOOOq == true){DDKuXFOOOq = false;}
      if(hVVRZikhSp == true){hVVRZikhSp = false;}
      if(CDxdqkhsuD == true){CDxdqkhsuD = false;}
      if(TKZXSnJAsP == true){TKZXSnJAsP = false;}
      if(WpulaUWhmF == true){WpulaUWhmF = false;}
      if(nOlRFzFfEm == true){nOlRFzFfEm = false;}
      if(WUTKXXwyzk == true){WUTKXXwyzk = false;}
      if(ZjnCSMHLCW == true){ZjnCSMHLCW = false;}
      if(OAGpOEXIRS == true){OAGpOEXIRS = false;}
      if(icwGsCWkOD == true){icwGsCWkOD = false;}
      if(XXiBfEajys == true){XXiBfEajys = false;}
      if(rtdgYWNduT == true){rtdgYWNduT = false;}
      if(znffCwnhJQ == true){znffCwnhJQ = false;}
      if(oWxzypQFJK == true){oWxzypQFJK = false;}
      if(dnJYanzOgV == true){dnJYanzOgV = false;}
      if(nsouDjqDyj == true){nsouDjqDyj = false;}
      if(PrZhDVwdhh == true){PrZhDVwdhh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WRPZPQBMUQ
{ 
  void MjCHwtMVYi()
  { 
      bool DzsmCjxPZK = false;
      bool whdRXxsBpH = false;
      bool wzPPQAPDqP = false;
      bool VDERyxGRTB = false;
      bool KopVTAtTiV = false;
      bool nBPqIDjdVC = false;
      bool TsXKorcLTZ = false;
      bool qagnxHJswQ = false;
      bool GuUVaTnNJt = false;
      bool NbwRgQQfga = false;
      bool XhAIdMUasB = false;
      bool tphFuYXCjU = false;
      bool EPCYMOeOGg = false;
      bool pcMUsYtLqN = false;
      bool zNTHzWtqVI = false;
      bool sOuLWHeawP = false;
      bool hFTMKQdMLW = false;
      bool dLKVVputYo = false;
      bool TnsymbhAyk = false;
      bool sSPafssHwh = false;
      string EZFIgEzZWz;
      string uVtORXtsAU;
      string csgonoBpAJ;
      string EtyhxTXtwc;
      string DTNtmUQwqM;
      string WkuQnKrPTq;
      string DIZNuBXMQy;
      string AzwXRJemCS;
      string XFzmmTzgsl;
      string aIaFZttAyl;
      string xEerMWnWze;
      string fFMBSyIlab;
      string BqSdXrYqyW;
      string PenQVTZGaP;
      string ebhOGUuYla;
      string kHZWjFkBUU;
      string xxYbPBQwfz;
      string YBKKwYJDWC;
      string wAEWkHEQgP;
      string hXlGSgDFzF;
      if(EZFIgEzZWz == xEerMWnWze){DzsmCjxPZK = true;}
      else if(xEerMWnWze == EZFIgEzZWz){XhAIdMUasB = true;}
      if(uVtORXtsAU == fFMBSyIlab){whdRXxsBpH = true;}
      else if(fFMBSyIlab == uVtORXtsAU){tphFuYXCjU = true;}
      if(csgonoBpAJ == BqSdXrYqyW){wzPPQAPDqP = true;}
      else if(BqSdXrYqyW == csgonoBpAJ){EPCYMOeOGg = true;}
      if(EtyhxTXtwc == PenQVTZGaP){VDERyxGRTB = true;}
      else if(PenQVTZGaP == EtyhxTXtwc){pcMUsYtLqN = true;}
      if(DTNtmUQwqM == ebhOGUuYla){KopVTAtTiV = true;}
      else if(ebhOGUuYla == DTNtmUQwqM){zNTHzWtqVI = true;}
      if(WkuQnKrPTq == kHZWjFkBUU){nBPqIDjdVC = true;}
      else if(kHZWjFkBUU == WkuQnKrPTq){sOuLWHeawP = true;}
      if(DIZNuBXMQy == xxYbPBQwfz){TsXKorcLTZ = true;}
      else if(xxYbPBQwfz == DIZNuBXMQy){hFTMKQdMLW = true;}
      if(AzwXRJemCS == YBKKwYJDWC){qagnxHJswQ = true;}
      if(XFzmmTzgsl == wAEWkHEQgP){GuUVaTnNJt = true;}
      if(aIaFZttAyl == hXlGSgDFzF){NbwRgQQfga = true;}
      while(YBKKwYJDWC == AzwXRJemCS){dLKVVputYo = true;}
      while(wAEWkHEQgP == wAEWkHEQgP){TnsymbhAyk = true;}
      while(hXlGSgDFzF == hXlGSgDFzF){sSPafssHwh = true;}
      if(DzsmCjxPZK == true){DzsmCjxPZK = false;}
      if(whdRXxsBpH == true){whdRXxsBpH = false;}
      if(wzPPQAPDqP == true){wzPPQAPDqP = false;}
      if(VDERyxGRTB == true){VDERyxGRTB = false;}
      if(KopVTAtTiV == true){KopVTAtTiV = false;}
      if(nBPqIDjdVC == true){nBPqIDjdVC = false;}
      if(TsXKorcLTZ == true){TsXKorcLTZ = false;}
      if(qagnxHJswQ == true){qagnxHJswQ = false;}
      if(GuUVaTnNJt == true){GuUVaTnNJt = false;}
      if(NbwRgQQfga == true){NbwRgQQfga = false;}
      if(XhAIdMUasB == true){XhAIdMUasB = false;}
      if(tphFuYXCjU == true){tphFuYXCjU = false;}
      if(EPCYMOeOGg == true){EPCYMOeOGg = false;}
      if(pcMUsYtLqN == true){pcMUsYtLqN = false;}
      if(zNTHzWtqVI == true){zNTHzWtqVI = false;}
      if(sOuLWHeawP == true){sOuLWHeawP = false;}
      if(hFTMKQdMLW == true){hFTMKQdMLW = false;}
      if(dLKVVputYo == true){dLKVVputYo = false;}
      if(TnsymbhAyk == true){TnsymbhAyk = false;}
      if(sSPafssHwh == true){sSPafssHwh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MPUKRKNTLO
{ 
  void MIQcldGWQm()
  { 
      bool DqZTlDYTaV = false;
      bool IwFExXRppB = false;
      bool YFeGpoHXWY = false;
      bool jnyWkgZLkl = false;
      bool UgcmTnUbiu = false;
      bool cdaPexLTdw = false;
      bool wAwCkTxiSQ = false;
      bool yGEDgrcrmB = false;
      bool UfobuFNiqm = false;
      bool UGgnpTjaBz = false;
      bool plIRxWZWhm = false;
      bool UTgbbPKUGf = false;
      bool GOqeurYPNM = false;
      bool HCpmLiXWeY = false;
      bool fpAESBYFMa = false;
      bool JPTVsatcWF = false;
      bool xJJUeRcsQr = false;
      bool qmohzPOlMt = false;
      bool STDlDriUtk = false;
      bool nuAZaWgfZB = false;
      string tyjHeQezDB;
      string SdJYFetHph;
      string gFlQgaAsNu;
      string ZJPDEgWFSp;
      string sXBgUcrHVi;
      string VKnEZoMTaD;
      string QllHaINFUX;
      string InnzoIDEuD;
      string hsXdYOVLdO;
      string UDeOZGlsGM;
      string OmlxagWnOR;
      string kHXWUWOIhL;
      string lBbFDEIYZh;
      string WKyiouGZDr;
      string rhxLdNfIsC;
      string rdKtZMLgtW;
      string YxNfuZJnhZ;
      string PnEpTEEPao;
      string YAeXpsADIb;
      string jZfgTuTQQE;
      if(tyjHeQezDB == OmlxagWnOR){DqZTlDYTaV = true;}
      else if(OmlxagWnOR == tyjHeQezDB){plIRxWZWhm = true;}
      if(SdJYFetHph == kHXWUWOIhL){IwFExXRppB = true;}
      else if(kHXWUWOIhL == SdJYFetHph){UTgbbPKUGf = true;}
      if(gFlQgaAsNu == lBbFDEIYZh){YFeGpoHXWY = true;}
      else if(lBbFDEIYZh == gFlQgaAsNu){GOqeurYPNM = true;}
      if(ZJPDEgWFSp == WKyiouGZDr){jnyWkgZLkl = true;}
      else if(WKyiouGZDr == ZJPDEgWFSp){HCpmLiXWeY = true;}
      if(sXBgUcrHVi == rhxLdNfIsC){UgcmTnUbiu = true;}
      else if(rhxLdNfIsC == sXBgUcrHVi){fpAESBYFMa = true;}
      if(VKnEZoMTaD == rdKtZMLgtW){cdaPexLTdw = true;}
      else if(rdKtZMLgtW == VKnEZoMTaD){JPTVsatcWF = true;}
      if(QllHaINFUX == YxNfuZJnhZ){wAwCkTxiSQ = true;}
      else if(YxNfuZJnhZ == QllHaINFUX){xJJUeRcsQr = true;}
      if(InnzoIDEuD == PnEpTEEPao){yGEDgrcrmB = true;}
      if(hsXdYOVLdO == YAeXpsADIb){UfobuFNiqm = true;}
      if(UDeOZGlsGM == jZfgTuTQQE){UGgnpTjaBz = true;}
      while(PnEpTEEPao == InnzoIDEuD){qmohzPOlMt = true;}
      while(YAeXpsADIb == YAeXpsADIb){STDlDriUtk = true;}
      while(jZfgTuTQQE == jZfgTuTQQE){nuAZaWgfZB = true;}
      if(DqZTlDYTaV == true){DqZTlDYTaV = false;}
      if(IwFExXRppB == true){IwFExXRppB = false;}
      if(YFeGpoHXWY == true){YFeGpoHXWY = false;}
      if(jnyWkgZLkl == true){jnyWkgZLkl = false;}
      if(UgcmTnUbiu == true){UgcmTnUbiu = false;}
      if(cdaPexLTdw == true){cdaPexLTdw = false;}
      if(wAwCkTxiSQ == true){wAwCkTxiSQ = false;}
      if(yGEDgrcrmB == true){yGEDgrcrmB = false;}
      if(UfobuFNiqm == true){UfobuFNiqm = false;}
      if(UGgnpTjaBz == true){UGgnpTjaBz = false;}
      if(plIRxWZWhm == true){plIRxWZWhm = false;}
      if(UTgbbPKUGf == true){UTgbbPKUGf = false;}
      if(GOqeurYPNM == true){GOqeurYPNM = false;}
      if(HCpmLiXWeY == true){HCpmLiXWeY = false;}
      if(fpAESBYFMa == true){fpAESBYFMa = false;}
      if(JPTVsatcWF == true){JPTVsatcWF = false;}
      if(xJJUeRcsQr == true){xJJUeRcsQr = false;}
      if(qmohzPOlMt == true){qmohzPOlMt = false;}
      if(STDlDriUtk == true){STDlDriUtk = false;}
      if(nuAZaWgfZB == true){nuAZaWgfZB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DKJTHTVGIG
{ 
  void MDMJaeGVap()
  { 
      bool KnldIFCinn = false;
      bool bSYOkiNZSL = false;
      bool ppfHiyNGAg = false;
      bool NCWOHSgVrh = false;
      bool aEANFRtxVE = false;
      bool CKbEXUIetK = false;
      bool quhGaZuJGR = false;
      bool CqgjkkoRCR = false;
      bool DQjweCZRqm = false;
      bool KBbhqeVtme = false;
      bool VjpECKdqTl = false;
      bool epbbGaLlaX = false;
      bool AdSSHAPzsF = false;
      bool PtdXwSZesU = false;
      bool YiyEITYzaP = false;
      bool SiSPeqRXwB = false;
      bool ueyNWkMEfm = false;
      bool BfsGzQeNKm = false;
      bool aPXclfbOrE = false;
      bool BChqYsjjhX = false;
      string ptPBjHfzXO;
      string iyetkXRUpa;
      string mjFHbeYaOE;
      string DKXuoiByOy;
      string FOVVVxkdqr;
      string QQuwGFoIZF;
      string iEjSLYZiFC;
      string VTGrUXOjkd;
      string stiVnlOboi;
      string tsXJPBdCJw;
      string WBYoVxJDah;
      string pEWwLnUNeY;
      string JytVHcuxbT;
      string sLZoGqVtAp;
      string bLBGroVbxa;
      string fPXklYHCgr;
      string CMaGodehoT;
      string RaNnwywncS;
      string HSzBncnoCh;
      string jTMHnPjtQN;
      if(ptPBjHfzXO == WBYoVxJDah){KnldIFCinn = true;}
      else if(WBYoVxJDah == ptPBjHfzXO){VjpECKdqTl = true;}
      if(iyetkXRUpa == pEWwLnUNeY){bSYOkiNZSL = true;}
      else if(pEWwLnUNeY == iyetkXRUpa){epbbGaLlaX = true;}
      if(mjFHbeYaOE == JytVHcuxbT){ppfHiyNGAg = true;}
      else if(JytVHcuxbT == mjFHbeYaOE){AdSSHAPzsF = true;}
      if(DKXuoiByOy == sLZoGqVtAp){NCWOHSgVrh = true;}
      else if(sLZoGqVtAp == DKXuoiByOy){PtdXwSZesU = true;}
      if(FOVVVxkdqr == bLBGroVbxa){aEANFRtxVE = true;}
      else if(bLBGroVbxa == FOVVVxkdqr){YiyEITYzaP = true;}
      if(QQuwGFoIZF == fPXklYHCgr){CKbEXUIetK = true;}
      else if(fPXklYHCgr == QQuwGFoIZF){SiSPeqRXwB = true;}
      if(iEjSLYZiFC == CMaGodehoT){quhGaZuJGR = true;}
      else if(CMaGodehoT == iEjSLYZiFC){ueyNWkMEfm = true;}
      if(VTGrUXOjkd == RaNnwywncS){CqgjkkoRCR = true;}
      if(stiVnlOboi == HSzBncnoCh){DQjweCZRqm = true;}
      if(tsXJPBdCJw == jTMHnPjtQN){KBbhqeVtme = true;}
      while(RaNnwywncS == VTGrUXOjkd){BfsGzQeNKm = true;}
      while(HSzBncnoCh == HSzBncnoCh){aPXclfbOrE = true;}
      while(jTMHnPjtQN == jTMHnPjtQN){BChqYsjjhX = true;}
      if(KnldIFCinn == true){KnldIFCinn = false;}
      if(bSYOkiNZSL == true){bSYOkiNZSL = false;}
      if(ppfHiyNGAg == true){ppfHiyNGAg = false;}
      if(NCWOHSgVrh == true){NCWOHSgVrh = false;}
      if(aEANFRtxVE == true){aEANFRtxVE = false;}
      if(CKbEXUIetK == true){CKbEXUIetK = false;}
      if(quhGaZuJGR == true){quhGaZuJGR = false;}
      if(CqgjkkoRCR == true){CqgjkkoRCR = false;}
      if(DQjweCZRqm == true){DQjweCZRqm = false;}
      if(KBbhqeVtme == true){KBbhqeVtme = false;}
      if(VjpECKdqTl == true){VjpECKdqTl = false;}
      if(epbbGaLlaX == true){epbbGaLlaX = false;}
      if(AdSSHAPzsF == true){AdSSHAPzsF = false;}
      if(PtdXwSZesU == true){PtdXwSZesU = false;}
      if(YiyEITYzaP == true){YiyEITYzaP = false;}
      if(SiSPeqRXwB == true){SiSPeqRXwB = false;}
      if(ueyNWkMEfm == true){ueyNWkMEfm = false;}
      if(BfsGzQeNKm == true){BfsGzQeNKm = false;}
      if(aPXclfbOrE == true){aPXclfbOrE = false;}
      if(BChqYsjjhX == true){BChqYsjjhX = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SGBECZTTHD
{ 
  void DEwIKxEkRz()
  { 
      bool FgbIzABIKd = false;
      bool GgilModYcq = false;
      bool sJNQiQXYeu = false;
      bool tZgpmXGAhm = false;
      bool AJYnXfReIo = false;
      bool aLJdwuSKZh = false;
      bool cbdPjfzszo = false;
      bool hcpopikYsC = false;
      bool zsQoThPDsq = false;
      bool EuazKYfIEq = false;
      bool gXeTGyUUnM = false;
      bool JtRWijLnuB = false;
      bool PwAIjlCpeR = false;
      bool DcUBVaNxhV = false;
      bool NuOghKcDQa = false;
      bool MrMDbkXWiT = false;
      bool udoWJgYQBH = false;
      bool VNFZmUgSZZ = false;
      bool JyZEEKPQZk = false;
      bool DpONZiGrWB = false;
      string skRJHtZuUC;
      string SPwtJYNCAo;
      string FhuChSwTsd;
      string MBAYLdkqnz;
      string heTtwzWgBW;
      string zRFCSLoPao;
      string yFCpoQOWie;
      string nVOAHaNAGh;
      string wOSVeuuyGA;
      string nVPyBcbcFe;
      string jzyVicCFtq;
      string xoIssilgLs;
      string VQQhqRFPzd;
      string ykubywQwlB;
      string rpDOCzNWFR;
      string puRLJTiPeQ;
      string XmFQsMsfuw;
      string SptJAeMCfr;
      string EhJgnCSiSd;
      string NBRUdFjmUp;
      if(skRJHtZuUC == jzyVicCFtq){FgbIzABIKd = true;}
      else if(jzyVicCFtq == skRJHtZuUC){gXeTGyUUnM = true;}
      if(SPwtJYNCAo == xoIssilgLs){GgilModYcq = true;}
      else if(xoIssilgLs == SPwtJYNCAo){JtRWijLnuB = true;}
      if(FhuChSwTsd == VQQhqRFPzd){sJNQiQXYeu = true;}
      else if(VQQhqRFPzd == FhuChSwTsd){PwAIjlCpeR = true;}
      if(MBAYLdkqnz == ykubywQwlB){tZgpmXGAhm = true;}
      else if(ykubywQwlB == MBAYLdkqnz){DcUBVaNxhV = true;}
      if(heTtwzWgBW == rpDOCzNWFR){AJYnXfReIo = true;}
      else if(rpDOCzNWFR == heTtwzWgBW){NuOghKcDQa = true;}
      if(zRFCSLoPao == puRLJTiPeQ){aLJdwuSKZh = true;}
      else if(puRLJTiPeQ == zRFCSLoPao){MrMDbkXWiT = true;}
      if(yFCpoQOWie == XmFQsMsfuw){cbdPjfzszo = true;}
      else if(XmFQsMsfuw == yFCpoQOWie){udoWJgYQBH = true;}
      if(nVOAHaNAGh == SptJAeMCfr){hcpopikYsC = true;}
      if(wOSVeuuyGA == EhJgnCSiSd){zsQoThPDsq = true;}
      if(nVPyBcbcFe == NBRUdFjmUp){EuazKYfIEq = true;}
      while(SptJAeMCfr == nVOAHaNAGh){VNFZmUgSZZ = true;}
      while(EhJgnCSiSd == EhJgnCSiSd){JyZEEKPQZk = true;}
      while(NBRUdFjmUp == NBRUdFjmUp){DpONZiGrWB = true;}
      if(FgbIzABIKd == true){FgbIzABIKd = false;}
      if(GgilModYcq == true){GgilModYcq = false;}
      if(sJNQiQXYeu == true){sJNQiQXYeu = false;}
      if(tZgpmXGAhm == true){tZgpmXGAhm = false;}
      if(AJYnXfReIo == true){AJYnXfReIo = false;}
      if(aLJdwuSKZh == true){aLJdwuSKZh = false;}
      if(cbdPjfzszo == true){cbdPjfzszo = false;}
      if(hcpopikYsC == true){hcpopikYsC = false;}
      if(zsQoThPDsq == true){zsQoThPDsq = false;}
      if(EuazKYfIEq == true){EuazKYfIEq = false;}
      if(gXeTGyUUnM == true){gXeTGyUUnM = false;}
      if(JtRWijLnuB == true){JtRWijLnuB = false;}
      if(PwAIjlCpeR == true){PwAIjlCpeR = false;}
      if(DcUBVaNxhV == true){DcUBVaNxhV = false;}
      if(NuOghKcDQa == true){NuOghKcDQa = false;}
      if(MrMDbkXWiT == true){MrMDbkXWiT = false;}
      if(udoWJgYQBH == true){udoWJgYQBH = false;}
      if(VNFZmUgSZZ == true){VNFZmUgSZZ = false;}
      if(JyZEEKPQZk == true){JyZEEKPQZk = false;}
      if(DpONZiGrWB == true){DpONZiGrWB = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BIDTICOBYG
{ 
  void OOWKwdelGY()
  { 
      bool EaSJjJlURp = false;
      bool wEAfFTVtKV = false;
      bool BwwXoYufKg = false;
      bool atyFCduKuL = false;
      bool tlBKpeKfkh = false;
      bool AYmfXHeuTL = false;
      bool NFKjPCFtca = false;
      bool YLzybPulgK = false;
      bool NPubgCGSgu = false;
      bool zzpixnbQum = false;
      bool ZPCJJoYoEH = false;
      bool LsNJxxKhDM = false;
      bool wlNhbYVPBm = false;
      bool ndptjxNlRq = false;
      bool cROYjFlflA = false;
      bool csNhqmCUgj = false;
      bool TImQTgCTZz = false;
      bool jYQLjdXnxW = false;
      bool modBJuQICx = false;
      bool usEhXTdxFC = false;
      string WzqpfeydEM;
      string EsSntkBICK;
      string UGZxPQUaOu;
      string tsaOuwAqKF;
      string xxnCkcQlMY;
      string jpXeQPZtZO;
      string JWOISKREsx;
      string jJywMXSWLm;
      string AiOpLDVuye;
      string WGRwuZhejZ;
      string KLrOnaraRr;
      string wChmNaIYxO;
      string oVGNacrCAm;
      string pnmlbopABj;
      string LtcFhTXdhU;
      string RJgUkoFemK;
      string hoaeCIPwKY;
      string jIWMDtrBeH;
      string FOTfUgGSAm;
      string iKMpCVTyCZ;
      if(WzqpfeydEM == KLrOnaraRr){EaSJjJlURp = true;}
      else if(KLrOnaraRr == WzqpfeydEM){ZPCJJoYoEH = true;}
      if(EsSntkBICK == wChmNaIYxO){wEAfFTVtKV = true;}
      else if(wChmNaIYxO == EsSntkBICK){LsNJxxKhDM = true;}
      if(UGZxPQUaOu == oVGNacrCAm){BwwXoYufKg = true;}
      else if(oVGNacrCAm == UGZxPQUaOu){wlNhbYVPBm = true;}
      if(tsaOuwAqKF == pnmlbopABj){atyFCduKuL = true;}
      else if(pnmlbopABj == tsaOuwAqKF){ndptjxNlRq = true;}
      if(xxnCkcQlMY == LtcFhTXdhU){tlBKpeKfkh = true;}
      else if(LtcFhTXdhU == xxnCkcQlMY){cROYjFlflA = true;}
      if(jpXeQPZtZO == RJgUkoFemK){AYmfXHeuTL = true;}
      else if(RJgUkoFemK == jpXeQPZtZO){csNhqmCUgj = true;}
      if(JWOISKREsx == hoaeCIPwKY){NFKjPCFtca = true;}
      else if(hoaeCIPwKY == JWOISKREsx){TImQTgCTZz = true;}
      if(jJywMXSWLm == jIWMDtrBeH){YLzybPulgK = true;}
      if(AiOpLDVuye == FOTfUgGSAm){NPubgCGSgu = true;}
      if(WGRwuZhejZ == iKMpCVTyCZ){zzpixnbQum = true;}
      while(jIWMDtrBeH == jJywMXSWLm){jYQLjdXnxW = true;}
      while(FOTfUgGSAm == FOTfUgGSAm){modBJuQICx = true;}
      while(iKMpCVTyCZ == iKMpCVTyCZ){usEhXTdxFC = true;}
      if(EaSJjJlURp == true){EaSJjJlURp = false;}
      if(wEAfFTVtKV == true){wEAfFTVtKV = false;}
      if(BwwXoYufKg == true){BwwXoYufKg = false;}
      if(atyFCduKuL == true){atyFCduKuL = false;}
      if(tlBKpeKfkh == true){tlBKpeKfkh = false;}
      if(AYmfXHeuTL == true){AYmfXHeuTL = false;}
      if(NFKjPCFtca == true){NFKjPCFtca = false;}
      if(YLzybPulgK == true){YLzybPulgK = false;}
      if(NPubgCGSgu == true){NPubgCGSgu = false;}
      if(zzpixnbQum == true){zzpixnbQum = false;}
      if(ZPCJJoYoEH == true){ZPCJJoYoEH = false;}
      if(LsNJxxKhDM == true){LsNJxxKhDM = false;}
      if(wlNhbYVPBm == true){wlNhbYVPBm = false;}
      if(ndptjxNlRq == true){ndptjxNlRq = false;}
      if(cROYjFlflA == true){cROYjFlflA = false;}
      if(csNhqmCUgj == true){csNhqmCUgj = false;}
      if(TImQTgCTZz == true){TImQTgCTZz = false;}
      if(jYQLjdXnxW == true){jYQLjdXnxW = false;}
      if(modBJuQICx == true){modBJuQICx = false;}
      if(usEhXTdxFC == true){usEhXTdxFC = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BHBARUQAFW
{ 
  void LVkxYnNiaP()
  { 
      bool cGkmyqdJtN = false;
      bool HbxnzfVZCH = false;
      bool PwcdqgyMha = false;
      bool iojlNfLhcd = false;
      bool UdYOHCtWJI = false;
      bool wCNoXfcHuR = false;
      bool rRbSeSqKZS = false;
      bool UmwXlkWOVn = false;
      bool lbDYefauOh = false;
      bool CEmKkDBGjo = false;
      bool LjxpmIFoCl = false;
      bool JOkGfQNDnz = false;
      bool qwurPQAlmi = false;
      bool jQdgcBeMlV = false;
      bool ZlrcXmSfeU = false;
      bool pyRkxPnEZW = false;
      bool ITCTBrkDdd = false;
      bool lAQRZEDSTb = false;
      bool xTGSrlysiN = false;
      bool NZmPNFjYae = false;
      string hnQXWRYkQb;
      string FWfIpOlMRU;
      string cURsRbYVnG;
      string wnuhcnrIOF;
      string BMeUdhrURH;
      string iSLNWNVWGt;
      string sCxKeQaxfC;
      string VdyLTPoxOi;
      string jDEgWcchLP;
      string RUCTMyPUoN;
      string GunPlFUzsG;
      string XYIWKoyPnB;
      string nslQjAbdyV;
      string zUHBcnPotR;
      string IcnEqsMeIX;
      string AAfnYMJWel;
      string cpAXHDQGtV;
      string YIVVsLFgZq;
      string mwUaUznTcP;
      string erKInRbPxm;
      if(hnQXWRYkQb == GunPlFUzsG){cGkmyqdJtN = true;}
      else if(GunPlFUzsG == hnQXWRYkQb){LjxpmIFoCl = true;}
      if(FWfIpOlMRU == XYIWKoyPnB){HbxnzfVZCH = true;}
      else if(XYIWKoyPnB == FWfIpOlMRU){JOkGfQNDnz = true;}
      if(cURsRbYVnG == nslQjAbdyV){PwcdqgyMha = true;}
      else if(nslQjAbdyV == cURsRbYVnG){qwurPQAlmi = true;}
      if(wnuhcnrIOF == zUHBcnPotR){iojlNfLhcd = true;}
      else if(zUHBcnPotR == wnuhcnrIOF){jQdgcBeMlV = true;}
      if(BMeUdhrURH == IcnEqsMeIX){UdYOHCtWJI = true;}
      else if(IcnEqsMeIX == BMeUdhrURH){ZlrcXmSfeU = true;}
      if(iSLNWNVWGt == AAfnYMJWel){wCNoXfcHuR = true;}
      else if(AAfnYMJWel == iSLNWNVWGt){pyRkxPnEZW = true;}
      if(sCxKeQaxfC == cpAXHDQGtV){rRbSeSqKZS = true;}
      else if(cpAXHDQGtV == sCxKeQaxfC){ITCTBrkDdd = true;}
      if(VdyLTPoxOi == YIVVsLFgZq){UmwXlkWOVn = true;}
      if(jDEgWcchLP == mwUaUznTcP){lbDYefauOh = true;}
      if(RUCTMyPUoN == erKInRbPxm){CEmKkDBGjo = true;}
      while(YIVVsLFgZq == VdyLTPoxOi){lAQRZEDSTb = true;}
      while(mwUaUznTcP == mwUaUznTcP){xTGSrlysiN = true;}
      while(erKInRbPxm == erKInRbPxm){NZmPNFjYae = true;}
      if(cGkmyqdJtN == true){cGkmyqdJtN = false;}
      if(HbxnzfVZCH == true){HbxnzfVZCH = false;}
      if(PwcdqgyMha == true){PwcdqgyMha = false;}
      if(iojlNfLhcd == true){iojlNfLhcd = false;}
      if(UdYOHCtWJI == true){UdYOHCtWJI = false;}
      if(wCNoXfcHuR == true){wCNoXfcHuR = false;}
      if(rRbSeSqKZS == true){rRbSeSqKZS = false;}
      if(UmwXlkWOVn == true){UmwXlkWOVn = false;}
      if(lbDYefauOh == true){lbDYefauOh = false;}
      if(CEmKkDBGjo == true){CEmKkDBGjo = false;}
      if(LjxpmIFoCl == true){LjxpmIFoCl = false;}
      if(JOkGfQNDnz == true){JOkGfQNDnz = false;}
      if(qwurPQAlmi == true){qwurPQAlmi = false;}
      if(jQdgcBeMlV == true){jQdgcBeMlV = false;}
      if(ZlrcXmSfeU == true){ZlrcXmSfeU = false;}
      if(pyRkxPnEZW == true){pyRkxPnEZW = false;}
      if(ITCTBrkDdd == true){ITCTBrkDdd = false;}
      if(lAQRZEDSTb == true){lAQRZEDSTb = false;}
      if(xTGSrlysiN == true){xTGSrlysiN = false;}
      if(NZmPNFjYae == true){NZmPNFjYae = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QKOXPUFKAI
{ 
  void NHAwUwWQAO()
  { 
      bool HgWONkikZQ = false;
      bool UnRUUlqEmP = false;
      bool MtOBEzSGuJ = false;
      bool PQtORzOnMJ = false;
      bool DdcCxjguSb = false;
      bool KwYuXNRVKN = false;
      bool SbpPBEtCDU = false;
      bool XCqdMiQMDU = false;
      bool RWnTyTqypE = false;
      bool gsczqbmMAs = false;
      bool odrxDHEuio = false;
      bool wFRowunVQh = false;
      bool MuLzneZrmk = false;
      bool uDLKemepPq = false;
      bool OwPBHaamDu = false;
      bool mRwSgsRrIY = false;
      bool hLIysrsHKI = false;
      bool QxGZSQfKqM = false;
      bool DloqxdHrck = false;
      bool UlgqICmRHO = false;
      string WkJogEPudF;
      string VqdEdAxECh;
      string HgSzVBqgas;
      string LbaapIHtTo;
      string AfclWkRtuo;
      string EZegtpRKSe;
      string MhTkssQCNU;
      string rDjNhaDgcj;
      string ixlknlxsgA;
      string pwanbRLrrc;
      string UPfZwBpRyS;
      string mkhiwPwiwH;
      string wXUNeuculQ;
      string PKnzPSRMnm;
      string wftONFOqrL;
      string VcukgxuRuB;
      string XtehCbxhTc;
      string yjtJLPcGAZ;
      string flDZulOCSR;
      string XzQmOBAMLk;
      if(WkJogEPudF == UPfZwBpRyS){HgWONkikZQ = true;}
      else if(UPfZwBpRyS == WkJogEPudF){odrxDHEuio = true;}
      if(VqdEdAxECh == mkhiwPwiwH){UnRUUlqEmP = true;}
      else if(mkhiwPwiwH == VqdEdAxECh){wFRowunVQh = true;}
      if(HgSzVBqgas == wXUNeuculQ){MtOBEzSGuJ = true;}
      else if(wXUNeuculQ == HgSzVBqgas){MuLzneZrmk = true;}
      if(LbaapIHtTo == PKnzPSRMnm){PQtORzOnMJ = true;}
      else if(PKnzPSRMnm == LbaapIHtTo){uDLKemepPq = true;}
      if(AfclWkRtuo == wftONFOqrL){DdcCxjguSb = true;}
      else if(wftONFOqrL == AfclWkRtuo){OwPBHaamDu = true;}
      if(EZegtpRKSe == VcukgxuRuB){KwYuXNRVKN = true;}
      else if(VcukgxuRuB == EZegtpRKSe){mRwSgsRrIY = true;}
      if(MhTkssQCNU == XtehCbxhTc){SbpPBEtCDU = true;}
      else if(XtehCbxhTc == MhTkssQCNU){hLIysrsHKI = true;}
      if(rDjNhaDgcj == yjtJLPcGAZ){XCqdMiQMDU = true;}
      if(ixlknlxsgA == flDZulOCSR){RWnTyTqypE = true;}
      if(pwanbRLrrc == XzQmOBAMLk){gsczqbmMAs = true;}
      while(yjtJLPcGAZ == rDjNhaDgcj){QxGZSQfKqM = true;}
      while(flDZulOCSR == flDZulOCSR){DloqxdHrck = true;}
      while(XzQmOBAMLk == XzQmOBAMLk){UlgqICmRHO = true;}
      if(HgWONkikZQ == true){HgWONkikZQ = false;}
      if(UnRUUlqEmP == true){UnRUUlqEmP = false;}
      if(MtOBEzSGuJ == true){MtOBEzSGuJ = false;}
      if(PQtORzOnMJ == true){PQtORzOnMJ = false;}
      if(DdcCxjguSb == true){DdcCxjguSb = false;}
      if(KwYuXNRVKN == true){KwYuXNRVKN = false;}
      if(SbpPBEtCDU == true){SbpPBEtCDU = false;}
      if(XCqdMiQMDU == true){XCqdMiQMDU = false;}
      if(RWnTyTqypE == true){RWnTyTqypE = false;}
      if(gsczqbmMAs == true){gsczqbmMAs = false;}
      if(odrxDHEuio == true){odrxDHEuio = false;}
      if(wFRowunVQh == true){wFRowunVQh = false;}
      if(MuLzneZrmk == true){MuLzneZrmk = false;}
      if(uDLKemepPq == true){uDLKemepPq = false;}
      if(OwPBHaamDu == true){OwPBHaamDu = false;}
      if(mRwSgsRrIY == true){mRwSgsRrIY = false;}
      if(hLIysrsHKI == true){hLIysrsHKI = false;}
      if(QxGZSQfKqM == true){QxGZSQfKqM = false;}
      if(DloqxdHrck == true){DloqxdHrck = false;}
      if(UlgqICmRHO == true){UlgqICmRHO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class QKRRHTRFOG
{ 
  void yDnKIKxsyl()
  { 
      bool TWCARoUCRL = false;
      bool XfGWIKALWe = false;
      bool CbBjVVxbjf = false;
      bool bXjexAcezX = false;
      bool TmAzOeeMKX = false;
      bool LcFbYqUSeu = false;
      bool rrSljzubYH = false;
      bool hVMzKObCUq = false;
      bool qkQMUySHFF = false;
      bool eKADfaUOPO = false;
      bool iCmCooPGRJ = false;
      bool PSxmEHbFRk = false;
      bool gmWTbWVISi = false;
      bool tMlFInxOWe = false;
      bool iLpuoMMMyY = false;
      bool zPxegueSZS = false;
      bool THmhPCfrdQ = false;
      bool JMhlCVwbdP = false;
      bool JIJtidPHXM = false;
      bool IenELaXIel = false;
      string jgYSFcRxpI;
      string xHDKyaDiDX;
      string HClajXfZQc;
      string ldKmIgipbe;
      string PqbPcjDqke;
      string IArbngtyhg;
      string xqfMVSxPCR;
      string rMHYHPaWxz;
      string ToolUkkhza;
      string lkIthXKOtJ;
      string mmoSmusTUY;
      string FXAiMHCtrj;
      string hOQaeHEMqi;
      string FMcmLgzVyY;
      string rRNZUOjYgZ;
      string VQVJbYWNme;
      string pfckTsuNWf;
      string IfuSrQApGQ;
      string WefQNopSLm;
      string KliThLncGb;
      if(jgYSFcRxpI == mmoSmusTUY){TWCARoUCRL = true;}
      else if(mmoSmusTUY == jgYSFcRxpI){iCmCooPGRJ = true;}
      if(xHDKyaDiDX == FXAiMHCtrj){XfGWIKALWe = true;}
      else if(FXAiMHCtrj == xHDKyaDiDX){PSxmEHbFRk = true;}
      if(HClajXfZQc == hOQaeHEMqi){CbBjVVxbjf = true;}
      else if(hOQaeHEMqi == HClajXfZQc){gmWTbWVISi = true;}
      if(ldKmIgipbe == FMcmLgzVyY){bXjexAcezX = true;}
      else if(FMcmLgzVyY == ldKmIgipbe){tMlFInxOWe = true;}
      if(PqbPcjDqke == rRNZUOjYgZ){TmAzOeeMKX = true;}
      else if(rRNZUOjYgZ == PqbPcjDqke){iLpuoMMMyY = true;}
      if(IArbngtyhg == VQVJbYWNme){LcFbYqUSeu = true;}
      else if(VQVJbYWNme == IArbngtyhg){zPxegueSZS = true;}
      if(xqfMVSxPCR == pfckTsuNWf){rrSljzubYH = true;}
      else if(pfckTsuNWf == xqfMVSxPCR){THmhPCfrdQ = true;}
      if(rMHYHPaWxz == IfuSrQApGQ){hVMzKObCUq = true;}
      if(ToolUkkhza == WefQNopSLm){qkQMUySHFF = true;}
      if(lkIthXKOtJ == KliThLncGb){eKADfaUOPO = true;}
      while(IfuSrQApGQ == rMHYHPaWxz){JMhlCVwbdP = true;}
      while(WefQNopSLm == WefQNopSLm){JIJtidPHXM = true;}
      while(KliThLncGb == KliThLncGb){IenELaXIel = true;}
      if(TWCARoUCRL == true){TWCARoUCRL = false;}
      if(XfGWIKALWe == true){XfGWIKALWe = false;}
      if(CbBjVVxbjf == true){CbBjVVxbjf = false;}
      if(bXjexAcezX == true){bXjexAcezX = false;}
      if(TmAzOeeMKX == true){TmAzOeeMKX = false;}
      if(LcFbYqUSeu == true){LcFbYqUSeu = false;}
      if(rrSljzubYH == true){rrSljzubYH = false;}
      if(hVMzKObCUq == true){hVMzKObCUq = false;}
      if(qkQMUySHFF == true){qkQMUySHFF = false;}
      if(eKADfaUOPO == true){eKADfaUOPO = false;}
      if(iCmCooPGRJ == true){iCmCooPGRJ = false;}
      if(PSxmEHbFRk == true){PSxmEHbFRk = false;}
      if(gmWTbWVISi == true){gmWTbWVISi = false;}
      if(tMlFInxOWe == true){tMlFInxOWe = false;}
      if(iLpuoMMMyY == true){iLpuoMMMyY = false;}
      if(zPxegueSZS == true){zPxegueSZS = false;}
      if(THmhPCfrdQ == true){THmhPCfrdQ = false;}
      if(JMhlCVwbdP == true){JMhlCVwbdP = false;}
      if(JIJtidPHXM == true){JIJtidPHXM = false;}
      if(IenELaXIel == true){IenELaXIel = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class EWFMBSAMDW
{ 
  void howEIpEHGS()
  { 
      bool DBUKJEwmEP = false;
      bool yhKQNPmBhr = false;
      bool wlZfwXuchO = false;
      bool luLcgXuHps = false;
      bool IqwWIrPhKK = false;
      bool GAUqERbnNk = false;
      bool JSFpdDVgjp = false;
      bool TeKojEoDXE = false;
      bool IMRWIlZUYP = false;
      bool wHtEjolGsF = false;
      bool WhyUpaofHA = false;
      bool bYQIcxAigq = false;
      bool dAIUtMkFFt = false;
      bool RiJRhZNwfg = false;
      bool wsNXmWqkal = false;
      bool nDYBbVZdad = false;
      bool piaMhsOyZD = false;
      bool iDCAujmjgW = false;
      bool UzeSGOKHoM = false;
      bool AjVCpMSAps = false;
      string RxWnGTDZTT;
      string qdFHzSBtSM;
      string ebLfLWucXq;
      string KqhwUdKbor;
      string VOoctbOiyj;
      string dIhUPpkpbi;
      string YaBcIzwIlU;
      string IYFaqPmilj;
      string ILenMBJMfe;
      string sTAGTtMLTt;
      string fFwcbFDHcA;
      string CCErZMuNPG;
      string pibHbnYyGw;
      string DaOqEDOBLH;
      string yCjfcqOIgG;
      string zywNExcoBm;
      string fEAfwamNnW;
      string xtCIInyuek;
      string ZQRzxEeBmH;
      string ZZzYasBDiB;
      if(RxWnGTDZTT == fFwcbFDHcA){DBUKJEwmEP = true;}
      else if(fFwcbFDHcA == RxWnGTDZTT){WhyUpaofHA = true;}
      if(qdFHzSBtSM == CCErZMuNPG){yhKQNPmBhr = true;}
      else if(CCErZMuNPG == qdFHzSBtSM){bYQIcxAigq = true;}
      if(ebLfLWucXq == pibHbnYyGw){wlZfwXuchO = true;}
      else if(pibHbnYyGw == ebLfLWucXq){dAIUtMkFFt = true;}
      if(KqhwUdKbor == DaOqEDOBLH){luLcgXuHps = true;}
      else if(DaOqEDOBLH == KqhwUdKbor){RiJRhZNwfg = true;}
      if(VOoctbOiyj == yCjfcqOIgG){IqwWIrPhKK = true;}
      else if(yCjfcqOIgG == VOoctbOiyj){wsNXmWqkal = true;}
      if(dIhUPpkpbi == zywNExcoBm){GAUqERbnNk = true;}
      else if(zywNExcoBm == dIhUPpkpbi){nDYBbVZdad = true;}
      if(YaBcIzwIlU == fEAfwamNnW){JSFpdDVgjp = true;}
      else if(fEAfwamNnW == YaBcIzwIlU){piaMhsOyZD = true;}
      if(IYFaqPmilj == xtCIInyuek){TeKojEoDXE = true;}
      if(ILenMBJMfe == ZQRzxEeBmH){IMRWIlZUYP = true;}
      if(sTAGTtMLTt == ZZzYasBDiB){wHtEjolGsF = true;}
      while(xtCIInyuek == IYFaqPmilj){iDCAujmjgW = true;}
      while(ZQRzxEeBmH == ZQRzxEeBmH){UzeSGOKHoM = true;}
      while(ZZzYasBDiB == ZZzYasBDiB){AjVCpMSAps = true;}
      if(DBUKJEwmEP == true){DBUKJEwmEP = false;}
      if(yhKQNPmBhr == true){yhKQNPmBhr = false;}
      if(wlZfwXuchO == true){wlZfwXuchO = false;}
      if(luLcgXuHps == true){luLcgXuHps = false;}
      if(IqwWIrPhKK == true){IqwWIrPhKK = false;}
      if(GAUqERbnNk == true){GAUqERbnNk = false;}
      if(JSFpdDVgjp == true){JSFpdDVgjp = false;}
      if(TeKojEoDXE == true){TeKojEoDXE = false;}
      if(IMRWIlZUYP == true){IMRWIlZUYP = false;}
      if(wHtEjolGsF == true){wHtEjolGsF = false;}
      if(WhyUpaofHA == true){WhyUpaofHA = false;}
      if(bYQIcxAigq == true){bYQIcxAigq = false;}
      if(dAIUtMkFFt == true){dAIUtMkFFt = false;}
      if(RiJRhZNwfg == true){RiJRhZNwfg = false;}
      if(wsNXmWqkal == true){wsNXmWqkal = false;}
      if(nDYBbVZdad == true){nDYBbVZdad = false;}
      if(piaMhsOyZD == true){piaMhsOyZD = false;}
      if(iDCAujmjgW == true){iDCAujmjgW = false;}
      if(UzeSGOKHoM == true){UzeSGOKHoM = false;}
      if(AjVCpMSAps == true){AjVCpMSAps = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MHFDZRCAHO
{ 
  void chTzXrBEcP()
  { 
      bool ZeBGKaRMyI = false;
      bool arkApbuOSU = false;
      bool JffaQCExwc = false;
      bool qmoRiRDpmd = false;
      bool ISabtCilIH = false;
      bool mcAYJtyapH = false;
      bool VTQVRnCTQX = false;
      bool YFanLpEEfN = false;
      bool TSoooNlQIo = false;
      bool SczeInCFKG = false;
      bool zPVjhgBLNS = false;
      bool ottbnCebfD = false;
      bool BlLaEwGwfW = false;
      bool nztZOEwEGI = false;
      bool LaCoAYhXqF = false;
      bool JZYGEMEuel = false;
      bool aFepdpzTMj = false;
      bool FZUXqaiVaf = false;
      bool NhEaNOZmVB = false;
      bool zBZUgGuMKJ = false;
      string JWkaXFykYS;
      string RnwyJedNCT;
      string koygsUhMjf;
      string ybGymuteBV;
      string qyfQWLyaOQ;
      string gRaWXUXEgD;
      string WkBsTogyLN;
      string jzLfcmJhJh;
      string VHxSygAjZS;
      string WpxyyFxmxg;
      string ezPBghFutk;
      string KptzgPnhum;
      string owXrqsBNgW;
      string oWjcDVrwlP;
      string aFpoHgwxRi;
      string ROqKrhbWJm;
      string frNWunGQwK;
      string XqweLiGikJ;
      string xEwyEaeZjV;
      string tUlyiirSsS;
      if(JWkaXFykYS == ezPBghFutk){ZeBGKaRMyI = true;}
      else if(ezPBghFutk == JWkaXFykYS){zPVjhgBLNS = true;}
      if(RnwyJedNCT == KptzgPnhum){arkApbuOSU = true;}
      else if(KptzgPnhum == RnwyJedNCT){ottbnCebfD = true;}
      if(koygsUhMjf == owXrqsBNgW){JffaQCExwc = true;}
      else if(owXrqsBNgW == koygsUhMjf){BlLaEwGwfW = true;}
      if(ybGymuteBV == oWjcDVrwlP){qmoRiRDpmd = true;}
      else if(oWjcDVrwlP == ybGymuteBV){nztZOEwEGI = true;}
      if(qyfQWLyaOQ == aFpoHgwxRi){ISabtCilIH = true;}
      else if(aFpoHgwxRi == qyfQWLyaOQ){LaCoAYhXqF = true;}
      if(gRaWXUXEgD == ROqKrhbWJm){mcAYJtyapH = true;}
      else if(ROqKrhbWJm == gRaWXUXEgD){JZYGEMEuel = true;}
      if(WkBsTogyLN == frNWunGQwK){VTQVRnCTQX = true;}
      else if(frNWunGQwK == WkBsTogyLN){aFepdpzTMj = true;}
      if(jzLfcmJhJh == XqweLiGikJ){YFanLpEEfN = true;}
      if(VHxSygAjZS == xEwyEaeZjV){TSoooNlQIo = true;}
      if(WpxyyFxmxg == tUlyiirSsS){SczeInCFKG = true;}
      while(XqweLiGikJ == jzLfcmJhJh){FZUXqaiVaf = true;}
      while(xEwyEaeZjV == xEwyEaeZjV){NhEaNOZmVB = true;}
      while(tUlyiirSsS == tUlyiirSsS){zBZUgGuMKJ = true;}
      if(ZeBGKaRMyI == true){ZeBGKaRMyI = false;}
      if(arkApbuOSU == true){arkApbuOSU = false;}
      if(JffaQCExwc == true){JffaQCExwc = false;}
      if(qmoRiRDpmd == true){qmoRiRDpmd = false;}
      if(ISabtCilIH == true){ISabtCilIH = false;}
      if(mcAYJtyapH == true){mcAYJtyapH = false;}
      if(VTQVRnCTQX == true){VTQVRnCTQX = false;}
      if(YFanLpEEfN == true){YFanLpEEfN = false;}
      if(TSoooNlQIo == true){TSoooNlQIo = false;}
      if(SczeInCFKG == true){SczeInCFKG = false;}
      if(zPVjhgBLNS == true){zPVjhgBLNS = false;}
      if(ottbnCebfD == true){ottbnCebfD = false;}
      if(BlLaEwGwfW == true){BlLaEwGwfW = false;}
      if(nztZOEwEGI == true){nztZOEwEGI = false;}
      if(LaCoAYhXqF == true){LaCoAYhXqF = false;}
      if(JZYGEMEuel == true){JZYGEMEuel = false;}
      if(aFepdpzTMj == true){aFepdpzTMj = false;}
      if(FZUXqaiVaf == true){FZUXqaiVaf = false;}
      if(NhEaNOZmVB == true){NhEaNOZmVB = false;}
      if(zBZUgGuMKJ == true){zBZUgGuMKJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GBMSQTBAFA
{ 
  void bMtTLNTGFt()
  { 
      bool guFTxtMaPA = false;
      bool iJueLXrboJ = false;
      bool bgwHUDdiYl = false;
      bool LUMrHapSio = false;
      bool ZtXLhngeJx = false;
      bool sBzXrAzxyt = false;
      bool PcBcoOxDWj = false;
      bool gYrAihJkYf = false;
      bool VyYbsJUozH = false;
      bool asUoYwrtWT = false;
      bool qdIYQTJznJ = false;
      bool pNXRYREZje = false;
      bool sHDOJmEDam = false;
      bool pOOForpzLY = false;
      bool zUApQMVCQH = false;
      bool rRKqZiZsPc = false;
      bool PGYASbzScf = false;
      bool aEOKfYHMyw = false;
      bool QCmSuYDXeF = false;
      bool SxihJswzWo = false;
      string owpiKZwWhX;
      string MqAQqYQxiU;
      string OKtPXHnpFS;
      string LeMAiAlaMd;
      string EoSoyubSBt;
      string joYHYeBZLC;
      string kFRPLhdzhp;
      string mojEQZrFQb;
      string fUnKhVGFbE;
      string yMOKsyWCdQ;
      string RbkLHtFTqy;
      string mwIjWPQGho;
      string aYXdqZaZKR;
      string CobAKDjcCn;
      string QlOcRCDemd;
      string hQUnorEHTh;
      string TeIYuxUHZT;
      string fmDsXoJtLo;
      string WwlgjlUmLR;
      string ybJUUlcogT;
      if(owpiKZwWhX == RbkLHtFTqy){guFTxtMaPA = true;}
      else if(RbkLHtFTqy == owpiKZwWhX){qdIYQTJznJ = true;}
      if(MqAQqYQxiU == mwIjWPQGho){iJueLXrboJ = true;}
      else if(mwIjWPQGho == MqAQqYQxiU){pNXRYREZje = true;}
      if(OKtPXHnpFS == aYXdqZaZKR){bgwHUDdiYl = true;}
      else if(aYXdqZaZKR == OKtPXHnpFS){sHDOJmEDam = true;}
      if(LeMAiAlaMd == CobAKDjcCn){LUMrHapSio = true;}
      else if(CobAKDjcCn == LeMAiAlaMd){pOOForpzLY = true;}
      if(EoSoyubSBt == QlOcRCDemd){ZtXLhngeJx = true;}
      else if(QlOcRCDemd == EoSoyubSBt){zUApQMVCQH = true;}
      if(joYHYeBZLC == hQUnorEHTh){sBzXrAzxyt = true;}
      else if(hQUnorEHTh == joYHYeBZLC){rRKqZiZsPc = true;}
      if(kFRPLhdzhp == TeIYuxUHZT){PcBcoOxDWj = true;}
      else if(TeIYuxUHZT == kFRPLhdzhp){PGYASbzScf = true;}
      if(mojEQZrFQb == fmDsXoJtLo){gYrAihJkYf = true;}
      if(fUnKhVGFbE == WwlgjlUmLR){VyYbsJUozH = true;}
      if(yMOKsyWCdQ == ybJUUlcogT){asUoYwrtWT = true;}
      while(fmDsXoJtLo == mojEQZrFQb){aEOKfYHMyw = true;}
      while(WwlgjlUmLR == WwlgjlUmLR){QCmSuYDXeF = true;}
      while(ybJUUlcogT == ybJUUlcogT){SxihJswzWo = true;}
      if(guFTxtMaPA == true){guFTxtMaPA = false;}
      if(iJueLXrboJ == true){iJueLXrboJ = false;}
      if(bgwHUDdiYl == true){bgwHUDdiYl = false;}
      if(LUMrHapSio == true){LUMrHapSio = false;}
      if(ZtXLhngeJx == true){ZtXLhngeJx = false;}
      if(sBzXrAzxyt == true){sBzXrAzxyt = false;}
      if(PcBcoOxDWj == true){PcBcoOxDWj = false;}
      if(gYrAihJkYf == true){gYrAihJkYf = false;}
      if(VyYbsJUozH == true){VyYbsJUozH = false;}
      if(asUoYwrtWT == true){asUoYwrtWT = false;}
      if(qdIYQTJznJ == true){qdIYQTJznJ = false;}
      if(pNXRYREZje == true){pNXRYREZje = false;}
      if(sHDOJmEDam == true){sHDOJmEDam = false;}
      if(pOOForpzLY == true){pOOForpzLY = false;}
      if(zUApQMVCQH == true){zUApQMVCQH = false;}
      if(rRKqZiZsPc == true){rRKqZiZsPc = false;}
      if(PGYASbzScf == true){PGYASbzScf = false;}
      if(aEOKfYHMyw == true){aEOKfYHMyw = false;}
      if(QCmSuYDXeF == true){QCmSuYDXeF = false;}
      if(SxihJswzWo == true){SxihJswzWo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RDCEPKNKHG
{ 
  void huKroGwdii()
  { 
      bool ddRsWUoENq = false;
      bool zLrozYAMUA = false;
      bool bRmWXtlLxI = false;
      bool xgSTFcymmH = false;
      bool UPcUkVOGRJ = false;
      bool IjtlRMptUb = false;
      bool hdHFtSrXjV = false;
      bool ZMZMfMnSlC = false;
      bool fftnAMmgCs = false;
      bool mESKZqHXaS = false;
      bool dpWwiuKgFE = false;
      bool XXISuCiSEg = false;
      bool eLVpiVtOmo = false;
      bool EQqTYoJxEF = false;
      bool CFaxVOwzPW = false;
      bool HnStmRTfMN = false;
      bool AutQzHfntF = false;
      bool slOzeZjYUT = false;
      bool YtzPhQxxfG = false;
      bool VckpEGLJxN = false;
      string bGNRFKepqN;
      string bKSeFatJqS;
      string miNXqgZIWl;
      string AtJajNPoaW;
      string doqCeFMEoE;
      string juXSmrroaX;
      string jWTENhAqur;
      string LEhTnyqLsl;
      string TVSRCEVqmq;
      string UZOClHUmnw;
      string EVJxseNDdo;
      string BgwoENDBCI;
      string LIEDXlpehg;
      string cEWwgyMUYW;
      string qSMypndZyX;
      string DSjfjwQFYH;
      string ohVxLwJzHF;
      string BhceRswFNj;
      string HQgJcKTqLk;
      string gGwRhIwOms;
      if(bGNRFKepqN == EVJxseNDdo){ddRsWUoENq = true;}
      else if(EVJxseNDdo == bGNRFKepqN){dpWwiuKgFE = true;}
      if(bKSeFatJqS == BgwoENDBCI){zLrozYAMUA = true;}
      else if(BgwoENDBCI == bKSeFatJqS){XXISuCiSEg = true;}
      if(miNXqgZIWl == LIEDXlpehg){bRmWXtlLxI = true;}
      else if(LIEDXlpehg == miNXqgZIWl){eLVpiVtOmo = true;}
      if(AtJajNPoaW == cEWwgyMUYW){xgSTFcymmH = true;}
      else if(cEWwgyMUYW == AtJajNPoaW){EQqTYoJxEF = true;}
      if(doqCeFMEoE == qSMypndZyX){UPcUkVOGRJ = true;}
      else if(qSMypndZyX == doqCeFMEoE){CFaxVOwzPW = true;}
      if(juXSmrroaX == DSjfjwQFYH){IjtlRMptUb = true;}
      else if(DSjfjwQFYH == juXSmrroaX){HnStmRTfMN = true;}
      if(jWTENhAqur == ohVxLwJzHF){hdHFtSrXjV = true;}
      else if(ohVxLwJzHF == jWTENhAqur){AutQzHfntF = true;}
      if(LEhTnyqLsl == BhceRswFNj){ZMZMfMnSlC = true;}
      if(TVSRCEVqmq == HQgJcKTqLk){fftnAMmgCs = true;}
      if(UZOClHUmnw == gGwRhIwOms){mESKZqHXaS = true;}
      while(BhceRswFNj == LEhTnyqLsl){slOzeZjYUT = true;}
      while(HQgJcKTqLk == HQgJcKTqLk){YtzPhQxxfG = true;}
      while(gGwRhIwOms == gGwRhIwOms){VckpEGLJxN = true;}
      if(ddRsWUoENq == true){ddRsWUoENq = false;}
      if(zLrozYAMUA == true){zLrozYAMUA = false;}
      if(bRmWXtlLxI == true){bRmWXtlLxI = false;}
      if(xgSTFcymmH == true){xgSTFcymmH = false;}
      if(UPcUkVOGRJ == true){UPcUkVOGRJ = false;}
      if(IjtlRMptUb == true){IjtlRMptUb = false;}
      if(hdHFtSrXjV == true){hdHFtSrXjV = false;}
      if(ZMZMfMnSlC == true){ZMZMfMnSlC = false;}
      if(fftnAMmgCs == true){fftnAMmgCs = false;}
      if(mESKZqHXaS == true){mESKZqHXaS = false;}
      if(dpWwiuKgFE == true){dpWwiuKgFE = false;}
      if(XXISuCiSEg == true){XXISuCiSEg = false;}
      if(eLVpiVtOmo == true){eLVpiVtOmo = false;}
      if(EQqTYoJxEF == true){EQqTYoJxEF = false;}
      if(CFaxVOwzPW == true){CFaxVOwzPW = false;}
      if(HnStmRTfMN == true){HnStmRTfMN = false;}
      if(AutQzHfntF == true){AutQzHfntF = false;}
      if(slOzeZjYUT == true){slOzeZjYUT = false;}
      if(YtzPhQxxfG == true){YtzPhQxxfG = false;}
      if(VckpEGLJxN == true){VckpEGLJxN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ZJPYKDYWCR
{ 
  void qBJYYjIHxe()
  { 
      bool yPmWjbscBq = false;
      bool qxbzyStgKj = false;
      bool VLASXGJIaC = false;
      bool gRThDxGiha = false;
      bool HTdRbOqGFa = false;
      bool CGCrpmOUzc = false;
      bool SjCyKYcsuA = false;
      bool LLzjbCgiqJ = false;
      bool PCDzAZParP = false;
      bool iljEHWQeGD = false;
      bool kxKEOpGSQm = false;
      bool GfcoNPgCxj = false;
      bool ICAeRZatbE = false;
      bool kCbmYRrykK = false;
      bool TxKFJCIuqr = false;
      bool pOgWthwemo = false;
      bool PLqLBDEoPu = false;
      bool XiRmQLquVO = false;
      bool QeDiOWUFsp = false;
      bool UMKutHFyDG = false;
      string dAzkbsdlAE;
      string MwCiaOPJVx;
      string HBtfpcuEzm;
      string qpDmSTYuxV;
      string nSjNpWTKBn;
      string AcaSgcFTuz;
      string yZlMNWrPPG;
      string pTMzJnQgoT;
      string JjCLlOOkGp;
      string OEAxXOgWAT;
      string dDutxfPaJr;
      string LXeapOWaZy;
      string HHJYrlTObu;
      string HsgUoFnHOG;
      string ymbNepYYHJ;
      string qFwBLWsVCK;
      string MBzOkSqJlj;
      string ZOMTzUfwtC;
      string MtGRDBwfJS;
      string LSFDswnNbL;
      if(dAzkbsdlAE == dDutxfPaJr){yPmWjbscBq = true;}
      else if(dDutxfPaJr == dAzkbsdlAE){kxKEOpGSQm = true;}
      if(MwCiaOPJVx == LXeapOWaZy){qxbzyStgKj = true;}
      else if(LXeapOWaZy == MwCiaOPJVx){GfcoNPgCxj = true;}
      if(HBtfpcuEzm == HHJYrlTObu){VLASXGJIaC = true;}
      else if(HHJYrlTObu == HBtfpcuEzm){ICAeRZatbE = true;}
      if(qpDmSTYuxV == HsgUoFnHOG){gRThDxGiha = true;}
      else if(HsgUoFnHOG == qpDmSTYuxV){kCbmYRrykK = true;}
      if(nSjNpWTKBn == ymbNepYYHJ){HTdRbOqGFa = true;}
      else if(ymbNepYYHJ == nSjNpWTKBn){TxKFJCIuqr = true;}
      if(AcaSgcFTuz == qFwBLWsVCK){CGCrpmOUzc = true;}
      else if(qFwBLWsVCK == AcaSgcFTuz){pOgWthwemo = true;}
      if(yZlMNWrPPG == MBzOkSqJlj){SjCyKYcsuA = true;}
      else if(MBzOkSqJlj == yZlMNWrPPG){PLqLBDEoPu = true;}
      if(pTMzJnQgoT == ZOMTzUfwtC){LLzjbCgiqJ = true;}
      if(JjCLlOOkGp == MtGRDBwfJS){PCDzAZParP = true;}
      if(OEAxXOgWAT == LSFDswnNbL){iljEHWQeGD = true;}
      while(ZOMTzUfwtC == pTMzJnQgoT){XiRmQLquVO = true;}
      while(MtGRDBwfJS == MtGRDBwfJS){QeDiOWUFsp = true;}
      while(LSFDswnNbL == LSFDswnNbL){UMKutHFyDG = true;}
      if(yPmWjbscBq == true){yPmWjbscBq = false;}
      if(qxbzyStgKj == true){qxbzyStgKj = false;}
      if(VLASXGJIaC == true){VLASXGJIaC = false;}
      if(gRThDxGiha == true){gRThDxGiha = false;}
      if(HTdRbOqGFa == true){HTdRbOqGFa = false;}
      if(CGCrpmOUzc == true){CGCrpmOUzc = false;}
      if(SjCyKYcsuA == true){SjCyKYcsuA = false;}
      if(LLzjbCgiqJ == true){LLzjbCgiqJ = false;}
      if(PCDzAZParP == true){PCDzAZParP = false;}
      if(iljEHWQeGD == true){iljEHWQeGD = false;}
      if(kxKEOpGSQm == true){kxKEOpGSQm = false;}
      if(GfcoNPgCxj == true){GfcoNPgCxj = false;}
      if(ICAeRZatbE == true){ICAeRZatbE = false;}
      if(kCbmYRrykK == true){kCbmYRrykK = false;}
      if(TxKFJCIuqr == true){TxKFJCIuqr = false;}
      if(pOgWthwemo == true){pOgWthwemo = false;}
      if(PLqLBDEoPu == true){PLqLBDEoPu = false;}
      if(XiRmQLquVO == true){XiRmQLquVO = false;}
      if(QeDiOWUFsp == true){QeDiOWUFsp = false;}
      if(UMKutHFyDG == true){UMKutHFyDG = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class ITLFVGSAHL
{ 
  void oLRktlDGSB()
  { 
      bool CiUINBcbzO = false;
      bool MkxSTrlUiA = false;
      bool iVdQblNJFI = false;
      bool BGWNbWFPCs = false;
      bool JSbJbYbrQR = false;
      bool ZtXzrgFEfa = false;
      bool mTWqMGOipx = false;
      bool lVSoqQsTnD = false;
      bool JUrmiGhWUu = false;
      bool QBjaGowkwV = false;
      bool JsBSfDbiHb = false;
      bool nkQcQRaiHG = false;
      bool YqNufklqhB = false;
      bool THXStQTYHe = false;
      bool eyPsHekGKn = false;
      bool ONcpwRfGlt = false;
      bool RjAGHfNxiz = false;
      bool yBbcaZbTnB = false;
      bool AfgsmGehVb = false;
      bool kMlMNtyJka = false;
      string xRBCfWBKOw;
      string fpQkTgEGFA;
      string CGPsrsBjbT;
      string BmJWPuzFmS;
      string YNgGqEKFAy;
      string TzMalqCIJm;
      string URittrgsVX;
      string hyTmbnYICn;
      string iwjcACZijY;
      string spwzxHdlMy;
      string XQjbQVeZyx;
      string RsFVEXgGtr;
      string RxIweqCToU;
      string KmwEPRMILd;
      string WAjmGzkauY;
      string ZXrTfZHmxZ;
      string TLkdmexBFl;
      string LYtPURNVpe;
      string FaYPtlubXo;
      string KYKkKLPGzb;
      if(xRBCfWBKOw == XQjbQVeZyx){CiUINBcbzO = true;}
      else if(XQjbQVeZyx == xRBCfWBKOw){JsBSfDbiHb = true;}
      if(fpQkTgEGFA == RsFVEXgGtr){MkxSTrlUiA = true;}
      else if(RsFVEXgGtr == fpQkTgEGFA){nkQcQRaiHG = true;}
      if(CGPsrsBjbT == RxIweqCToU){iVdQblNJFI = true;}
      else if(RxIweqCToU == CGPsrsBjbT){YqNufklqhB = true;}
      if(BmJWPuzFmS == KmwEPRMILd){BGWNbWFPCs = true;}
      else if(KmwEPRMILd == BmJWPuzFmS){THXStQTYHe = true;}
      if(YNgGqEKFAy == WAjmGzkauY){JSbJbYbrQR = true;}
      else if(WAjmGzkauY == YNgGqEKFAy){eyPsHekGKn = true;}
      if(TzMalqCIJm == ZXrTfZHmxZ){ZtXzrgFEfa = true;}
      else if(ZXrTfZHmxZ == TzMalqCIJm){ONcpwRfGlt = true;}
      if(URittrgsVX == TLkdmexBFl){mTWqMGOipx = true;}
      else if(TLkdmexBFl == URittrgsVX){RjAGHfNxiz = true;}
      if(hyTmbnYICn == LYtPURNVpe){lVSoqQsTnD = true;}
      if(iwjcACZijY == FaYPtlubXo){JUrmiGhWUu = true;}
      if(spwzxHdlMy == KYKkKLPGzb){QBjaGowkwV = true;}
      while(LYtPURNVpe == hyTmbnYICn){yBbcaZbTnB = true;}
      while(FaYPtlubXo == FaYPtlubXo){AfgsmGehVb = true;}
      while(KYKkKLPGzb == KYKkKLPGzb){kMlMNtyJka = true;}
      if(CiUINBcbzO == true){CiUINBcbzO = false;}
      if(MkxSTrlUiA == true){MkxSTrlUiA = false;}
      if(iVdQblNJFI == true){iVdQblNJFI = false;}
      if(BGWNbWFPCs == true){BGWNbWFPCs = false;}
      if(JSbJbYbrQR == true){JSbJbYbrQR = false;}
      if(ZtXzrgFEfa == true){ZtXzrgFEfa = false;}
      if(mTWqMGOipx == true){mTWqMGOipx = false;}
      if(lVSoqQsTnD == true){lVSoqQsTnD = false;}
      if(JUrmiGhWUu == true){JUrmiGhWUu = false;}
      if(QBjaGowkwV == true){QBjaGowkwV = false;}
      if(JsBSfDbiHb == true){JsBSfDbiHb = false;}
      if(nkQcQRaiHG == true){nkQcQRaiHG = false;}
      if(YqNufklqhB == true){YqNufklqhB = false;}
      if(THXStQTYHe == true){THXStQTYHe = false;}
      if(eyPsHekGKn == true){eyPsHekGKn = false;}
      if(ONcpwRfGlt == true){ONcpwRfGlt = false;}
      if(RjAGHfNxiz == true){RjAGHfNxiz = false;}
      if(yBbcaZbTnB == true){yBbcaZbTnB = false;}
      if(AfgsmGehVb == true){AfgsmGehVb = false;}
      if(kMlMNtyJka == true){kMlMNtyJka = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LTCWVMQOWM
{ 
  void GEIbIZjUlF()
  { 
      bool iYGtXbUCCf = false;
      bool EMxUpmPYhN = false;
      bool iGjOUcERhe = false;
      bool DCjoyeMzeD = false;
      bool BVPsyOKtgA = false;
      bool TrbxccnqZu = false;
      bool XDUWbBcIVK = false;
      bool aQsdMMpqQF = false;
      bool RJjgKKalLV = false;
      bool pUYifyAsQx = false;
      bool lBYWZDyxkQ = false;
      bool lxaTcNSmLR = false;
      bool apwzTFqKxO = false;
      bool MxZTTPkMGH = false;
      bool LcBujssFFS = false;
      bool eNgzVuFYCU = false;
      bool MnAVDoWHAM = false;
      bool bGgEFxoBPa = false;
      bool LlEejyPaxY = false;
      bool KMJluXceqt = false;
      string XHPOwIBzxq;
      string YBTHptNteg;
      string VgmtLgoGqp;
      string reGTNjnMlS;
      string ZZwaGsyNxo;
      string yCzuKpTVCO;
      string StIHzstzsF;
      string GBHLGukLmR;
      string ibJBqofLJX;
      string PSJWAfQnbN;
      string zyUrixtmnO;
      string EIikfqkHPJ;
      string NipUonqkLa;
      string exjYSjGRSs;
      string trWuTHiYqI;
      string uJYRAPIzCt;
      string FJbNiHYJRw;
      string jSoVPULuBh;
      string EStDfVXRYC;
      string bHOXnjQJLM;
      if(XHPOwIBzxq == zyUrixtmnO){iYGtXbUCCf = true;}
      else if(zyUrixtmnO == XHPOwIBzxq){lBYWZDyxkQ = true;}
      if(YBTHptNteg == EIikfqkHPJ){EMxUpmPYhN = true;}
      else if(EIikfqkHPJ == YBTHptNteg){lxaTcNSmLR = true;}
      if(VgmtLgoGqp == NipUonqkLa){iGjOUcERhe = true;}
      else if(NipUonqkLa == VgmtLgoGqp){apwzTFqKxO = true;}
      if(reGTNjnMlS == exjYSjGRSs){DCjoyeMzeD = true;}
      else if(exjYSjGRSs == reGTNjnMlS){MxZTTPkMGH = true;}
      if(ZZwaGsyNxo == trWuTHiYqI){BVPsyOKtgA = true;}
      else if(trWuTHiYqI == ZZwaGsyNxo){LcBujssFFS = true;}
      if(yCzuKpTVCO == uJYRAPIzCt){TrbxccnqZu = true;}
      else if(uJYRAPIzCt == yCzuKpTVCO){eNgzVuFYCU = true;}
      if(StIHzstzsF == FJbNiHYJRw){XDUWbBcIVK = true;}
      else if(FJbNiHYJRw == StIHzstzsF){MnAVDoWHAM = true;}
      if(GBHLGukLmR == jSoVPULuBh){aQsdMMpqQF = true;}
      if(ibJBqofLJX == EStDfVXRYC){RJjgKKalLV = true;}
      if(PSJWAfQnbN == bHOXnjQJLM){pUYifyAsQx = true;}
      while(jSoVPULuBh == GBHLGukLmR){bGgEFxoBPa = true;}
      while(EStDfVXRYC == EStDfVXRYC){LlEejyPaxY = true;}
      while(bHOXnjQJLM == bHOXnjQJLM){KMJluXceqt = true;}
      if(iYGtXbUCCf == true){iYGtXbUCCf = false;}
      if(EMxUpmPYhN == true){EMxUpmPYhN = false;}
      if(iGjOUcERhe == true){iGjOUcERhe = false;}
      if(DCjoyeMzeD == true){DCjoyeMzeD = false;}
      if(BVPsyOKtgA == true){BVPsyOKtgA = false;}
      if(TrbxccnqZu == true){TrbxccnqZu = false;}
      if(XDUWbBcIVK == true){XDUWbBcIVK = false;}
      if(aQsdMMpqQF == true){aQsdMMpqQF = false;}
      if(RJjgKKalLV == true){RJjgKKalLV = false;}
      if(pUYifyAsQx == true){pUYifyAsQx = false;}
      if(lBYWZDyxkQ == true){lBYWZDyxkQ = false;}
      if(lxaTcNSmLR == true){lxaTcNSmLR = false;}
      if(apwzTFqKxO == true){apwzTFqKxO = false;}
      if(MxZTTPkMGH == true){MxZTTPkMGH = false;}
      if(LcBujssFFS == true){LcBujssFFS = false;}
      if(eNgzVuFYCU == true){eNgzVuFYCU = false;}
      if(MnAVDoWHAM == true){MnAVDoWHAM = false;}
      if(bGgEFxoBPa == true){bGgEFxoBPa = false;}
      if(LlEejyPaxY == true){LlEejyPaxY = false;}
      if(KMJluXceqt == true){KMJluXceqt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SJCDQMVRWS
{ 
  void iFJpISzRQi()
  { 
      bool DRHkZlbLfn = false;
      bool JzHXESekEe = false;
      bool kYoOXLOtia = false;
      bool zyjgBRqwer = false;
      bool kUNoZJjcJs = false;
      bool SIcgqdbrNN = false;
      bool WgcFnLPZWl = false;
      bool aiGMsQLAoO = false;
      bool alkpHpUCpW = false;
      bool ozhQUEqIAO = false;
      bool sKFWRjCQma = false;
      bool SbDzmjwlyy = false;
      bool pMjzxbApro = false;
      bool YCzZUQgNYW = false;
      bool nEPOENuicj = false;
      bool HEJCXgZFZo = false;
      bool jMYyCPbceV = false;
      bool DiNaJlxUuj = false;
      bool gQMHgTrxCn = false;
      bool panimbeQMn = false;
      string txaKWpeSbz;
      string xySVQCrnKy;
      string CWoXamyDAC;
      string oJVZUySLli;
      string WLEzzPnUVq;
      string CPApfWIDii;
      string qaRYJmcsAR;
      string jfCabqSoXL;
      string zdhIAYJesx;
      string pPbCrusfoM;
      string muTDHVNYOo;
      string ESeWjNsJYj;
      string dxzDaTEAif;
      string xmzLYEJdBd;
      string LjQDsWkFRd;
      string BjGFhouQcm;
      string RNpirQTSyf;
      string EPIuYsfuJE;
      string iyhEdNftdi;
      string IrANLythLb;
      if(txaKWpeSbz == muTDHVNYOo){DRHkZlbLfn = true;}
      else if(muTDHVNYOo == txaKWpeSbz){sKFWRjCQma = true;}
      if(xySVQCrnKy == ESeWjNsJYj){JzHXESekEe = true;}
      else if(ESeWjNsJYj == xySVQCrnKy){SbDzmjwlyy = true;}
      if(CWoXamyDAC == dxzDaTEAif){kYoOXLOtia = true;}
      else if(dxzDaTEAif == CWoXamyDAC){pMjzxbApro = true;}
      if(oJVZUySLli == xmzLYEJdBd){zyjgBRqwer = true;}
      else if(xmzLYEJdBd == oJVZUySLli){YCzZUQgNYW = true;}
      if(WLEzzPnUVq == LjQDsWkFRd){kUNoZJjcJs = true;}
      else if(LjQDsWkFRd == WLEzzPnUVq){nEPOENuicj = true;}
      if(CPApfWIDii == BjGFhouQcm){SIcgqdbrNN = true;}
      else if(BjGFhouQcm == CPApfWIDii){HEJCXgZFZo = true;}
      if(qaRYJmcsAR == RNpirQTSyf){WgcFnLPZWl = true;}
      else if(RNpirQTSyf == qaRYJmcsAR){jMYyCPbceV = true;}
      if(jfCabqSoXL == EPIuYsfuJE){aiGMsQLAoO = true;}
      if(zdhIAYJesx == iyhEdNftdi){alkpHpUCpW = true;}
      if(pPbCrusfoM == IrANLythLb){ozhQUEqIAO = true;}
      while(EPIuYsfuJE == jfCabqSoXL){DiNaJlxUuj = true;}
      while(iyhEdNftdi == iyhEdNftdi){gQMHgTrxCn = true;}
      while(IrANLythLb == IrANLythLb){panimbeQMn = true;}
      if(DRHkZlbLfn == true){DRHkZlbLfn = false;}
      if(JzHXESekEe == true){JzHXESekEe = false;}
      if(kYoOXLOtia == true){kYoOXLOtia = false;}
      if(zyjgBRqwer == true){zyjgBRqwer = false;}
      if(kUNoZJjcJs == true){kUNoZJjcJs = false;}
      if(SIcgqdbrNN == true){SIcgqdbrNN = false;}
      if(WgcFnLPZWl == true){WgcFnLPZWl = false;}
      if(aiGMsQLAoO == true){aiGMsQLAoO = false;}
      if(alkpHpUCpW == true){alkpHpUCpW = false;}
      if(ozhQUEqIAO == true){ozhQUEqIAO = false;}
      if(sKFWRjCQma == true){sKFWRjCQma = false;}
      if(SbDzmjwlyy == true){SbDzmjwlyy = false;}
      if(pMjzxbApro == true){pMjzxbApro = false;}
      if(YCzZUQgNYW == true){YCzZUQgNYW = false;}
      if(nEPOENuicj == true){nEPOENuicj = false;}
      if(HEJCXgZFZo == true){HEJCXgZFZo = false;}
      if(jMYyCPbceV == true){jMYyCPbceV = false;}
      if(DiNaJlxUuj == true){DiNaJlxUuj = false;}
      if(gQMHgTrxCn == true){gQMHgTrxCn = false;}
      if(panimbeQMn == true){panimbeQMn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XCOHEGFSGB
{ 
  void OohJikFwdb()
  { 
      bool gBuwZfYKAw = false;
      bool EizBerQtZA = false;
      bool TckjnbjAPd = false;
      bool EkhinSxDfb = false;
      bool NVHsuRszKd = false;
      bool bHLIBDQKIx = false;
      bool uYuVrTBPML = false;
      bool tNNymxVqnV = false;
      bool ikpkCwZYhj = false;
      bool CltifnakBb = false;
      bool PclyiNsMUE = false;
      bool hYWlNKiJTu = false;
      bool sbhcJHSeSy = false;
      bool WkpLFGyqCn = false;
      bool CMDcBdPIVU = false;
      bool FRQFintJoa = false;
      bool uOtWgxhKfH = false;
      bool MqGfrfYEae = false;
      bool rJJxrFTjcR = false;
      bool LoBarnSgwE = false;
      string CXIupRyqyp;
      string eDBHyXTuTP;
      string ALafkiYJdp;
      string zRjYldaDIF;
      string PGwsZnrycs;
      string ZnOHjpxbKG;
      string roNIqMTcUT;
      string wQlNAtPgGx;
      string qafjPKDbnu;
      string gyrBMarnCM;
      string TPdIwIBAHp;
      string aPVNPRIcrf;
      string JqoYlxXkAZ;
      string CbetcaxWnG;
      string ICTxURcekt;
      string ROuWDbaEVz;
      string kQAhnzjbjn;
      string UkokWGXqEU;
      string UrzFHmfHpA;
      string ArAoVrClJU;
      if(CXIupRyqyp == TPdIwIBAHp){gBuwZfYKAw = true;}
      else if(TPdIwIBAHp == CXIupRyqyp){PclyiNsMUE = true;}
      if(eDBHyXTuTP == aPVNPRIcrf){EizBerQtZA = true;}
      else if(aPVNPRIcrf == eDBHyXTuTP){hYWlNKiJTu = true;}
      if(ALafkiYJdp == JqoYlxXkAZ){TckjnbjAPd = true;}
      else if(JqoYlxXkAZ == ALafkiYJdp){sbhcJHSeSy = true;}
      if(zRjYldaDIF == CbetcaxWnG){EkhinSxDfb = true;}
      else if(CbetcaxWnG == zRjYldaDIF){WkpLFGyqCn = true;}
      if(PGwsZnrycs == ICTxURcekt){NVHsuRszKd = true;}
      else if(ICTxURcekt == PGwsZnrycs){CMDcBdPIVU = true;}
      if(ZnOHjpxbKG == ROuWDbaEVz){bHLIBDQKIx = true;}
      else if(ROuWDbaEVz == ZnOHjpxbKG){FRQFintJoa = true;}
      if(roNIqMTcUT == kQAhnzjbjn){uYuVrTBPML = true;}
      else if(kQAhnzjbjn == roNIqMTcUT){uOtWgxhKfH = true;}
      if(wQlNAtPgGx == UkokWGXqEU){tNNymxVqnV = true;}
      if(qafjPKDbnu == UrzFHmfHpA){ikpkCwZYhj = true;}
      if(gyrBMarnCM == ArAoVrClJU){CltifnakBb = true;}
      while(UkokWGXqEU == wQlNAtPgGx){MqGfrfYEae = true;}
      while(UrzFHmfHpA == UrzFHmfHpA){rJJxrFTjcR = true;}
      while(ArAoVrClJU == ArAoVrClJU){LoBarnSgwE = true;}
      if(gBuwZfYKAw == true){gBuwZfYKAw = false;}
      if(EizBerQtZA == true){EizBerQtZA = false;}
      if(TckjnbjAPd == true){TckjnbjAPd = false;}
      if(EkhinSxDfb == true){EkhinSxDfb = false;}
      if(NVHsuRszKd == true){NVHsuRszKd = false;}
      if(bHLIBDQKIx == true){bHLIBDQKIx = false;}
      if(uYuVrTBPML == true){uYuVrTBPML = false;}
      if(tNNymxVqnV == true){tNNymxVqnV = false;}
      if(ikpkCwZYhj == true){ikpkCwZYhj = false;}
      if(CltifnakBb == true){CltifnakBb = false;}
      if(PclyiNsMUE == true){PclyiNsMUE = false;}
      if(hYWlNKiJTu == true){hYWlNKiJTu = false;}
      if(sbhcJHSeSy == true){sbhcJHSeSy = false;}
      if(WkpLFGyqCn == true){WkpLFGyqCn = false;}
      if(CMDcBdPIVU == true){CMDcBdPIVU = false;}
      if(FRQFintJoa == true){FRQFintJoa = false;}
      if(uOtWgxhKfH == true){uOtWgxhKfH = false;}
      if(MqGfrfYEae == true){MqGfrfYEae = false;}
      if(rJJxrFTjcR == true){rJJxrFTjcR = false;}
      if(LoBarnSgwE == true){LoBarnSgwE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OJSBPAEOVG
{ 
  void EXAccDTONr()
  { 
      bool aOjPFSGaLf = false;
      bool KCEVaUSLiQ = false;
      bool OfRuHRxMBO = false;
      bool fPFFATVxTu = false;
      bool ksLArbaRMQ = false;
      bool aztTjtwnHm = false;
      bool UsZxztxzUG = false;
      bool zVrneUURDf = false;
      bool qfAjmgUlUG = false;
      bool WgogZrTOBx = false;
      bool dbwFRFGGnO = false;
      bool ZejbworoJR = false;
      bool YLeOjSgLNm = false;
      bool AGPGBOitSh = false;
      bool ZOxFgqwMjj = false;
      bool zeEiXSxhVz = false;
      bool XurBOSPdag = false;
      bool GyVbrKNYhr = false;
      bool VUadmVBgii = false;
      bool RBYTJeWwqN = false;
      string VJnoIBkWDB;
      string nFsQGqutld;
      string NAMXICaZTW;
      string tpwMZXfTSR;
      string lIyNIeGnkQ;
      string rWgHMVwQgG;
      string aLCfHxIgqh;
      string KbbTrFBzOc;
      string wloDdWJozD;
      string bOmLqZGRGL;
      string WZxDFQuMOH;
      string TxBEJnWyCj;
      string nSgmGwoPbR;
      string zjsDFWokxN;
      string awZridMiCD;
      string DHPVWDrJxJ;
      string afkxIjDyyl;
      string ykBnJqsoEb;
      string iGMYNqUXfC;
      string efmnltLcjz;
      if(VJnoIBkWDB == WZxDFQuMOH){aOjPFSGaLf = true;}
      else if(WZxDFQuMOH == VJnoIBkWDB){dbwFRFGGnO = true;}
      if(nFsQGqutld == TxBEJnWyCj){KCEVaUSLiQ = true;}
      else if(TxBEJnWyCj == nFsQGqutld){ZejbworoJR = true;}
      if(NAMXICaZTW == nSgmGwoPbR){OfRuHRxMBO = true;}
      else if(nSgmGwoPbR == NAMXICaZTW){YLeOjSgLNm = true;}
      if(tpwMZXfTSR == zjsDFWokxN){fPFFATVxTu = true;}
      else if(zjsDFWokxN == tpwMZXfTSR){AGPGBOitSh = true;}
      if(lIyNIeGnkQ == awZridMiCD){ksLArbaRMQ = true;}
      else if(awZridMiCD == lIyNIeGnkQ){ZOxFgqwMjj = true;}
      if(rWgHMVwQgG == DHPVWDrJxJ){aztTjtwnHm = true;}
      else if(DHPVWDrJxJ == rWgHMVwQgG){zeEiXSxhVz = true;}
      if(aLCfHxIgqh == afkxIjDyyl){UsZxztxzUG = true;}
      else if(afkxIjDyyl == aLCfHxIgqh){XurBOSPdag = true;}
      if(KbbTrFBzOc == ykBnJqsoEb){zVrneUURDf = true;}
      if(wloDdWJozD == iGMYNqUXfC){qfAjmgUlUG = true;}
      if(bOmLqZGRGL == efmnltLcjz){WgogZrTOBx = true;}
      while(ykBnJqsoEb == KbbTrFBzOc){GyVbrKNYhr = true;}
      while(iGMYNqUXfC == iGMYNqUXfC){VUadmVBgii = true;}
      while(efmnltLcjz == efmnltLcjz){RBYTJeWwqN = true;}
      if(aOjPFSGaLf == true){aOjPFSGaLf = false;}
      if(KCEVaUSLiQ == true){KCEVaUSLiQ = false;}
      if(OfRuHRxMBO == true){OfRuHRxMBO = false;}
      if(fPFFATVxTu == true){fPFFATVxTu = false;}
      if(ksLArbaRMQ == true){ksLArbaRMQ = false;}
      if(aztTjtwnHm == true){aztTjtwnHm = false;}
      if(UsZxztxzUG == true){UsZxztxzUG = false;}
      if(zVrneUURDf == true){zVrneUURDf = false;}
      if(qfAjmgUlUG == true){qfAjmgUlUG = false;}
      if(WgogZrTOBx == true){WgogZrTOBx = false;}
      if(dbwFRFGGnO == true){dbwFRFGGnO = false;}
      if(ZejbworoJR == true){ZejbworoJR = false;}
      if(YLeOjSgLNm == true){YLeOjSgLNm = false;}
      if(AGPGBOitSh == true){AGPGBOitSh = false;}
      if(ZOxFgqwMjj == true){ZOxFgqwMjj = false;}
      if(zeEiXSxhVz == true){zeEiXSxhVz = false;}
      if(XurBOSPdag == true){XurBOSPdag = false;}
      if(GyVbrKNYhr == true){GyVbrKNYhr = false;}
      if(VUadmVBgii == true){VUadmVBgii = false;}
      if(RBYTJeWwqN == true){RBYTJeWwqN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class UZDLRZDKYL
{ 
  void ZLwWNiQbHm()
  { 
      bool wjhwELcNKY = false;
      bool AmsQxjqHom = false;
      bool SieVEATwwS = false;
      bool hjhfQHLLXw = false;
      bool BGUIaYAkVD = false;
      bool bSBRbNFMqE = false;
      bool rgQqpPFMzp = false;
      bool YejrlMNVte = false;
      bool zhZrpPiPED = false;
      bool CzUTETpcRK = false;
      bool TmkfexiIeP = false;
      bool ziRMEJHYAa = false;
      bool goIwnZKQnR = false;
      bool WjDjLLxtMs = false;
      bool gZkoESgIOu = false;
      bool MRcqddeoqF = false;
      bool lRUMsOehAr = false;
      bool MMLHsGETrU = false;
      bool rCBrFFHMGg = false;
      bool gICHHOFXwo = false;
      string NFPiwfqmys;
      string XlKLwegdDr;
      string TohTkXZHaa;
      string xbqpMXTcqP;
      string SdWAAnCeZo;
      string GIBNeXXuxY;
      string FjLWwwtTzX;
      string LEmmYpSgTx;
      string UuhlUoZdLn;
      string AXeRKiSAED;
      string shfBcCUNno;
      string FpbAEKBYCG;
      string VQIKWmYVcp;
      string XRcPDlPfJR;
      string TLoViJweNz;
      string gsxxhbZUdo;
      string dzkyRmNLxR;
      string tVPjmJblbM;
      string qsHcIdTega;
      string MtCTnTaqEu;
      if(NFPiwfqmys == shfBcCUNno){wjhwELcNKY = true;}
      else if(shfBcCUNno == NFPiwfqmys){TmkfexiIeP = true;}
      if(XlKLwegdDr == FpbAEKBYCG){AmsQxjqHom = true;}
      else if(FpbAEKBYCG == XlKLwegdDr){ziRMEJHYAa = true;}
      if(TohTkXZHaa == VQIKWmYVcp){SieVEATwwS = true;}
      else if(VQIKWmYVcp == TohTkXZHaa){goIwnZKQnR = true;}
      if(xbqpMXTcqP == XRcPDlPfJR){hjhfQHLLXw = true;}
      else if(XRcPDlPfJR == xbqpMXTcqP){WjDjLLxtMs = true;}
      if(SdWAAnCeZo == TLoViJweNz){BGUIaYAkVD = true;}
      else if(TLoViJweNz == SdWAAnCeZo){gZkoESgIOu = true;}
      if(GIBNeXXuxY == gsxxhbZUdo){bSBRbNFMqE = true;}
      else if(gsxxhbZUdo == GIBNeXXuxY){MRcqddeoqF = true;}
      if(FjLWwwtTzX == dzkyRmNLxR){rgQqpPFMzp = true;}
      else if(dzkyRmNLxR == FjLWwwtTzX){lRUMsOehAr = true;}
      if(LEmmYpSgTx == tVPjmJblbM){YejrlMNVte = true;}
      if(UuhlUoZdLn == qsHcIdTega){zhZrpPiPED = true;}
      if(AXeRKiSAED == MtCTnTaqEu){CzUTETpcRK = true;}
      while(tVPjmJblbM == LEmmYpSgTx){MMLHsGETrU = true;}
      while(qsHcIdTega == qsHcIdTega){rCBrFFHMGg = true;}
      while(MtCTnTaqEu == MtCTnTaqEu){gICHHOFXwo = true;}
      if(wjhwELcNKY == true){wjhwELcNKY = false;}
      if(AmsQxjqHom == true){AmsQxjqHom = false;}
      if(SieVEATwwS == true){SieVEATwwS = false;}
      if(hjhfQHLLXw == true){hjhfQHLLXw = false;}
      if(BGUIaYAkVD == true){BGUIaYAkVD = false;}
      if(bSBRbNFMqE == true){bSBRbNFMqE = false;}
      if(rgQqpPFMzp == true){rgQqpPFMzp = false;}
      if(YejrlMNVte == true){YejrlMNVte = false;}
      if(zhZrpPiPED == true){zhZrpPiPED = false;}
      if(CzUTETpcRK == true){CzUTETpcRK = false;}
      if(TmkfexiIeP == true){TmkfexiIeP = false;}
      if(ziRMEJHYAa == true){ziRMEJHYAa = false;}
      if(goIwnZKQnR == true){goIwnZKQnR = false;}
      if(WjDjLLxtMs == true){WjDjLLxtMs = false;}
      if(gZkoESgIOu == true){gZkoESgIOu = false;}
      if(MRcqddeoqF == true){MRcqddeoqF = false;}
      if(lRUMsOehAr == true){lRUMsOehAr = false;}
      if(MMLHsGETrU == true){MMLHsGETrU = false;}
      if(rCBrFFHMGg == true){rCBrFFHMGg = false;}
      if(gICHHOFXwo == true){gICHHOFXwo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WRWPSYEGMQ
{ 
  void rUEqCDOTmJ()
  { 
      bool BHekNBRiTZ = false;
      bool YBxyxWmkep = false;
      bool IoMDsTWpjO = false;
      bool aJNjnIyyHQ = false;
      bool ntEQpMDdTR = false;
      bool PmnLpMtaqt = false;
      bool eayEzPnNJo = false;
      bool XocNBNkuMg = false;
      bool PrWSzmVCtb = false;
      bool NXGNkMwzRZ = false;
      bool XpTesaKZJb = false;
      bool byNKdlEfFL = false;
      bool tlThaAiPco = false;
      bool RkdmIZYrYY = false;
      bool curRAOJuiz = false;
      bool IQtGflWlDs = false;
      bool qtxUKqUaCE = false;
      bool NLGghysBgL = false;
      bool pnfcgBpWHV = false;
      bool CmnfghyXGJ = false;
      string lAraWCfcgu;
      string UIbSziplQb;
      string LHkAeYPDVC;
      string ewhqoEYedY;
      string rTzudTOfqA;
      string lGDASeOqwx;
      string mbHdQXZkdu;
      string JKIqjqHoWL;
      string JjMUPXrgXJ;
      string ogiflQLVeF;
      string sdBupbyMQa;
      string XYUkmDPlNr;
      string frMUGcbbET;
      string elKQRfhQen;
      string ZfQxkPLyri;
      string AZHlZBRgoN;
      string AWHxulNETH;
      string UyHYSzbPip;
      string QciVSFdYqP;
      string OBbQnDYSBz;
      if(lAraWCfcgu == sdBupbyMQa){BHekNBRiTZ = true;}
      else if(sdBupbyMQa == lAraWCfcgu){XpTesaKZJb = true;}
      if(UIbSziplQb == XYUkmDPlNr){YBxyxWmkep = true;}
      else if(XYUkmDPlNr == UIbSziplQb){byNKdlEfFL = true;}
      if(LHkAeYPDVC == frMUGcbbET){IoMDsTWpjO = true;}
      else if(frMUGcbbET == LHkAeYPDVC){tlThaAiPco = true;}
      if(ewhqoEYedY == elKQRfhQen){aJNjnIyyHQ = true;}
      else if(elKQRfhQen == ewhqoEYedY){RkdmIZYrYY = true;}
      if(rTzudTOfqA == ZfQxkPLyri){ntEQpMDdTR = true;}
      else if(ZfQxkPLyri == rTzudTOfqA){curRAOJuiz = true;}
      if(lGDASeOqwx == AZHlZBRgoN){PmnLpMtaqt = true;}
      else if(AZHlZBRgoN == lGDASeOqwx){IQtGflWlDs = true;}
      if(mbHdQXZkdu == AWHxulNETH){eayEzPnNJo = true;}
      else if(AWHxulNETH == mbHdQXZkdu){qtxUKqUaCE = true;}
      if(JKIqjqHoWL == UyHYSzbPip){XocNBNkuMg = true;}
      if(JjMUPXrgXJ == QciVSFdYqP){PrWSzmVCtb = true;}
      if(ogiflQLVeF == OBbQnDYSBz){NXGNkMwzRZ = true;}
      while(UyHYSzbPip == JKIqjqHoWL){NLGghysBgL = true;}
      while(QciVSFdYqP == QciVSFdYqP){pnfcgBpWHV = true;}
      while(OBbQnDYSBz == OBbQnDYSBz){CmnfghyXGJ = true;}
      if(BHekNBRiTZ == true){BHekNBRiTZ = false;}
      if(YBxyxWmkep == true){YBxyxWmkep = false;}
      if(IoMDsTWpjO == true){IoMDsTWpjO = false;}
      if(aJNjnIyyHQ == true){aJNjnIyyHQ = false;}
      if(ntEQpMDdTR == true){ntEQpMDdTR = false;}
      if(PmnLpMtaqt == true){PmnLpMtaqt = false;}
      if(eayEzPnNJo == true){eayEzPnNJo = false;}
      if(XocNBNkuMg == true){XocNBNkuMg = false;}
      if(PrWSzmVCtb == true){PrWSzmVCtb = false;}
      if(NXGNkMwzRZ == true){NXGNkMwzRZ = false;}
      if(XpTesaKZJb == true){XpTesaKZJb = false;}
      if(byNKdlEfFL == true){byNKdlEfFL = false;}
      if(tlThaAiPco == true){tlThaAiPco = false;}
      if(RkdmIZYrYY == true){RkdmIZYrYY = false;}
      if(curRAOJuiz == true){curRAOJuiz = false;}
      if(IQtGflWlDs == true){IQtGflWlDs = false;}
      if(qtxUKqUaCE == true){qtxUKqUaCE = false;}
      if(NLGghysBgL == true){NLGghysBgL = false;}
      if(pnfcgBpWHV == true){pnfcgBpWHV = false;}
      if(CmnfghyXGJ == true){CmnfghyXGJ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JHGBTLWDMG
{ 
  void aCtCnkIVhl()
  { 
      bool AfKTyrZciu = false;
      bool fFUugaCOxx = false;
      bool wnKAhYMRFj = false;
      bool XlwixRtpiB = false;
      bool rIdyMTQaPy = false;
      bool josCKLUJlZ = false;
      bool fIXzWsJlxP = false;
      bool gMewFnKMnO = false;
      bool HkBGzissXl = false;
      bool xjDJVikdaS = false;
      bool KedzRWJxxW = false;
      bool AHUXRtGQAn = false;
      bool APNKSwIVaV = false;
      bool CfkVDcHBnc = false;
      bool ZuwAloOfqa = false;
      bool bUdmRtQSgV = false;
      bool NmVaUUWzTn = false;
      bool mFbWNESNYd = false;
      bool gcdAshBxEJ = false;
      bool MnUSOlZVfQ = false;
      string VSbCfflYLa;
      string CKwiRqLoVf;
      string wGzkqEicsi;
      string CkExirzHoN;
      string jKTBRYNsSE;
      string qBwJJPTpLa;
      string xIebXRFzAY;
      string qEAHwllxSS;
      string poJorBoumS;
      string YIwNQaercA;
      string SOrsMmAnMW;
      string cVXrjHzpjW;
      string cKMgAhHmqV;
      string HtTRAbxPbE;
      string AAhPpCkDcJ;
      string bnIGXHCLse;
      string eNwqPhCFrI;
      string KiOeWbbwpy;
      string xEKhtSlNtr;
      string ZkWxYmDpzH;
      if(VSbCfflYLa == SOrsMmAnMW){AfKTyrZciu = true;}
      else if(SOrsMmAnMW == VSbCfflYLa){KedzRWJxxW = true;}
      if(CKwiRqLoVf == cVXrjHzpjW){fFUugaCOxx = true;}
      else if(cVXrjHzpjW == CKwiRqLoVf){AHUXRtGQAn = true;}
      if(wGzkqEicsi == cKMgAhHmqV){wnKAhYMRFj = true;}
      else if(cKMgAhHmqV == wGzkqEicsi){APNKSwIVaV = true;}
      if(CkExirzHoN == HtTRAbxPbE){XlwixRtpiB = true;}
      else if(HtTRAbxPbE == CkExirzHoN){CfkVDcHBnc = true;}
      if(jKTBRYNsSE == AAhPpCkDcJ){rIdyMTQaPy = true;}
      else if(AAhPpCkDcJ == jKTBRYNsSE){ZuwAloOfqa = true;}
      if(qBwJJPTpLa == bnIGXHCLse){josCKLUJlZ = true;}
      else if(bnIGXHCLse == qBwJJPTpLa){bUdmRtQSgV = true;}
      if(xIebXRFzAY == eNwqPhCFrI){fIXzWsJlxP = true;}
      else if(eNwqPhCFrI == xIebXRFzAY){NmVaUUWzTn = true;}
      if(qEAHwllxSS == KiOeWbbwpy){gMewFnKMnO = true;}
      if(poJorBoumS == xEKhtSlNtr){HkBGzissXl = true;}
      if(YIwNQaercA == ZkWxYmDpzH){xjDJVikdaS = true;}
      while(KiOeWbbwpy == qEAHwllxSS){mFbWNESNYd = true;}
      while(xEKhtSlNtr == xEKhtSlNtr){gcdAshBxEJ = true;}
      while(ZkWxYmDpzH == ZkWxYmDpzH){MnUSOlZVfQ = true;}
      if(AfKTyrZciu == true){AfKTyrZciu = false;}
      if(fFUugaCOxx == true){fFUugaCOxx = false;}
      if(wnKAhYMRFj == true){wnKAhYMRFj = false;}
      if(XlwixRtpiB == true){XlwixRtpiB = false;}
      if(rIdyMTQaPy == true){rIdyMTQaPy = false;}
      if(josCKLUJlZ == true){josCKLUJlZ = false;}
      if(fIXzWsJlxP == true){fIXzWsJlxP = false;}
      if(gMewFnKMnO == true){gMewFnKMnO = false;}
      if(HkBGzissXl == true){HkBGzissXl = false;}
      if(xjDJVikdaS == true){xjDJVikdaS = false;}
      if(KedzRWJxxW == true){KedzRWJxxW = false;}
      if(AHUXRtGQAn == true){AHUXRtGQAn = false;}
      if(APNKSwIVaV == true){APNKSwIVaV = false;}
      if(CfkVDcHBnc == true){CfkVDcHBnc = false;}
      if(ZuwAloOfqa == true){ZuwAloOfqa = false;}
      if(bUdmRtQSgV == true){bUdmRtQSgV = false;}
      if(NmVaUUWzTn == true){NmVaUUWzTn = false;}
      if(mFbWNESNYd == true){mFbWNESNYd = false;}
      if(gcdAshBxEJ == true){gcdAshBxEJ = false;}
      if(MnUSOlZVfQ == true){MnUSOlZVfQ = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VULFEKRXFR
{ 
  void gGNOHCDMWg()
  { 
      bool UoiSCGOmpC = false;
      bool dTphATzZZe = false;
      bool kwnjQYcuUs = false;
      bool LFYGTxEASG = false;
      bool fUaOXDTPKb = false;
      bool rfBPzXAzCd = false;
      bool cTRwpXtiNC = false;
      bool PyGTyQVYLM = false;
      bool ENjrBfEJHw = false;
      bool wEyocMCdYA = false;
      bool IjCHMsYgQh = false;
      bool CbnoYPiZuJ = false;
      bool TjbQGKARqk = false;
      bool OqzLETKbfH = false;
      bool GWsZsTQIPM = false;
      bool cciOfYIkBB = false;
      bool sGpeSKbbkF = false;
      bool EHsnDwqBXl = false;
      bool tqCdIlIkef = false;
      bool LZaeNAfNGh = false;
      string jrIYmagXxX;
      string BqiZPXGPDM;
      string gwbpKGDVDJ;
      string abBmVDEfkj;
      string nrMQOtVHGm;
      string pXWFdnPSNg;
      string WKUnxZpwDL;
      string SZdTXniKaQ;
      string jwPQTDFuJh;
      string mlRDMcXXTo;
      string EZDuTPsCho;
      string EEqxxauDUt;
      string UtmVXgIZHu;
      string PKqlLJrUxD;
      string JCNTditUGk;
      string hUDZUJmhWl;
      string HSumZsNFkm;
      string iaFQjSfSqo;
      string lmTVMuHQRB;
      string WKxKwqGatk;
      if(jrIYmagXxX == EZDuTPsCho){UoiSCGOmpC = true;}
      else if(EZDuTPsCho == jrIYmagXxX){IjCHMsYgQh = true;}
      if(BqiZPXGPDM == EEqxxauDUt){dTphATzZZe = true;}
      else if(EEqxxauDUt == BqiZPXGPDM){CbnoYPiZuJ = true;}
      if(gwbpKGDVDJ == UtmVXgIZHu){kwnjQYcuUs = true;}
      else if(UtmVXgIZHu == gwbpKGDVDJ){TjbQGKARqk = true;}
      if(abBmVDEfkj == PKqlLJrUxD){LFYGTxEASG = true;}
      else if(PKqlLJrUxD == abBmVDEfkj){OqzLETKbfH = true;}
      if(nrMQOtVHGm == JCNTditUGk){fUaOXDTPKb = true;}
      else if(JCNTditUGk == nrMQOtVHGm){GWsZsTQIPM = true;}
      if(pXWFdnPSNg == hUDZUJmhWl){rfBPzXAzCd = true;}
      else if(hUDZUJmhWl == pXWFdnPSNg){cciOfYIkBB = true;}
      if(WKUnxZpwDL == HSumZsNFkm){cTRwpXtiNC = true;}
      else if(HSumZsNFkm == WKUnxZpwDL){sGpeSKbbkF = true;}
      if(SZdTXniKaQ == iaFQjSfSqo){PyGTyQVYLM = true;}
      if(jwPQTDFuJh == lmTVMuHQRB){ENjrBfEJHw = true;}
      if(mlRDMcXXTo == WKxKwqGatk){wEyocMCdYA = true;}
      while(iaFQjSfSqo == SZdTXniKaQ){EHsnDwqBXl = true;}
      while(lmTVMuHQRB == lmTVMuHQRB){tqCdIlIkef = true;}
      while(WKxKwqGatk == WKxKwqGatk){LZaeNAfNGh = true;}
      if(UoiSCGOmpC == true){UoiSCGOmpC = false;}
      if(dTphATzZZe == true){dTphATzZZe = false;}
      if(kwnjQYcuUs == true){kwnjQYcuUs = false;}
      if(LFYGTxEASG == true){LFYGTxEASG = false;}
      if(fUaOXDTPKb == true){fUaOXDTPKb = false;}
      if(rfBPzXAzCd == true){rfBPzXAzCd = false;}
      if(cTRwpXtiNC == true){cTRwpXtiNC = false;}
      if(PyGTyQVYLM == true){PyGTyQVYLM = false;}
      if(ENjrBfEJHw == true){ENjrBfEJHw = false;}
      if(wEyocMCdYA == true){wEyocMCdYA = false;}
      if(IjCHMsYgQh == true){IjCHMsYgQh = false;}
      if(CbnoYPiZuJ == true){CbnoYPiZuJ = false;}
      if(TjbQGKARqk == true){TjbQGKARqk = false;}
      if(OqzLETKbfH == true){OqzLETKbfH = false;}
      if(GWsZsTQIPM == true){GWsZsTQIPM = false;}
      if(cciOfYIkBB == true){cciOfYIkBB = false;}
      if(sGpeSKbbkF == true){sGpeSKbbkF = false;}
      if(EHsnDwqBXl == true){EHsnDwqBXl = false;}
      if(tqCdIlIkef == true){tqCdIlIkef = false;}
      if(LZaeNAfNGh == true){LZaeNAfNGh = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class RUTBZEWXNM
{ 
  void fzMVQbPxPL()
  { 
      bool YHZonSlFQL = false;
      bool mddmnzZHrO = false;
      bool kOJSZuceRR = false;
      bool JcKMfnrrij = false;
      bool kOYkZbmnZp = false;
      bool EBpLMKxGOs = false;
      bool pGnyBKTQGA = false;
      bool GgYIcWoYsy = false;
      bool HoFaNIlyAV = false;
      bool plcotdaZiO = false;
      bool HVgYiWcgio = false;
      bool UXNQkBBMks = false;
      bool XVQkKUDiOy = false;
      bool wOpdpkHgrp = false;
      bool fFOKARXMEr = false;
      bool cNooqlFwON = false;
      bool StSksFhiZA = false;
      bool sTUtNSGGjr = false;
      bool xqdVUhZAmD = false;
      bool eKRdXkldQu = false;
      string gRfYsiDaNI;
      string WmgexrGnhk;
      string npQngFJjpD;
      string fdubehmmMr;
      string OJjVcgTBpy;
      string HkpsVqArqn;
      string ZSkJbsrEAP;
      string ESaoWdFuKA;
      string kHtruumBFm;
      string huwYSuYZsj;
      string oOBZsnJAWr;
      string kFgqCRXrNg;
      string FpzMhJWCWa;
      string EfBaLVFTyG;
      string kleNCbNFVA;
      string XfLMdyPnrj;
      string RGdwmEDGeg;
      string MEtGwSLkKF;
      string uRgWDXTowM;
      string GRaOodIiax;
      if(gRfYsiDaNI == oOBZsnJAWr){YHZonSlFQL = true;}
      else if(oOBZsnJAWr == gRfYsiDaNI){HVgYiWcgio = true;}
      if(WmgexrGnhk == kFgqCRXrNg){mddmnzZHrO = true;}
      else if(kFgqCRXrNg == WmgexrGnhk){UXNQkBBMks = true;}
      if(npQngFJjpD == FpzMhJWCWa){kOJSZuceRR = true;}
      else if(FpzMhJWCWa == npQngFJjpD){XVQkKUDiOy = true;}
      if(fdubehmmMr == EfBaLVFTyG){JcKMfnrrij = true;}
      else if(EfBaLVFTyG == fdubehmmMr){wOpdpkHgrp = true;}
      if(OJjVcgTBpy == kleNCbNFVA){kOYkZbmnZp = true;}
      else if(kleNCbNFVA == OJjVcgTBpy){fFOKARXMEr = true;}
      if(HkpsVqArqn == XfLMdyPnrj){EBpLMKxGOs = true;}
      else if(XfLMdyPnrj == HkpsVqArqn){cNooqlFwON = true;}
      if(ZSkJbsrEAP == RGdwmEDGeg){pGnyBKTQGA = true;}
      else if(RGdwmEDGeg == ZSkJbsrEAP){StSksFhiZA = true;}
      if(ESaoWdFuKA == MEtGwSLkKF){GgYIcWoYsy = true;}
      if(kHtruumBFm == uRgWDXTowM){HoFaNIlyAV = true;}
      if(huwYSuYZsj == GRaOodIiax){plcotdaZiO = true;}
      while(MEtGwSLkKF == ESaoWdFuKA){sTUtNSGGjr = true;}
      while(uRgWDXTowM == uRgWDXTowM){xqdVUhZAmD = true;}
      while(GRaOodIiax == GRaOodIiax){eKRdXkldQu = true;}
      if(YHZonSlFQL == true){YHZonSlFQL = false;}
      if(mddmnzZHrO == true){mddmnzZHrO = false;}
      if(kOJSZuceRR == true){kOJSZuceRR = false;}
      if(JcKMfnrrij == true){JcKMfnrrij = false;}
      if(kOYkZbmnZp == true){kOYkZbmnZp = false;}
      if(EBpLMKxGOs == true){EBpLMKxGOs = false;}
      if(pGnyBKTQGA == true){pGnyBKTQGA = false;}
      if(GgYIcWoYsy == true){GgYIcWoYsy = false;}
      if(HoFaNIlyAV == true){HoFaNIlyAV = false;}
      if(plcotdaZiO == true){plcotdaZiO = false;}
      if(HVgYiWcgio == true){HVgYiWcgio = false;}
      if(UXNQkBBMks == true){UXNQkBBMks = false;}
      if(XVQkKUDiOy == true){XVQkKUDiOy = false;}
      if(wOpdpkHgrp == true){wOpdpkHgrp = false;}
      if(fFOKARXMEr == true){fFOKARXMEr = false;}
      if(cNooqlFwON == true){cNooqlFwON = false;}
      if(StSksFhiZA == true){StSksFhiZA = false;}
      if(sTUtNSGGjr == true){sTUtNSGGjr = false;}
      if(xqdVUhZAmD == true){xqdVUhZAmD = false;}
      if(eKRdXkldQu == true){eKRdXkldQu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class MIVUHQAJHG
{ 
  void LZbqSWrGDE()
  { 
      bool eftglgawdN = false;
      bool HMQyGaWxDy = false;
      bool JaJNOYVdEV = false;
      bool oidOiTWDiK = false;
      bool dxoUfiHxtJ = false;
      bool IRYgUDWgSN = false;
      bool KxrHYXiCOm = false;
      bool ESNrULlngf = false;
      bool RXitNXKwte = false;
      bool xpkVzgFhwH = false;
      bool AFhHFoDfMJ = false;
      bool RhzmwmNxsA = false;
      bool MlIjpOhrsn = false;
      bool GErYeLoboY = false;
      bool TejkNOpyYM = false;
      bool SHnuuNxyRp = false;
      bool HLmXCMpZHj = false;
      bool xlHUghBWVa = false;
      bool XjSfUyPwdR = false;
      bool OpJyRIqLhA = false;
      string BFtLttzlnM;
      string usOakCaXlS;
      string fhuyxltQJr;
      string CCEWjdMPbP;
      string FXzdupGbHU;
      string OfwxXSKuNI;
      string zhbuKroqqH;
      string pLuWMHiRDH;
      string fSIndWGGzQ;
      string SQBTQXuDSC;
      string BOjAeCjYjP;
      string isrGbfZnRn;
      string khrZnYKbGo;
      string MfyAGRhHIb;
      string lhWigZYtqj;
      string yHleRSVJMG;
      string qHStDDtrAK;
      string xRfoBDLJsO;
      string AQkVqFtSim;
      string uYuBkeObed;
      if(BFtLttzlnM == BOjAeCjYjP){eftglgawdN = true;}
      else if(BOjAeCjYjP == BFtLttzlnM){AFhHFoDfMJ = true;}
      if(usOakCaXlS == isrGbfZnRn){HMQyGaWxDy = true;}
      else if(isrGbfZnRn == usOakCaXlS){RhzmwmNxsA = true;}
      if(fhuyxltQJr == khrZnYKbGo){JaJNOYVdEV = true;}
      else if(khrZnYKbGo == fhuyxltQJr){MlIjpOhrsn = true;}
      if(CCEWjdMPbP == MfyAGRhHIb){oidOiTWDiK = true;}
      else if(MfyAGRhHIb == CCEWjdMPbP){GErYeLoboY = true;}
      if(FXzdupGbHU == lhWigZYtqj){dxoUfiHxtJ = true;}
      else if(lhWigZYtqj == FXzdupGbHU){TejkNOpyYM = true;}
      if(OfwxXSKuNI == yHleRSVJMG){IRYgUDWgSN = true;}
      else if(yHleRSVJMG == OfwxXSKuNI){SHnuuNxyRp = true;}
      if(zhbuKroqqH == qHStDDtrAK){KxrHYXiCOm = true;}
      else if(qHStDDtrAK == zhbuKroqqH){HLmXCMpZHj = true;}
      if(pLuWMHiRDH == xRfoBDLJsO){ESNrULlngf = true;}
      if(fSIndWGGzQ == AQkVqFtSim){RXitNXKwte = true;}
      if(SQBTQXuDSC == uYuBkeObed){xpkVzgFhwH = true;}
      while(xRfoBDLJsO == pLuWMHiRDH){xlHUghBWVa = true;}
      while(AQkVqFtSim == AQkVqFtSim){XjSfUyPwdR = true;}
      while(uYuBkeObed == uYuBkeObed){OpJyRIqLhA = true;}
      if(eftglgawdN == true){eftglgawdN = false;}
      if(HMQyGaWxDy == true){HMQyGaWxDy = false;}
      if(JaJNOYVdEV == true){JaJNOYVdEV = false;}
      if(oidOiTWDiK == true){oidOiTWDiK = false;}
      if(dxoUfiHxtJ == true){dxoUfiHxtJ = false;}
      if(IRYgUDWgSN == true){IRYgUDWgSN = false;}
      if(KxrHYXiCOm == true){KxrHYXiCOm = false;}
      if(ESNrULlngf == true){ESNrULlngf = false;}
      if(RXitNXKwte == true){RXitNXKwte = false;}
      if(xpkVzgFhwH == true){xpkVzgFhwH = false;}
      if(AFhHFoDfMJ == true){AFhHFoDfMJ = false;}
      if(RhzmwmNxsA == true){RhzmwmNxsA = false;}
      if(MlIjpOhrsn == true){MlIjpOhrsn = false;}
      if(GErYeLoboY == true){GErYeLoboY = false;}
      if(TejkNOpyYM == true){TejkNOpyYM = false;}
      if(SHnuuNxyRp == true){SHnuuNxyRp = false;}
      if(HLmXCMpZHj == true){HLmXCMpZHj = false;}
      if(xlHUghBWVa == true){xlHUghBWVa = false;}
      if(XjSfUyPwdR == true){XjSfUyPwdR = false;}
      if(OpJyRIqLhA == true){OpJyRIqLhA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WIKYMEJMVF
{ 
  void dlwPziwqGc()
  { 
      bool NngeAgIUQw = false;
      bool tBGHjLhsjQ = false;
      bool ALgjUbRQiA = false;
      bool PuTdwAYTND = false;
      bool MtNQRQnycN = false;
      bool CATMpCGxJX = false;
      bool PqlPgzhbKj = false;
      bool mNefEGyQRE = false;
      bool nSnUyOaGmt = false;
      bool sbJomUHxiS = false;
      bool iLmsanVmIj = false;
      bool EpeFjwDQRS = false;
      bool XaoQpLEosi = false;
      bool yBgzVbeutK = false;
      bool snyDpDwEFp = false;
      bool QpfQuHWWCO = false;
      bool iftYkDJeiI = false;
      bool RIWGZickam = false;
      bool OzjCRHUPEO = false;
      bool EIreaMtYRm = false;
      string iGSYAIitKI;
      string LzzuaUUnCu;
      string PAzPzSggxN;
      string tBoWzizFCt;
      string DBbQcboCZP;
      string ShDDJhGCYg;
      string aCxwNKJbur;
      string UPkHqNYimW;
      string deGguMykNR;
      string bxfUTBEeCS;
      string RNGhuCwuLw;
      string JWOBDuwQBA;
      string MYOdPWjQmp;
      string JsgTFQXXlr;
      string PCYQHEidRY;
      string FmdBGsycSX;
      string SfIVmPptwm;
      string XXeIkJzxVK;
      string nGyOkVOeNL;
      string RuFNZjtDdq;
      if(iGSYAIitKI == RNGhuCwuLw){NngeAgIUQw = true;}
      else if(RNGhuCwuLw == iGSYAIitKI){iLmsanVmIj = true;}
      if(LzzuaUUnCu == JWOBDuwQBA){tBGHjLhsjQ = true;}
      else if(JWOBDuwQBA == LzzuaUUnCu){EpeFjwDQRS = true;}
      if(PAzPzSggxN == MYOdPWjQmp){ALgjUbRQiA = true;}
      else if(MYOdPWjQmp == PAzPzSggxN){XaoQpLEosi = true;}
      if(tBoWzizFCt == JsgTFQXXlr){PuTdwAYTND = true;}
      else if(JsgTFQXXlr == tBoWzizFCt){yBgzVbeutK = true;}
      if(DBbQcboCZP == PCYQHEidRY){MtNQRQnycN = true;}
      else if(PCYQHEidRY == DBbQcboCZP){snyDpDwEFp = true;}
      if(ShDDJhGCYg == FmdBGsycSX){CATMpCGxJX = true;}
      else if(FmdBGsycSX == ShDDJhGCYg){QpfQuHWWCO = true;}
      if(aCxwNKJbur == SfIVmPptwm){PqlPgzhbKj = true;}
      else if(SfIVmPptwm == aCxwNKJbur){iftYkDJeiI = true;}
      if(UPkHqNYimW == XXeIkJzxVK){mNefEGyQRE = true;}
      if(deGguMykNR == nGyOkVOeNL){nSnUyOaGmt = true;}
      if(bxfUTBEeCS == RuFNZjtDdq){sbJomUHxiS = true;}
      while(XXeIkJzxVK == UPkHqNYimW){RIWGZickam = true;}
      while(nGyOkVOeNL == nGyOkVOeNL){OzjCRHUPEO = true;}
      while(RuFNZjtDdq == RuFNZjtDdq){EIreaMtYRm = true;}
      if(NngeAgIUQw == true){NngeAgIUQw = false;}
      if(tBGHjLhsjQ == true){tBGHjLhsjQ = false;}
      if(ALgjUbRQiA == true){ALgjUbRQiA = false;}
      if(PuTdwAYTND == true){PuTdwAYTND = false;}
      if(MtNQRQnycN == true){MtNQRQnycN = false;}
      if(CATMpCGxJX == true){CATMpCGxJX = false;}
      if(PqlPgzhbKj == true){PqlPgzhbKj = false;}
      if(mNefEGyQRE == true){mNefEGyQRE = false;}
      if(nSnUyOaGmt == true){nSnUyOaGmt = false;}
      if(sbJomUHxiS == true){sbJomUHxiS = false;}
      if(iLmsanVmIj == true){iLmsanVmIj = false;}
      if(EpeFjwDQRS == true){EpeFjwDQRS = false;}
      if(XaoQpLEosi == true){XaoQpLEosi = false;}
      if(yBgzVbeutK == true){yBgzVbeutK = false;}
      if(snyDpDwEFp == true){snyDpDwEFp = false;}
      if(QpfQuHWWCO == true){QpfQuHWWCO = false;}
      if(iftYkDJeiI == true){iftYkDJeiI = false;}
      if(RIWGZickam == true){RIWGZickam = false;}
      if(OzjCRHUPEO == true){OzjCRHUPEO = false;}
      if(EIreaMtYRm == true){EIreaMtYRm = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FRZULLODTU
{ 
  void tkgjMqynmu()
  { 
      bool QYFTSAnATX = false;
      bool hFgwPKxXBs = false;
      bool saCdtEaKoE = false;
      bool JkJAwuzIJp = false;
      bool KBTySQGsnr = false;
      bool FPOxRoVaZN = false;
      bool YdLOOwwgAT = false;
      bool LnNKhrHELc = false;
      bool yhWwztWLhH = false;
      bool YXiuQgpEjV = false;
      bool TYsTgQCDOY = false;
      bool gRPwPqGOPT = false;
      bool ZkyrunFRtB = false;
      bool CCUoJYoDPh = false;
      bool rTwJZHHlfY = false;
      bool ZaoPuiZAYr = false;
      bool KjJNWewZmq = false;
      bool CDyhbGnoSn = false;
      bool fTlUaaNFsa = false;
      bool yneWWHKhmu = false;
      string pMWekhbLCG;
      string rTuKcEfhQr;
      string zuBLmXSIVR;
      string AhBfsXYqpw;
      string BsrUPKFbGg;
      string sgHrBuLLqM;
      string OSokGKSOQE;
      string fnrUqFDfuc;
      string SqBnXTZIey;
      string LNzdhhDHOE;
      string rpKCBEomAu;
      string qfPOsXQAMm;
      string jJjSrRoNlI;
      string CwQpjcfzmT;
      string dmIjGfxjkJ;
      string RCtbUiGrJi;
      string PaArsuPYeQ;
      string qQNsIwBALM;
      string TEccqzBnVA;
      string WmpDDDmPqL;
      if(pMWekhbLCG == rpKCBEomAu){QYFTSAnATX = true;}
      else if(rpKCBEomAu == pMWekhbLCG){TYsTgQCDOY = true;}
      if(rTuKcEfhQr == qfPOsXQAMm){hFgwPKxXBs = true;}
      else if(qfPOsXQAMm == rTuKcEfhQr){gRPwPqGOPT = true;}
      if(zuBLmXSIVR == jJjSrRoNlI){saCdtEaKoE = true;}
      else if(jJjSrRoNlI == zuBLmXSIVR){ZkyrunFRtB = true;}
      if(AhBfsXYqpw == CwQpjcfzmT){JkJAwuzIJp = true;}
      else if(CwQpjcfzmT == AhBfsXYqpw){CCUoJYoDPh = true;}
      if(BsrUPKFbGg == dmIjGfxjkJ){KBTySQGsnr = true;}
      else if(dmIjGfxjkJ == BsrUPKFbGg){rTwJZHHlfY = true;}
      if(sgHrBuLLqM == RCtbUiGrJi){FPOxRoVaZN = true;}
      else if(RCtbUiGrJi == sgHrBuLLqM){ZaoPuiZAYr = true;}
      if(OSokGKSOQE == PaArsuPYeQ){YdLOOwwgAT = true;}
      else if(PaArsuPYeQ == OSokGKSOQE){KjJNWewZmq = true;}
      if(fnrUqFDfuc == qQNsIwBALM){LnNKhrHELc = true;}
      if(SqBnXTZIey == TEccqzBnVA){yhWwztWLhH = true;}
      if(LNzdhhDHOE == WmpDDDmPqL){YXiuQgpEjV = true;}
      while(qQNsIwBALM == fnrUqFDfuc){CDyhbGnoSn = true;}
      while(TEccqzBnVA == TEccqzBnVA){fTlUaaNFsa = true;}
      while(WmpDDDmPqL == WmpDDDmPqL){yneWWHKhmu = true;}
      if(QYFTSAnATX == true){QYFTSAnATX = false;}
      if(hFgwPKxXBs == true){hFgwPKxXBs = false;}
      if(saCdtEaKoE == true){saCdtEaKoE = false;}
      if(JkJAwuzIJp == true){JkJAwuzIJp = false;}
      if(KBTySQGsnr == true){KBTySQGsnr = false;}
      if(FPOxRoVaZN == true){FPOxRoVaZN = false;}
      if(YdLOOwwgAT == true){YdLOOwwgAT = false;}
      if(LnNKhrHELc == true){LnNKhrHELc = false;}
      if(yhWwztWLhH == true){yhWwztWLhH = false;}
      if(YXiuQgpEjV == true){YXiuQgpEjV = false;}
      if(TYsTgQCDOY == true){TYsTgQCDOY = false;}
      if(gRPwPqGOPT == true){gRPwPqGOPT = false;}
      if(ZkyrunFRtB == true){ZkyrunFRtB = false;}
      if(CCUoJYoDPh == true){CCUoJYoDPh = false;}
      if(rTwJZHHlfY == true){rTwJZHHlfY = false;}
      if(ZaoPuiZAYr == true){ZaoPuiZAYr = false;}
      if(KjJNWewZmq == true){KjJNWewZmq = false;}
      if(CDyhbGnoSn == true){CDyhbGnoSn = false;}
      if(fTlUaaNFsa == true){fTlUaaNFsa = false;}
      if(yneWWHKhmu == true){yneWWHKhmu = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class CAOWOQDJDO
{ 
  void xnRxFnranr()
  { 
      bool FdRusWmlyl = false;
      bool LepIGtENCB = false;
      bool jmkwAnOWxN = false;
      bool TmmHFLUDuu = false;
      bool DEOneHJKVm = false;
      bool UAmkSSrVuE = false;
      bool HQnkSYqiCt = false;
      bool ZASHyfXxQr = false;
      bool dEAKsSzBol = false;
      bool TyIpmmPcVF = false;
      bool QaHaMTheml = false;
      bool uuEDaDPcrz = false;
      bool YptQaarcTL = false;
      bool PtbLbDwYxY = false;
      bool jTNCqXtFOj = false;
      bool WlkVgZfqAl = false;
      bool aFLbVASsaW = false;
      bool HpfSPEbrYC = false;
      bool gmNjndJFdB = false;
      bool WFYLxdIlfq = false;
      string EZnJFNbVHg;
      string ypLLWnSyCV;
      string QFcUfVfgzb;
      string BKxFLAOBzG;
      string WeneGWRoHA;
      string plUrFzwTcj;
      string guVeensFAV;
      string ZQywzkQyNJ;
      string DpYzlPoEzL;
      string EuYPYaIJAS;
      string cLolaYmYsT;
      string hyFTfpVWrO;
      string ynFnReriWX;
      string bHytfrBmXN;
      string cQLedGoFih;
      string FlQkToFATu;
      string gEBUraALmm;
      string ohyelhcaKL;
      string olznSppMZq;
      string mASbgYOpmC;
      if(EZnJFNbVHg == cLolaYmYsT){FdRusWmlyl = true;}
      else if(cLolaYmYsT == EZnJFNbVHg){QaHaMTheml = true;}
      if(ypLLWnSyCV == hyFTfpVWrO){LepIGtENCB = true;}
      else if(hyFTfpVWrO == ypLLWnSyCV){uuEDaDPcrz = true;}
      if(QFcUfVfgzb == ynFnReriWX){jmkwAnOWxN = true;}
      else if(ynFnReriWX == QFcUfVfgzb){YptQaarcTL = true;}
      if(BKxFLAOBzG == bHytfrBmXN){TmmHFLUDuu = true;}
      else if(bHytfrBmXN == BKxFLAOBzG){PtbLbDwYxY = true;}
      if(WeneGWRoHA == cQLedGoFih){DEOneHJKVm = true;}
      else if(cQLedGoFih == WeneGWRoHA){jTNCqXtFOj = true;}
      if(plUrFzwTcj == FlQkToFATu){UAmkSSrVuE = true;}
      else if(FlQkToFATu == plUrFzwTcj){WlkVgZfqAl = true;}
      if(guVeensFAV == gEBUraALmm){HQnkSYqiCt = true;}
      else if(gEBUraALmm == guVeensFAV){aFLbVASsaW = true;}
      if(ZQywzkQyNJ == ohyelhcaKL){ZASHyfXxQr = true;}
      if(DpYzlPoEzL == olznSppMZq){dEAKsSzBol = true;}
      if(EuYPYaIJAS == mASbgYOpmC){TyIpmmPcVF = true;}
      while(ohyelhcaKL == ZQywzkQyNJ){HpfSPEbrYC = true;}
      while(olznSppMZq == olznSppMZq){gmNjndJFdB = true;}
      while(mASbgYOpmC == mASbgYOpmC){WFYLxdIlfq = true;}
      if(FdRusWmlyl == true){FdRusWmlyl = false;}
      if(LepIGtENCB == true){LepIGtENCB = false;}
      if(jmkwAnOWxN == true){jmkwAnOWxN = false;}
      if(TmmHFLUDuu == true){TmmHFLUDuu = false;}
      if(DEOneHJKVm == true){DEOneHJKVm = false;}
      if(UAmkSSrVuE == true){UAmkSSrVuE = false;}
      if(HQnkSYqiCt == true){HQnkSYqiCt = false;}
      if(ZASHyfXxQr == true){ZASHyfXxQr = false;}
      if(dEAKsSzBol == true){dEAKsSzBol = false;}
      if(TyIpmmPcVF == true){TyIpmmPcVF = false;}
      if(QaHaMTheml == true){QaHaMTheml = false;}
      if(uuEDaDPcrz == true){uuEDaDPcrz = false;}
      if(YptQaarcTL == true){YptQaarcTL = false;}
      if(PtbLbDwYxY == true){PtbLbDwYxY = false;}
      if(jTNCqXtFOj == true){jTNCqXtFOj = false;}
      if(WlkVgZfqAl == true){WlkVgZfqAl = false;}
      if(aFLbVASsaW == true){aFLbVASsaW = false;}
      if(HpfSPEbrYC == true){HpfSPEbrYC = false;}
      if(gmNjndJFdB == true){gmNjndJFdB = false;}
      if(WFYLxdIlfq == true){WFYLxdIlfq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class WOENAJNYKZ
{ 
  void csDtAxRuTb()
  { 
      bool EwerubCxhX = false;
      bool ZDjSjWmFtY = false;
      bool TBlELljjRG = false;
      bool QkAYIIdeEj = false;
      bool lMJnfQFfkW = false;
      bool PLhppDJagJ = false;
      bool qSfaHnDKKa = false;
      bool rIAJlHqyXu = false;
      bool fDytpEDJiE = false;
      bool zDxurTghgq = false;
      bool MtVKeHKXEh = false;
      bool LLClcmTkjC = false;
      bool ktaFboKFQR = false;
      bool aIQfhlhnPi = false;
      bool seEBsmALRF = false;
      bool zuydFSSQio = false;
      bool wFGlHCtVDq = false;
      bool FfUBIGoZMn = false;
      bool ozmLSeKlZP = false;
      bool SRFalnXRtp = false;
      string TohBRrkLaF;
      string FpEMsSZAaJ;
      string EEKnSwEAyh;
      string NEkVBGGdMu;
      string LjnILouDdA;
      string bsGZfOetiU;
      string mkMrrBFiKm;
      string LunexUPBcd;
      string ABVyGTxKom;
      string bLurIaWwQJ;
      string iMrqnfFbcL;
      string ydRFTkOOlX;
      string DusXKzzuWW;
      string xTjStnJeJZ;
      string NVhdnytpXW;
      string LHJrGnTTfn;
      string fKEFNdsyKf;
      string bjFLKhNPiM;
      string EJWzLoKqMG;
      string TozUtDQbPJ;
      if(TohBRrkLaF == iMrqnfFbcL){EwerubCxhX = true;}
      else if(iMrqnfFbcL == TohBRrkLaF){MtVKeHKXEh = true;}
      if(FpEMsSZAaJ == ydRFTkOOlX){ZDjSjWmFtY = true;}
      else if(ydRFTkOOlX == FpEMsSZAaJ){LLClcmTkjC = true;}
      if(EEKnSwEAyh == DusXKzzuWW){TBlELljjRG = true;}
      else if(DusXKzzuWW == EEKnSwEAyh){ktaFboKFQR = true;}
      if(NEkVBGGdMu == xTjStnJeJZ){QkAYIIdeEj = true;}
      else if(xTjStnJeJZ == NEkVBGGdMu){aIQfhlhnPi = true;}
      if(LjnILouDdA == NVhdnytpXW){lMJnfQFfkW = true;}
      else if(NVhdnytpXW == LjnILouDdA){seEBsmALRF = true;}
      if(bsGZfOetiU == LHJrGnTTfn){PLhppDJagJ = true;}
      else if(LHJrGnTTfn == bsGZfOetiU){zuydFSSQio = true;}
      if(mkMrrBFiKm == fKEFNdsyKf){qSfaHnDKKa = true;}
      else if(fKEFNdsyKf == mkMrrBFiKm){wFGlHCtVDq = true;}
      if(LunexUPBcd == bjFLKhNPiM){rIAJlHqyXu = true;}
      if(ABVyGTxKom == EJWzLoKqMG){fDytpEDJiE = true;}
      if(bLurIaWwQJ == TozUtDQbPJ){zDxurTghgq = true;}
      while(bjFLKhNPiM == LunexUPBcd){FfUBIGoZMn = true;}
      while(EJWzLoKqMG == EJWzLoKqMG){ozmLSeKlZP = true;}
      while(TozUtDQbPJ == TozUtDQbPJ){SRFalnXRtp = true;}
      if(EwerubCxhX == true){EwerubCxhX = false;}
      if(ZDjSjWmFtY == true){ZDjSjWmFtY = false;}
      if(TBlELljjRG == true){TBlELljjRG = false;}
      if(QkAYIIdeEj == true){QkAYIIdeEj = false;}
      if(lMJnfQFfkW == true){lMJnfQFfkW = false;}
      if(PLhppDJagJ == true){PLhppDJagJ = false;}
      if(qSfaHnDKKa == true){qSfaHnDKKa = false;}
      if(rIAJlHqyXu == true){rIAJlHqyXu = false;}
      if(fDytpEDJiE == true){fDytpEDJiE = false;}
      if(zDxurTghgq == true){zDxurTghgq = false;}
      if(MtVKeHKXEh == true){MtVKeHKXEh = false;}
      if(LLClcmTkjC == true){LLClcmTkjC = false;}
      if(ktaFboKFQR == true){ktaFboKFQR = false;}
      if(aIQfhlhnPi == true){aIQfhlhnPi = false;}
      if(seEBsmALRF == true){seEBsmALRF = false;}
      if(zuydFSSQio == true){zuydFSSQio = false;}
      if(wFGlHCtVDq == true){wFGlHCtVDq = false;}
      if(FfUBIGoZMn == true){FfUBIGoZMn = false;}
      if(ozmLSeKlZP == true){ozmLSeKlZP = false;}
      if(SRFalnXRtp == true){SRFalnXRtp = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class XICKYPTVEU
{ 
  void qkfJwDVYrb()
  { 
      bool IJCjoXSlur = false;
      bool BSoxnSqjKw = false;
      bool wTtFZySjSW = false;
      bool WftklKXtnj = false;
      bool BZJraokuZU = false;
      bool OwxBqnPxKp = false;
      bool FiTYVVXEKd = false;
      bool twwauVKBej = false;
      bool MWJxHahrsy = false;
      bool tajxdRGkWE = false;
      bool QgkwaSAWGU = false;
      bool dOSZdiZVQt = false;
      bool mfKMiSMywc = false;
      bool OYdZTfkMWo = false;
      bool aLlYTeIKTu = false;
      bool rkTbQkWYNH = false;
      bool FBZcFEkrFx = false;
      bool xMybTbskrD = false;
      bool KCeBkdFBIi = false;
      bool JMzPiYlISO = false;
      string DuhpUZiWOX;
      string CQSBEnxVWA;
      string TykBWfqiNm;
      string YMRsRsfCHH;
      string euKcfCsTeZ;
      string zfRgaEKfSn;
      string izIbypzZjK;
      string ItOOVXnHVU;
      string mWEnmsukRN;
      string iziSYelhdp;
      string DTjTRFQCKC;
      string PCKyBLuUoF;
      string uNHVbILupq;
      string lGdfQTmjWh;
      string braOxgnjRM;
      string iRsYtoLNrI;
      string tVMSQJYegg;
      string XKyGcCmrYu;
      string TadcIUMEkb;
      string HZrOnGfHyn;
      if(DuhpUZiWOX == DTjTRFQCKC){IJCjoXSlur = true;}
      else if(DTjTRFQCKC == DuhpUZiWOX){QgkwaSAWGU = true;}
      if(CQSBEnxVWA == PCKyBLuUoF){BSoxnSqjKw = true;}
      else if(PCKyBLuUoF == CQSBEnxVWA){dOSZdiZVQt = true;}
      if(TykBWfqiNm == uNHVbILupq){wTtFZySjSW = true;}
      else if(uNHVbILupq == TykBWfqiNm){mfKMiSMywc = true;}
      if(YMRsRsfCHH == lGdfQTmjWh){WftklKXtnj = true;}
      else if(lGdfQTmjWh == YMRsRsfCHH){OYdZTfkMWo = true;}
      if(euKcfCsTeZ == braOxgnjRM){BZJraokuZU = true;}
      else if(braOxgnjRM == euKcfCsTeZ){aLlYTeIKTu = true;}
      if(zfRgaEKfSn == iRsYtoLNrI){OwxBqnPxKp = true;}
      else if(iRsYtoLNrI == zfRgaEKfSn){rkTbQkWYNH = true;}
      if(izIbypzZjK == tVMSQJYegg){FiTYVVXEKd = true;}
      else if(tVMSQJYegg == izIbypzZjK){FBZcFEkrFx = true;}
      if(ItOOVXnHVU == XKyGcCmrYu){twwauVKBej = true;}
      if(mWEnmsukRN == TadcIUMEkb){MWJxHahrsy = true;}
      if(iziSYelhdp == HZrOnGfHyn){tajxdRGkWE = true;}
      while(XKyGcCmrYu == ItOOVXnHVU){xMybTbskrD = true;}
      while(TadcIUMEkb == TadcIUMEkb){KCeBkdFBIi = true;}
      while(HZrOnGfHyn == HZrOnGfHyn){JMzPiYlISO = true;}
      if(IJCjoXSlur == true){IJCjoXSlur = false;}
      if(BSoxnSqjKw == true){BSoxnSqjKw = false;}
      if(wTtFZySjSW == true){wTtFZySjSW = false;}
      if(WftklKXtnj == true){WftklKXtnj = false;}
      if(BZJraokuZU == true){BZJraokuZU = false;}
      if(OwxBqnPxKp == true){OwxBqnPxKp = false;}
      if(FiTYVVXEKd == true){FiTYVVXEKd = false;}
      if(twwauVKBej == true){twwauVKBej = false;}
      if(MWJxHahrsy == true){MWJxHahrsy = false;}
      if(tajxdRGkWE == true){tajxdRGkWE = false;}
      if(QgkwaSAWGU == true){QgkwaSAWGU = false;}
      if(dOSZdiZVQt == true){dOSZdiZVQt = false;}
      if(mfKMiSMywc == true){mfKMiSMywc = false;}
      if(OYdZTfkMWo == true){OYdZTfkMWo = false;}
      if(aLlYTeIKTu == true){aLlYTeIKTu = false;}
      if(rkTbQkWYNH == true){rkTbQkWYNH = false;}
      if(FBZcFEkrFx == true){FBZcFEkrFx = false;}
      if(xMybTbskrD == true){xMybTbskrD = false;}
      if(KCeBkdFBIi == true){KCeBkdFBIi = false;}
      if(JMzPiYlISO == true){JMzPiYlISO = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PCQTIYQZJG
{ 
  void yolRGuSALM()
  { 
      bool sTdYkOLbYA = false;
      bool WMmlunSGkq = false;
      bool GQlJEdmCrm = false;
      bool YjlnskGCxO = false;
      bool msbkoAsGVr = false;
      bool MqxKIbhTTH = false;
      bool aIikbmZzsK = false;
      bool gQIWENoATE = false;
      bool jfgAaglbeQ = false;
      bool ClhpmeJpJC = false;
      bool ZtNLKuiaGu = false;
      bool mgzPWtaiKt = false;
      bool AyTbHuiRiY = false;
      bool EzqRdpSVoe = false;
      bool yLUjdVlSTr = false;
      bool DaePBbZkDP = false;
      bool MQqgpnphkf = false;
      bool bFsfuocjyl = false;
      bool IQyqVdTbPB = false;
      bool PgRZSgUmso = false;
      string UYfKDdQkXH;
      string KCVbTcYjfH;
      string NVnBYZkajh;
      string eoyboDEMyW;
      string lSUMGMcZlU;
      string UueKPyjdlc;
      string soApbVEslP;
      string XLkJVuAbuL;
      string xKoOMjJmMp;
      string SaaUdwRejj;
      string IimVxIJVoT;
      string yIiZehAlcr;
      string LRgwwxkQeZ;
      string dVqKDOXSRE;
      string ZnyrrKrAow;
      string weBqYpBqYA;
      string XspOpbQXWQ;
      string KgTYSXSoEl;
      string EjaicyUAlM;
      string MfsQffyiCP;
      if(UYfKDdQkXH == IimVxIJVoT){sTdYkOLbYA = true;}
      else if(IimVxIJVoT == UYfKDdQkXH){ZtNLKuiaGu = true;}
      if(KCVbTcYjfH == yIiZehAlcr){WMmlunSGkq = true;}
      else if(yIiZehAlcr == KCVbTcYjfH){mgzPWtaiKt = true;}
      if(NVnBYZkajh == LRgwwxkQeZ){GQlJEdmCrm = true;}
      else if(LRgwwxkQeZ == NVnBYZkajh){AyTbHuiRiY = true;}
      if(eoyboDEMyW == dVqKDOXSRE){YjlnskGCxO = true;}
      else if(dVqKDOXSRE == eoyboDEMyW){EzqRdpSVoe = true;}
      if(lSUMGMcZlU == ZnyrrKrAow){msbkoAsGVr = true;}
      else if(ZnyrrKrAow == lSUMGMcZlU){yLUjdVlSTr = true;}
      if(UueKPyjdlc == weBqYpBqYA){MqxKIbhTTH = true;}
      else if(weBqYpBqYA == UueKPyjdlc){DaePBbZkDP = true;}
      if(soApbVEslP == XspOpbQXWQ){aIikbmZzsK = true;}
      else if(XspOpbQXWQ == soApbVEslP){MQqgpnphkf = true;}
      if(XLkJVuAbuL == KgTYSXSoEl){gQIWENoATE = true;}
      if(xKoOMjJmMp == EjaicyUAlM){jfgAaglbeQ = true;}
      if(SaaUdwRejj == MfsQffyiCP){ClhpmeJpJC = true;}
      while(KgTYSXSoEl == XLkJVuAbuL){bFsfuocjyl = true;}
      while(EjaicyUAlM == EjaicyUAlM){IQyqVdTbPB = true;}
      while(MfsQffyiCP == MfsQffyiCP){PgRZSgUmso = true;}
      if(sTdYkOLbYA == true){sTdYkOLbYA = false;}
      if(WMmlunSGkq == true){WMmlunSGkq = false;}
      if(GQlJEdmCrm == true){GQlJEdmCrm = false;}
      if(YjlnskGCxO == true){YjlnskGCxO = false;}
      if(msbkoAsGVr == true){msbkoAsGVr = false;}
      if(MqxKIbhTTH == true){MqxKIbhTTH = false;}
      if(aIikbmZzsK == true){aIikbmZzsK = false;}
      if(gQIWENoATE == true){gQIWENoATE = false;}
      if(jfgAaglbeQ == true){jfgAaglbeQ = false;}
      if(ClhpmeJpJC == true){ClhpmeJpJC = false;}
      if(ZtNLKuiaGu == true){ZtNLKuiaGu = false;}
      if(mgzPWtaiKt == true){mgzPWtaiKt = false;}
      if(AyTbHuiRiY == true){AyTbHuiRiY = false;}
      if(EzqRdpSVoe == true){EzqRdpSVoe = false;}
      if(yLUjdVlSTr == true){yLUjdVlSTr = false;}
      if(DaePBbZkDP == true){DaePBbZkDP = false;}
      if(MQqgpnphkf == true){MQqgpnphkf = false;}
      if(bFsfuocjyl == true){bFsfuocjyl = false;}
      if(IQyqVdTbPB == true){IQyqVdTbPB = false;}
      if(PgRZSgUmso == true){PgRZSgUmso = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class LSMTOGSWWU
{ 
  void rlfCFslBez()
  { 
      bool AfrQLwCDRe = false;
      bool NNupsxWQVp = false;
      bool lBVYLONuCA = false;
      bool KIdXIGXVYo = false;
      bool JEnxbBciMm = false;
      bool lrhsSyJTEx = false;
      bool hHAQTEiXhm = false;
      bool ZAqgggNYxK = false;
      bool lehPNRakEK = false;
      bool wERbGBEXdW = false;
      bool RFBYNlAALl = false;
      bool jkWsKDYkDB = false;
      bool dySXqUiPae = false;
      bool cpuZsgcJBb = false;
      bool QZYSZLhYQC = false;
      bool cTUbrSldhr = false;
      bool DkYgItowzd = false;
      bool ZPiajDZzxe = false;
      bool mJyXCJVSha = false;
      bool tzaJYfArJe = false;
      string EABaXPWbDL;
      string mDgpnJoWtW;
      string WfYTfrcTEf;
      string cHncRZmflX;
      string BPCmLPcbeT;
      string tXJyYXbhte;
      string mDXXheODXO;
      string VIjVUHdlHQ;
      string OBojkFypDt;
      string xEkGkyqdEf;
      string AZDPxlmmdR;
      string toZdPkNnmc;
      string fYzdQRXdkV;
      string PzhbPoxYlz;
      string SLLlAHeGqz;
      string kDqbzaaYzf;
      string OJCHCtHyOa;
      string mnBnJUMNdI;
      string lQlkkLIFbo;
      string OYZGkJJEiF;
      if(EABaXPWbDL == AZDPxlmmdR){AfrQLwCDRe = true;}
      else if(AZDPxlmmdR == EABaXPWbDL){RFBYNlAALl = true;}
      if(mDgpnJoWtW == toZdPkNnmc){NNupsxWQVp = true;}
      else if(toZdPkNnmc == mDgpnJoWtW){jkWsKDYkDB = true;}
      if(WfYTfrcTEf == fYzdQRXdkV){lBVYLONuCA = true;}
      else if(fYzdQRXdkV == WfYTfrcTEf){dySXqUiPae = true;}
      if(cHncRZmflX == PzhbPoxYlz){KIdXIGXVYo = true;}
      else if(PzhbPoxYlz == cHncRZmflX){cpuZsgcJBb = true;}
      if(BPCmLPcbeT == SLLlAHeGqz){JEnxbBciMm = true;}
      else if(SLLlAHeGqz == BPCmLPcbeT){QZYSZLhYQC = true;}
      if(tXJyYXbhte == kDqbzaaYzf){lrhsSyJTEx = true;}
      else if(kDqbzaaYzf == tXJyYXbhte){cTUbrSldhr = true;}
      if(mDXXheODXO == OJCHCtHyOa){hHAQTEiXhm = true;}
      else if(OJCHCtHyOa == mDXXheODXO){DkYgItowzd = true;}
      if(VIjVUHdlHQ == mnBnJUMNdI){ZAqgggNYxK = true;}
      if(OBojkFypDt == lQlkkLIFbo){lehPNRakEK = true;}
      if(xEkGkyqdEf == OYZGkJJEiF){wERbGBEXdW = true;}
      while(mnBnJUMNdI == VIjVUHdlHQ){ZPiajDZzxe = true;}
      while(lQlkkLIFbo == lQlkkLIFbo){mJyXCJVSha = true;}
      while(OYZGkJJEiF == OYZGkJJEiF){tzaJYfArJe = true;}
      if(AfrQLwCDRe == true){AfrQLwCDRe = false;}
      if(NNupsxWQVp == true){NNupsxWQVp = false;}
      if(lBVYLONuCA == true){lBVYLONuCA = false;}
      if(KIdXIGXVYo == true){KIdXIGXVYo = false;}
      if(JEnxbBciMm == true){JEnxbBciMm = false;}
      if(lrhsSyJTEx == true){lrhsSyJTEx = false;}
      if(hHAQTEiXhm == true){hHAQTEiXhm = false;}
      if(ZAqgggNYxK == true){ZAqgggNYxK = false;}
      if(lehPNRakEK == true){lehPNRakEK = false;}
      if(wERbGBEXdW == true){wERbGBEXdW = false;}
      if(RFBYNlAALl == true){RFBYNlAALl = false;}
      if(jkWsKDYkDB == true){jkWsKDYkDB = false;}
      if(dySXqUiPae == true){dySXqUiPae = false;}
      if(cpuZsgcJBb == true){cpuZsgcJBb = false;}
      if(QZYSZLhYQC == true){QZYSZLhYQC = false;}
      if(cTUbrSldhr == true){cTUbrSldhr = false;}
      if(DkYgItowzd == true){DkYgItowzd = false;}
      if(ZPiajDZzxe == true){ZPiajDZzxe = false;}
      if(mJyXCJVSha == true){mJyXCJVSha = false;}
      if(tzaJYfArJe == true){tzaJYfArJe = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class PHWVWRECVP
{ 
  void lzyHaYhFnS()
  { 
      bool QXlZRHkbGe = false;
      bool yawYhFlqgG = false;
      bool RWIJzXBFAO = false;
      bool DEQQmeVOHT = false;
      bool TAfPZAuYpm = false;
      bool DKjEFsObas = false;
      bool GfarasKekD = false;
      bool iWiNmQqsjy = false;
      bool bbPNLsTLLt = false;
      bool PsnONmZmnf = false;
      bool pPnmDfaVQn = false;
      bool BVrbRdQMgh = false;
      bool ZyTqmoiUAz = false;
      bool qDEEIMnbKe = false;
      bool RxtWrtBLwi = false;
      bool WMGGGqwxzN = false;
      bool wGVRxyQmTE = false;
      bool onCBQbEwyr = false;
      bool mXXQcdxxxR = false;
      bool PqcwhKbcZt = false;
      string kCmGXXLjty;
      string FeMehRoiFE;
      string ncQPcmRCXC;
      string toUIRUfVoc;
      string oPLkZXGBUA;
      string ILzFEIlWck;
      string GILrEPKDNM;
      string lTtcOXQeoE;
      string gPhXtipPEI;
      string hIrEWgDTgK;
      string NCnHOnQWpf;
      string qHhYEpZnyV;
      string ZtzEecaoFh;
      string UxxeqdaaJF;
      string RSYXQaaTAf;
      string GPUKkSCjck;
      string MbQILtJChZ;
      string OzdsEGUbmE;
      string CHoYaEcltX;
      string aqxYroCQLW;
      if(kCmGXXLjty == NCnHOnQWpf){QXlZRHkbGe = true;}
      else if(NCnHOnQWpf == kCmGXXLjty){pPnmDfaVQn = true;}
      if(FeMehRoiFE == qHhYEpZnyV){yawYhFlqgG = true;}
      else if(qHhYEpZnyV == FeMehRoiFE){BVrbRdQMgh = true;}
      if(ncQPcmRCXC == ZtzEecaoFh){RWIJzXBFAO = true;}
      else if(ZtzEecaoFh == ncQPcmRCXC){ZyTqmoiUAz = true;}
      if(toUIRUfVoc == UxxeqdaaJF){DEQQmeVOHT = true;}
      else if(UxxeqdaaJF == toUIRUfVoc){qDEEIMnbKe = true;}
      if(oPLkZXGBUA == RSYXQaaTAf){TAfPZAuYpm = true;}
      else if(RSYXQaaTAf == oPLkZXGBUA){RxtWrtBLwi = true;}
      if(ILzFEIlWck == GPUKkSCjck){DKjEFsObas = true;}
      else if(GPUKkSCjck == ILzFEIlWck){WMGGGqwxzN = true;}
      if(GILrEPKDNM == MbQILtJChZ){GfarasKekD = true;}
      else if(MbQILtJChZ == GILrEPKDNM){wGVRxyQmTE = true;}
      if(lTtcOXQeoE == OzdsEGUbmE){iWiNmQqsjy = true;}
      if(gPhXtipPEI == CHoYaEcltX){bbPNLsTLLt = true;}
      if(hIrEWgDTgK == aqxYroCQLW){PsnONmZmnf = true;}
      while(OzdsEGUbmE == lTtcOXQeoE){onCBQbEwyr = true;}
      while(CHoYaEcltX == CHoYaEcltX){mXXQcdxxxR = true;}
      while(aqxYroCQLW == aqxYroCQLW){PqcwhKbcZt = true;}
      if(QXlZRHkbGe == true){QXlZRHkbGe = false;}
      if(yawYhFlqgG == true){yawYhFlqgG = false;}
      if(RWIJzXBFAO == true){RWIJzXBFAO = false;}
      if(DEQQmeVOHT == true){DEQQmeVOHT = false;}
      if(TAfPZAuYpm == true){TAfPZAuYpm = false;}
      if(DKjEFsObas == true){DKjEFsObas = false;}
      if(GfarasKekD == true){GfarasKekD = false;}
      if(iWiNmQqsjy == true){iWiNmQqsjy = false;}
      if(bbPNLsTLLt == true){bbPNLsTLLt = false;}
      if(PsnONmZmnf == true){PsnONmZmnf = false;}
      if(pPnmDfaVQn == true){pPnmDfaVQn = false;}
      if(BVrbRdQMgh == true){BVrbRdQMgh = false;}
      if(ZyTqmoiUAz == true){ZyTqmoiUAz = false;}
      if(qDEEIMnbKe == true){qDEEIMnbKe = false;}
      if(RxtWrtBLwi == true){RxtWrtBLwi = false;}
      if(WMGGGqwxzN == true){WMGGGqwxzN = false;}
      if(wGVRxyQmTE == true){wGVRxyQmTE = false;}
      if(onCBQbEwyr == true){onCBQbEwyr = false;}
      if(mXXQcdxxxR == true){mXXQcdxxxR = false;}
      if(PqcwhKbcZt == true){PqcwhKbcZt = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class SFNDRIIBYP
{ 
  void KpyZEwyJwA()
  { 
      bool LsqRjuGKby = false;
      bool ORJRzGOuCZ = false;
      bool zGaUjVDYAq = false;
      bool WprFHbZyzn = false;
      bool sewXHBRMfE = false;
      bool uoLtNFSWgO = false;
      bool rhKjaXYDJW = false;
      bool gQuNfUKDMi = false;
      bool EprwwgeyrC = false;
      bool ICQkwLULxi = false;
      bool bNCIWdsxNs = false;
      bool tPJyJBoNms = false;
      bool raLGOeAKRT = false;
      bool zzFdGpniZR = false;
      bool mgjzIsrRau = false;
      bool WGXDpBCwtq = false;
      bool diftstJfqT = false;
      bool MFnVFKpMxU = false;
      bool mhjVxsaCmF = false;
      bool mqohmEBpFN = false;
      string iazdddCbAZ;
      string sfSLAYXNnV;
      string UBdTjxZqqw;
      string eJfjlSwIEm;
      string jxWLNXQXdK;
      string nZxiHgxesg;
      string DKjDtjAhZg;
      string cOJBkXeRFM;
      string fCMgIELwcE;
      string VQOWiZNSRd;
      string WiyxgYQwxj;
      string CeYfABxDuo;
      string XDFVKMAlRL;
      string JMAboxDPzj;
      string nPzsCqqiVh;
      string bJWcGSdblU;
      string XeekZoexqD;
      string xgDLAHELST;
      string VRPiiApXVV;
      string SbdeLJkNTM;
      if(iazdddCbAZ == WiyxgYQwxj){LsqRjuGKby = true;}
      else if(WiyxgYQwxj == iazdddCbAZ){bNCIWdsxNs = true;}
      if(sfSLAYXNnV == CeYfABxDuo){ORJRzGOuCZ = true;}
      else if(CeYfABxDuo == sfSLAYXNnV){tPJyJBoNms = true;}
      if(UBdTjxZqqw == XDFVKMAlRL){zGaUjVDYAq = true;}
      else if(XDFVKMAlRL == UBdTjxZqqw){raLGOeAKRT = true;}
      if(eJfjlSwIEm == JMAboxDPzj){WprFHbZyzn = true;}
      else if(JMAboxDPzj == eJfjlSwIEm){zzFdGpniZR = true;}
      if(jxWLNXQXdK == nPzsCqqiVh){sewXHBRMfE = true;}
      else if(nPzsCqqiVh == jxWLNXQXdK){mgjzIsrRau = true;}
      if(nZxiHgxesg == bJWcGSdblU){uoLtNFSWgO = true;}
      else if(bJWcGSdblU == nZxiHgxesg){WGXDpBCwtq = true;}
      if(DKjDtjAhZg == XeekZoexqD){rhKjaXYDJW = true;}
      else if(XeekZoexqD == DKjDtjAhZg){diftstJfqT = true;}
      if(cOJBkXeRFM == xgDLAHELST){gQuNfUKDMi = true;}
      if(fCMgIELwcE == VRPiiApXVV){EprwwgeyrC = true;}
      if(VQOWiZNSRd == SbdeLJkNTM){ICQkwLULxi = true;}
      while(xgDLAHELST == cOJBkXeRFM){MFnVFKpMxU = true;}
      while(VRPiiApXVV == VRPiiApXVV){mhjVxsaCmF = true;}
      while(SbdeLJkNTM == SbdeLJkNTM){mqohmEBpFN = true;}
      if(LsqRjuGKby == true){LsqRjuGKby = false;}
      if(ORJRzGOuCZ == true){ORJRzGOuCZ = false;}
      if(zGaUjVDYAq == true){zGaUjVDYAq = false;}
      if(WprFHbZyzn == true){WprFHbZyzn = false;}
      if(sewXHBRMfE == true){sewXHBRMfE = false;}
      if(uoLtNFSWgO == true){uoLtNFSWgO = false;}
      if(rhKjaXYDJW == true){rhKjaXYDJW = false;}
      if(gQuNfUKDMi == true){gQuNfUKDMi = false;}
      if(EprwwgeyrC == true){EprwwgeyrC = false;}
      if(ICQkwLULxi == true){ICQkwLULxi = false;}
      if(bNCIWdsxNs == true){bNCIWdsxNs = false;}
      if(tPJyJBoNms == true){tPJyJBoNms = false;}
      if(raLGOeAKRT == true){raLGOeAKRT = false;}
      if(zzFdGpniZR == true){zzFdGpniZR = false;}
      if(mgjzIsrRau == true){mgjzIsrRau = false;}
      if(WGXDpBCwtq == true){WGXDpBCwtq = false;}
      if(diftstJfqT == true){diftstJfqT = false;}
      if(MFnVFKpMxU == true){MFnVFKpMxU = false;}
      if(mhjVxsaCmF == true){mhjVxsaCmF = false;}
      if(mqohmEBpFN == true){mqohmEBpFN = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DZAEXWUBAE
{ 
  void MOGaTLBXOY()
  { 
      bool YARUFZKRFN = false;
      bool ZzwMNOwKDk = false;
      bool BjNhddBlHX = false;
      bool nTGeMKWBzM = false;
      bool KUFBfodAoP = false;
      bool RnBKIolipa = false;
      bool XYKhYFsRAC = false;
      bool ruKpageLBo = false;
      bool AAxqGzSuVX = false;
      bool XmKXnlJWca = false;
      bool uMAlXVdKib = false;
      bool JZgBaXbTbY = false;
      bool mxucdEjlBo = false;
      bool WHiasmAKSj = false;
      bool VIMsNyFJwQ = false;
      bool KpuDmyQWHT = false;
      bool lqItECpFIn = false;
      bool WQArDekCPf = false;
      bool zaojefMfnt = false;
      bool VdCzxezSnL = false;
      string ZOpDrjuNaT;
      string WQRmGxVGYK;
      string xiBahlfzLb;
      string qWejUflXBf;
      string OPVnFGosfX;
      string ugZPHqMOVG;
      string EwCzcAkhSa;
      string LxEsugMNVa;
      string xmSmfJwPcd;
      string TWBubSNWKE;
      string UGaFMybgTZ;
      string dysuIRcOSS;
      string DVFnGnuJQc;
      string TpCQFpMMag;
      string gPwCzUcGmK;
      string oyDqBzBlzX;
      string CSQqGnucoS;
      string EPnllnYCol;
      string KnVFmUlLOG;
      string naNCdNzFwK;
      if(ZOpDrjuNaT == UGaFMybgTZ){YARUFZKRFN = true;}
      else if(UGaFMybgTZ == ZOpDrjuNaT){uMAlXVdKib = true;}
      if(WQRmGxVGYK == dysuIRcOSS){ZzwMNOwKDk = true;}
      else if(dysuIRcOSS == WQRmGxVGYK){JZgBaXbTbY = true;}
      if(xiBahlfzLb == DVFnGnuJQc){BjNhddBlHX = true;}
      else if(DVFnGnuJQc == xiBahlfzLb){mxucdEjlBo = true;}
      if(qWejUflXBf == TpCQFpMMag){nTGeMKWBzM = true;}
      else if(TpCQFpMMag == qWejUflXBf){WHiasmAKSj = true;}
      if(OPVnFGosfX == gPwCzUcGmK){KUFBfodAoP = true;}
      else if(gPwCzUcGmK == OPVnFGosfX){VIMsNyFJwQ = true;}
      if(ugZPHqMOVG == oyDqBzBlzX){RnBKIolipa = true;}
      else if(oyDqBzBlzX == ugZPHqMOVG){KpuDmyQWHT = true;}
      if(EwCzcAkhSa == CSQqGnucoS){XYKhYFsRAC = true;}
      else if(CSQqGnucoS == EwCzcAkhSa){lqItECpFIn = true;}
      if(LxEsugMNVa == EPnllnYCol){ruKpageLBo = true;}
      if(xmSmfJwPcd == KnVFmUlLOG){AAxqGzSuVX = true;}
      if(TWBubSNWKE == naNCdNzFwK){XmKXnlJWca = true;}
      while(EPnllnYCol == LxEsugMNVa){WQArDekCPf = true;}
      while(KnVFmUlLOG == KnVFmUlLOG){zaojefMfnt = true;}
      while(naNCdNzFwK == naNCdNzFwK){VdCzxezSnL = true;}
      if(YARUFZKRFN == true){YARUFZKRFN = false;}
      if(ZzwMNOwKDk == true){ZzwMNOwKDk = false;}
      if(BjNhddBlHX == true){BjNhddBlHX = false;}
      if(nTGeMKWBzM == true){nTGeMKWBzM = false;}
      if(KUFBfodAoP == true){KUFBfodAoP = false;}
      if(RnBKIolipa == true){RnBKIolipa = false;}
      if(XYKhYFsRAC == true){XYKhYFsRAC = false;}
      if(ruKpageLBo == true){ruKpageLBo = false;}
      if(AAxqGzSuVX == true){AAxqGzSuVX = false;}
      if(XmKXnlJWca == true){XmKXnlJWca = false;}
      if(uMAlXVdKib == true){uMAlXVdKib = false;}
      if(JZgBaXbTbY == true){JZgBaXbTbY = false;}
      if(mxucdEjlBo == true){mxucdEjlBo = false;}
      if(WHiasmAKSj == true){WHiasmAKSj = false;}
      if(VIMsNyFJwQ == true){VIMsNyFJwQ = false;}
      if(KpuDmyQWHT == true){KpuDmyQWHT = false;}
      if(lqItECpFIn == true){lqItECpFIn = false;}
      if(WQArDekCPf == true){WQArDekCPf = false;}
      if(zaojefMfnt == true){zaojefMfnt = false;}
      if(VdCzxezSnL == true){VdCzxezSnL = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BICBBSPJJN
{ 
  void umqlHUesBV()
  { 
      bool kgicwoffHH = false;
      bool MtkVBJwwqF = false;
      bool QeClUgBkPF = false;
      bool mmXXXIRUhd = false;
      bool CQBkkfXQtT = false;
      bool eVRxqFexHP = false;
      bool nuQCTYWNys = false;
      bool raNNDAcQAn = false;
      bool IkpwaQVZgR = false;
      bool cUMSoGzzQM = false;
      bool IKRBbDHDsq = false;
      bool cNwTGDEUbD = false;
      bool CxfgPWOtXx = false;
      bool AdAnfySnTW = false;
      bool ISgpnPYsMl = false;
      bool BeXNJLdjlF = false;
      bool oMewpzMInI = false;
      bool jQAEtiPpsz = false;
      bool yUhluUGmdT = false;
      bool ThBOhwmjzo = false;
      string GGgdtfSChg;
      string tPfdigNemC;
      string SpbQERUipd;
      string pFUlVppirP;
      string TrLjzVhaEx;
      string JDRqKPePYb;
      string KjLDqnusHs;
      string qdlWhaeqQh;
      string jsdsUmndaG;
      string LZJPEjfIsL;
      string anSLGeQtIi;
      string jmzqrRGShD;
      string zRSBgQTVSx;
      string uWyEHwaRlo;
      string VtUqVYaSXm;
      string dMZoMJVQep;
      string LtbLnNntju;
      string pngsgFwafI;
      string oByeThxJrx;
      string aJmudbSJDH;
      if(GGgdtfSChg == anSLGeQtIi){kgicwoffHH = true;}
      else if(anSLGeQtIi == GGgdtfSChg){IKRBbDHDsq = true;}
      if(tPfdigNemC == jmzqrRGShD){MtkVBJwwqF = true;}
      else if(jmzqrRGShD == tPfdigNemC){cNwTGDEUbD = true;}
      if(SpbQERUipd == zRSBgQTVSx){QeClUgBkPF = true;}
      else if(zRSBgQTVSx == SpbQERUipd){CxfgPWOtXx = true;}
      if(pFUlVppirP == uWyEHwaRlo){mmXXXIRUhd = true;}
      else if(uWyEHwaRlo == pFUlVppirP){AdAnfySnTW = true;}
      if(TrLjzVhaEx == VtUqVYaSXm){CQBkkfXQtT = true;}
      else if(VtUqVYaSXm == TrLjzVhaEx){ISgpnPYsMl = true;}
      if(JDRqKPePYb == dMZoMJVQep){eVRxqFexHP = true;}
      else if(dMZoMJVQep == JDRqKPePYb){BeXNJLdjlF = true;}
      if(KjLDqnusHs == LtbLnNntju){nuQCTYWNys = true;}
      else if(LtbLnNntju == KjLDqnusHs){oMewpzMInI = true;}
      if(qdlWhaeqQh == pngsgFwafI){raNNDAcQAn = true;}
      if(jsdsUmndaG == oByeThxJrx){IkpwaQVZgR = true;}
      if(LZJPEjfIsL == aJmudbSJDH){cUMSoGzzQM = true;}
      while(pngsgFwafI == qdlWhaeqQh){jQAEtiPpsz = true;}
      while(oByeThxJrx == oByeThxJrx){yUhluUGmdT = true;}
      while(aJmudbSJDH == aJmudbSJDH){ThBOhwmjzo = true;}
      if(kgicwoffHH == true){kgicwoffHH = false;}
      if(MtkVBJwwqF == true){MtkVBJwwqF = false;}
      if(QeClUgBkPF == true){QeClUgBkPF = false;}
      if(mmXXXIRUhd == true){mmXXXIRUhd = false;}
      if(CQBkkfXQtT == true){CQBkkfXQtT = false;}
      if(eVRxqFexHP == true){eVRxqFexHP = false;}
      if(nuQCTYWNys == true){nuQCTYWNys = false;}
      if(raNNDAcQAn == true){raNNDAcQAn = false;}
      if(IkpwaQVZgR == true){IkpwaQVZgR = false;}
      if(cUMSoGzzQM == true){cUMSoGzzQM = false;}
      if(IKRBbDHDsq == true){IKRBbDHDsq = false;}
      if(cNwTGDEUbD == true){cNwTGDEUbD = false;}
      if(CxfgPWOtXx == true){CxfgPWOtXx = false;}
      if(AdAnfySnTW == true){AdAnfySnTW = false;}
      if(ISgpnPYsMl == true){ISgpnPYsMl = false;}
      if(BeXNJLdjlF == true){BeXNJLdjlF = false;}
      if(oMewpzMInI == true){oMewpzMInI = false;}
      if(jQAEtiPpsz == true){jQAEtiPpsz = false;}
      if(yUhluUGmdT == true){yUhluUGmdT = false;}
      if(ThBOhwmjzo == true){ThBOhwmjzo = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class OXDDVGRWKN
{ 
  void ykgwAtpdaf()
  { 
      bool pmDeejwSbR = false;
      bool sPYcFyNacc = false;
      bool hIllxTgwgy = false;
      bool HrXsrMByCf = false;
      bool OapMeuNQVs = false;
      bool SoTAfgBQgK = false;
      bool CzldKArfjs = false;
      bool wPYwOGSYHC = false;
      bool GQZnfRYWNL = false;
      bool LkRrdOoLCd = false;
      bool DhfcwCQWlY = false;
      bool NbJeElZyxr = false;
      bool bUSYgLxgSz = false;
      bool RGVczXjxqc = false;
      bool BBknxiammr = false;
      bool opNKChPHuk = false;
      bool FjElGGGgef = false;
      bool BbGUpajday = false;
      bool jKsaKasLZQ = false;
      bool IcCGXRwygq = false;
      string GMkfurqziP;
      string QpFKRbiOSF;
      string bucskwVsAu;
      string fPwEMAgeXj;
      string nYytmKjZcG;
      string bzMkVKiquy;
      string WxzmjAsAyQ;
      string uEqhRDxiGa;
      string MuQsSgDLiC;
      string QojZNpYLJl;
      string RibLLXIdTR;
      string NAkUlafFTC;
      string xgRuzFTQUa;
      string dkYTIcTFhu;
      string XZqQoQEOnm;
      string KTqHojQfLG;
      string DqJsmbaQLJ;
      string HaXYeXpBGi;
      string AFchPKtReY;
      string LmsbqfQxcN;
      if(GMkfurqziP == RibLLXIdTR){pmDeejwSbR = true;}
      else if(RibLLXIdTR == GMkfurqziP){DhfcwCQWlY = true;}
      if(QpFKRbiOSF == NAkUlafFTC){sPYcFyNacc = true;}
      else if(NAkUlafFTC == QpFKRbiOSF){NbJeElZyxr = true;}
      if(bucskwVsAu == xgRuzFTQUa){hIllxTgwgy = true;}
      else if(xgRuzFTQUa == bucskwVsAu){bUSYgLxgSz = true;}
      if(fPwEMAgeXj == dkYTIcTFhu){HrXsrMByCf = true;}
      else if(dkYTIcTFhu == fPwEMAgeXj){RGVczXjxqc = true;}
      if(nYytmKjZcG == XZqQoQEOnm){OapMeuNQVs = true;}
      else if(XZqQoQEOnm == nYytmKjZcG){BBknxiammr = true;}
      if(bzMkVKiquy == KTqHojQfLG){SoTAfgBQgK = true;}
      else if(KTqHojQfLG == bzMkVKiquy){opNKChPHuk = true;}
      if(WxzmjAsAyQ == DqJsmbaQLJ){CzldKArfjs = true;}
      else if(DqJsmbaQLJ == WxzmjAsAyQ){FjElGGGgef = true;}
      if(uEqhRDxiGa == HaXYeXpBGi){wPYwOGSYHC = true;}
      if(MuQsSgDLiC == AFchPKtReY){GQZnfRYWNL = true;}
      if(QojZNpYLJl == LmsbqfQxcN){LkRrdOoLCd = true;}
      while(HaXYeXpBGi == uEqhRDxiGa){BbGUpajday = true;}
      while(AFchPKtReY == AFchPKtReY){jKsaKasLZQ = true;}
      while(LmsbqfQxcN == LmsbqfQxcN){IcCGXRwygq = true;}
      if(pmDeejwSbR == true){pmDeejwSbR = false;}
      if(sPYcFyNacc == true){sPYcFyNacc = false;}
      if(hIllxTgwgy == true){hIllxTgwgy = false;}
      if(HrXsrMByCf == true){HrXsrMByCf = false;}
      if(OapMeuNQVs == true){OapMeuNQVs = false;}
      if(SoTAfgBQgK == true){SoTAfgBQgK = false;}
      if(CzldKArfjs == true){CzldKArfjs = false;}
      if(wPYwOGSYHC == true){wPYwOGSYHC = false;}
      if(GQZnfRYWNL == true){GQZnfRYWNL = false;}
      if(LkRrdOoLCd == true){LkRrdOoLCd = false;}
      if(DhfcwCQWlY == true){DhfcwCQWlY = false;}
      if(NbJeElZyxr == true){NbJeElZyxr = false;}
      if(bUSYgLxgSz == true){bUSYgLxgSz = false;}
      if(RGVczXjxqc == true){RGVczXjxqc = false;}
      if(BBknxiammr == true){BBknxiammr = false;}
      if(opNKChPHuk == true){opNKChPHuk = false;}
      if(FjElGGGgef == true){FjElGGGgef = false;}
      if(BbGUpajday == true){BbGUpajday = false;}
      if(jKsaKasLZQ == true){jKsaKasLZQ = false;}
      if(IcCGXRwygq == true){IcCGXRwygq = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NEWVTGEVYY
{ 
  void tgIFfMGCmV()
  { 
      bool oTghcSGMWj = false;
      bool MoHSoylcsc = false;
      bool hTbuBKnSZC = false;
      bool emrQlewlnf = false;
      bool STYQKBqZAG = false;
      bool VIEAlUqlfR = false;
      bool enlTFYxqfJ = false;
      bool CznNKOuLkA = false;
      bool SNRSgWiqdR = false;
      bool mKGVBegFLk = false;
      bool DAtqEWTqpX = false;
      bool ZfAeyQWTlh = false;
      bool pFMOWqrRMf = false;
      bool gpAzwSTaOK = false;
      bool CrBGqHJaXj = false;
      bool INSctymeTN = false;
      bool dBdKpifAhf = false;
      bool wwFbfxDGWO = false;
      bool gRtbWTXWFy = false;
      bool enGAfBNqQH = false;
      string zHgKLTJSQY;
      string mEidDSprmd;
      string rjfKbDMXSO;
      string iQuHlYxXbu;
      string IBGwPgoGHE;
      string ZRoNhPbWiI;
      string HlABpVIbre;
      string USPqVpcitt;
      string OGQNnQbhYW;
      string xZnTnVBEGt;
      string YNNbiGCxrr;
      string mrcnsBVlMY;
      string taCUguduDz;
      string oXcAVBrndh;
      string OeogjFwAal;
      string AOqgzyKsun;
      string cGsrDOPcSr;
      string JgIeKPkVkb;
      string ydUQzZfuAX;
      string pyspFHkxdU;
      if(zHgKLTJSQY == YNNbiGCxrr){oTghcSGMWj = true;}
      else if(YNNbiGCxrr == zHgKLTJSQY){DAtqEWTqpX = true;}
      if(mEidDSprmd == mrcnsBVlMY){MoHSoylcsc = true;}
      else if(mrcnsBVlMY == mEidDSprmd){ZfAeyQWTlh = true;}
      if(rjfKbDMXSO == taCUguduDz){hTbuBKnSZC = true;}
      else if(taCUguduDz == rjfKbDMXSO){pFMOWqrRMf = true;}
      if(iQuHlYxXbu == oXcAVBrndh){emrQlewlnf = true;}
      else if(oXcAVBrndh == iQuHlYxXbu){gpAzwSTaOK = true;}
      if(IBGwPgoGHE == OeogjFwAal){STYQKBqZAG = true;}
      else if(OeogjFwAal == IBGwPgoGHE){CrBGqHJaXj = true;}
      if(ZRoNhPbWiI == AOqgzyKsun){VIEAlUqlfR = true;}
      else if(AOqgzyKsun == ZRoNhPbWiI){INSctymeTN = true;}
      if(HlABpVIbre == cGsrDOPcSr){enlTFYxqfJ = true;}
      else if(cGsrDOPcSr == HlABpVIbre){dBdKpifAhf = true;}
      if(USPqVpcitt == JgIeKPkVkb){CznNKOuLkA = true;}
      if(OGQNnQbhYW == ydUQzZfuAX){SNRSgWiqdR = true;}
      if(xZnTnVBEGt == pyspFHkxdU){mKGVBegFLk = true;}
      while(JgIeKPkVkb == USPqVpcitt){wwFbfxDGWO = true;}
      while(ydUQzZfuAX == ydUQzZfuAX){gRtbWTXWFy = true;}
      while(pyspFHkxdU == pyspFHkxdU){enGAfBNqQH = true;}
      if(oTghcSGMWj == true){oTghcSGMWj = false;}
      if(MoHSoylcsc == true){MoHSoylcsc = false;}
      if(hTbuBKnSZC == true){hTbuBKnSZC = false;}
      if(emrQlewlnf == true){emrQlewlnf = false;}
      if(STYQKBqZAG == true){STYQKBqZAG = false;}
      if(VIEAlUqlfR == true){VIEAlUqlfR = false;}
      if(enlTFYxqfJ == true){enlTFYxqfJ = false;}
      if(CznNKOuLkA == true){CznNKOuLkA = false;}
      if(SNRSgWiqdR == true){SNRSgWiqdR = false;}
      if(mKGVBegFLk == true){mKGVBegFLk = false;}
      if(DAtqEWTqpX == true){DAtqEWTqpX = false;}
      if(ZfAeyQWTlh == true){ZfAeyQWTlh = false;}
      if(pFMOWqrRMf == true){pFMOWqrRMf = false;}
      if(gpAzwSTaOK == true){gpAzwSTaOK = false;}
      if(CrBGqHJaXj == true){CrBGqHJaXj = false;}
      if(INSctymeTN == true){INSctymeTN = false;}
      if(dBdKpifAhf == true){dBdKpifAhf = false;}
      if(wwFbfxDGWO == true){wwFbfxDGWO = false;}
      if(gRtbWTXWFy == true){gRtbWTXWFy = false;}
      if(enGAfBNqQH == true){enGAfBNqQH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HXWECLZRJR
{ 
  void hKXSqxHhDh()
  { 
      bool nWmlKqBRHT = false;
      bool WExfeymsNJ = false;
      bool LYlrDSwdJL = false;
      bool IIjusIlOfp = false;
      bool noaPNaaUaO = false;
      bool caioRPjthq = false;
      bool tXIwKOUBmW = false;
      bool MXMXVWxWec = false;
      bool rbCuihYucc = false;
      bool XrgouQgeEz = false;
      bool GYgEBENKTu = false;
      bool RNOFzSUNMc = false;
      bool oZXKyTqXcF = false;
      bool pRSfVSuxsd = false;
      bool TNahKKcbpc = false;
      bool uSRaKfAWGh = false;
      bool MohNFAuiig = false;
      bool XXUiIaypzm = false;
      bool qPhAKWAqUo = false;
      bool RXqKQefNbK = false;
      string kzLsTuMGEV;
      string MzbubQdVAn;
      string KirDQXRuBD;
      string ZutSNcAhXR;
      string RsuKtefbpz;
      string HKMlrCHIbw;
      string cREBwQFIhz;
      string PuyhVQngIl;
      string qoFBwPiHZK;
      string VrGaoAosOd;
      string ZWtqiLLdPb;
      string NZzpEmLCFf;
      string hxhWcXoWsV;
      string btReMUuORj;
      string EFryxwrPVC;
      string nlmiwbnWNL;
      string WnktzSHUcj;
      string qxjVbcFiZg;
      string fYBqgoGiij;
      string fYAGMPTxzd;
      if(kzLsTuMGEV == ZWtqiLLdPb){nWmlKqBRHT = true;}
      else if(ZWtqiLLdPb == kzLsTuMGEV){GYgEBENKTu = true;}
      if(MzbubQdVAn == NZzpEmLCFf){WExfeymsNJ = true;}
      else if(NZzpEmLCFf == MzbubQdVAn){RNOFzSUNMc = true;}
      if(KirDQXRuBD == hxhWcXoWsV){LYlrDSwdJL = true;}
      else if(hxhWcXoWsV == KirDQXRuBD){oZXKyTqXcF = true;}
      if(ZutSNcAhXR == btReMUuORj){IIjusIlOfp = true;}
      else if(btReMUuORj == ZutSNcAhXR){pRSfVSuxsd = true;}
      if(RsuKtefbpz == EFryxwrPVC){noaPNaaUaO = true;}
      else if(EFryxwrPVC == RsuKtefbpz){TNahKKcbpc = true;}
      if(HKMlrCHIbw == nlmiwbnWNL){caioRPjthq = true;}
      else if(nlmiwbnWNL == HKMlrCHIbw){uSRaKfAWGh = true;}
      if(cREBwQFIhz == WnktzSHUcj){tXIwKOUBmW = true;}
      else if(WnktzSHUcj == cREBwQFIhz){MohNFAuiig = true;}
      if(PuyhVQngIl == qxjVbcFiZg){MXMXVWxWec = true;}
      if(qoFBwPiHZK == fYBqgoGiij){rbCuihYucc = true;}
      if(VrGaoAosOd == fYAGMPTxzd){XrgouQgeEz = true;}
      while(qxjVbcFiZg == PuyhVQngIl){XXUiIaypzm = true;}
      while(fYBqgoGiij == fYBqgoGiij){qPhAKWAqUo = true;}
      while(fYAGMPTxzd == fYAGMPTxzd){RXqKQefNbK = true;}
      if(nWmlKqBRHT == true){nWmlKqBRHT = false;}
      if(WExfeymsNJ == true){WExfeymsNJ = false;}
      if(LYlrDSwdJL == true){LYlrDSwdJL = false;}
      if(IIjusIlOfp == true){IIjusIlOfp = false;}
      if(noaPNaaUaO == true){noaPNaaUaO = false;}
      if(caioRPjthq == true){caioRPjthq = false;}
      if(tXIwKOUBmW == true){tXIwKOUBmW = false;}
      if(MXMXVWxWec == true){MXMXVWxWec = false;}
      if(rbCuihYucc == true){rbCuihYucc = false;}
      if(XrgouQgeEz == true){XrgouQgeEz = false;}
      if(GYgEBENKTu == true){GYgEBENKTu = false;}
      if(RNOFzSUNMc == true){RNOFzSUNMc = false;}
      if(oZXKyTqXcF == true){oZXKyTqXcF = false;}
      if(pRSfVSuxsd == true){pRSfVSuxsd = false;}
      if(TNahKKcbpc == true){TNahKKcbpc = false;}
      if(uSRaKfAWGh == true){uSRaKfAWGh = false;}
      if(MohNFAuiig == true){MohNFAuiig = false;}
      if(XXUiIaypzm == true){XXUiIaypzm = false;}
      if(qPhAKWAqUo == true){qPhAKWAqUo = false;}
      if(RXqKQefNbK == true){RXqKQefNbK = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HQPUZJPERJ
{ 
  void uxKQeVTpkJ()
  { 
      bool CMtSeLczGD = false;
      bool OMMtGdynAI = false;
      bool NDKCQEQdSC = false;
      bool mMyArqjpec = false;
      bool HFRQJQxyyh = false;
      bool cCkhFzCqCZ = false;
      bool tyfDPBNMCO = false;
      bool tHJWBHlssz = false;
      bool HRfjYkaQts = false;
      bool ornEhIYNAI = false;
      bool DjwjnVsOfx = false;
      bool MIyCScSude = false;
      bool awRIZBSFor = false;
      bool hYrkwWVuwD = false;
      bool rMjwgftlkF = false;
      bool odiVgGSmKl = false;
      bool CmZnOmQNGw = false;
      bool GWyamOLUbs = false;
      bool GfTiCEpWDo = false;
      bool EikKAEmwhs = false;
      string DRVrTusOxV;
      string RqeLtpyZmZ;
      string mfbcIlMDwR;
      string wBsbOETRSh;
      string DdNWTjejEb;
      string njHTAXorjg;
      string hpSeYPyCxo;
      string WgGEVISFIX;
      string HcyTcMtYbq;
      string OahkzMLhxE;
      string lblPFHcDyq;
      string EjiMYFryBM;
      string VYZGZuzVEJ;
      string xkIEkephOV;
      string ipajleqUMt;
      string UMfjiMCfoJ;
      string IHzosSbeas;
      string OJifdeyoTu;
      string DdJKnkFXUX;
      string cJsEMaqlqT;
      if(DRVrTusOxV == lblPFHcDyq){CMtSeLczGD = true;}
      else if(lblPFHcDyq == DRVrTusOxV){DjwjnVsOfx = true;}
      if(RqeLtpyZmZ == EjiMYFryBM){OMMtGdynAI = true;}
      else if(EjiMYFryBM == RqeLtpyZmZ){MIyCScSude = true;}
      if(mfbcIlMDwR == VYZGZuzVEJ){NDKCQEQdSC = true;}
      else if(VYZGZuzVEJ == mfbcIlMDwR){awRIZBSFor = true;}
      if(wBsbOETRSh == xkIEkephOV){mMyArqjpec = true;}
      else if(xkIEkephOV == wBsbOETRSh){hYrkwWVuwD = true;}
      if(DdNWTjejEb == ipajleqUMt){HFRQJQxyyh = true;}
      else if(ipajleqUMt == DdNWTjejEb){rMjwgftlkF = true;}
      if(njHTAXorjg == UMfjiMCfoJ){cCkhFzCqCZ = true;}
      else if(UMfjiMCfoJ == njHTAXorjg){odiVgGSmKl = true;}
      if(hpSeYPyCxo == IHzosSbeas){tyfDPBNMCO = true;}
      else if(IHzosSbeas == hpSeYPyCxo){CmZnOmQNGw = true;}
      if(WgGEVISFIX == OJifdeyoTu){tHJWBHlssz = true;}
      if(HcyTcMtYbq == DdJKnkFXUX){HRfjYkaQts = true;}
      if(OahkzMLhxE == cJsEMaqlqT){ornEhIYNAI = true;}
      while(OJifdeyoTu == WgGEVISFIX){GWyamOLUbs = true;}
      while(DdJKnkFXUX == DdJKnkFXUX){GfTiCEpWDo = true;}
      while(cJsEMaqlqT == cJsEMaqlqT){EikKAEmwhs = true;}
      if(CMtSeLczGD == true){CMtSeLczGD = false;}
      if(OMMtGdynAI == true){OMMtGdynAI = false;}
      if(NDKCQEQdSC == true){NDKCQEQdSC = false;}
      if(mMyArqjpec == true){mMyArqjpec = false;}
      if(HFRQJQxyyh == true){HFRQJQxyyh = false;}
      if(cCkhFzCqCZ == true){cCkhFzCqCZ = false;}
      if(tyfDPBNMCO == true){tyfDPBNMCO = false;}
      if(tHJWBHlssz == true){tHJWBHlssz = false;}
      if(HRfjYkaQts == true){HRfjYkaQts = false;}
      if(ornEhIYNAI == true){ornEhIYNAI = false;}
      if(DjwjnVsOfx == true){DjwjnVsOfx = false;}
      if(MIyCScSude == true){MIyCScSude = false;}
      if(awRIZBSFor == true){awRIZBSFor = false;}
      if(hYrkwWVuwD == true){hYrkwWVuwD = false;}
      if(rMjwgftlkF == true){rMjwgftlkF = false;}
      if(odiVgGSmKl == true){odiVgGSmKl = false;}
      if(CmZnOmQNGw == true){CmZnOmQNGw = false;}
      if(GWyamOLUbs == true){GWyamOLUbs = false;}
      if(GfTiCEpWDo == true){GfTiCEpWDo = false;}
      if(EikKAEmwhs == true){EikKAEmwhs = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BFAUXZQBTE
{ 
  void LbKBhmUFda()
  { 
      bool kmrQtarppM = false;
      bool EnZhCfdIBV = false;
      bool pZyDhWerYa = false;
      bool NYwAxWeDNj = false;
      bool anBkyXLsWu = false;
      bool buwRDLoniX = false;
      bool wzQOzerZke = false;
      bool BZtQuCYMFJ = false;
      bool KkVdyRbdwl = false;
      bool hqUpnaWllS = false;
      bool flIqtBAfVB = false;
      bool mbiYMKdmQh = false;
      bool DNcyxKFEbU = false;
      bool elZldXbeix = false;
      bool QwHuzFHwcc = false;
      bool xjVEPRJJsj = false;
      bool wwEBVACwun = false;
      bool izDSQHGgzq = false;
      bool CJbyXuQVWL = false;
      bool ORkyBUrFYy = false;
      string suIIScSBXD;
      string cqMCAfHLwH;
      string AHwpnSbUEj;
      string jxuDogcDpT;
      string fKMawCwqWL;
      string dADfHkzSUA;
      string AyxmTfuJNT;
      string DEUcaouUsk;
      string VkxsFitOuE;
      string hJhoShMYGQ;
      string HldDHZeZFR;
      string peZjdPADhk;
      string lfNIltVfKr;
      string bVNcdjGslf;
      string ETJdOCjyKC;
      string gJqVeaXURM;
      string DyPMoTRwGt;
      string IGRBfRWjti;
      string KWUHLSdqyC;
      string LlbYbFExnt;
      if(suIIScSBXD == HldDHZeZFR){kmrQtarppM = true;}
      else if(HldDHZeZFR == suIIScSBXD){flIqtBAfVB = true;}
      if(cqMCAfHLwH == peZjdPADhk){EnZhCfdIBV = true;}
      else if(peZjdPADhk == cqMCAfHLwH){mbiYMKdmQh = true;}
      if(AHwpnSbUEj == lfNIltVfKr){pZyDhWerYa = true;}
      else if(lfNIltVfKr == AHwpnSbUEj){DNcyxKFEbU = true;}
      if(jxuDogcDpT == bVNcdjGslf){NYwAxWeDNj = true;}
      else if(bVNcdjGslf == jxuDogcDpT){elZldXbeix = true;}
      if(fKMawCwqWL == ETJdOCjyKC){anBkyXLsWu = true;}
      else if(ETJdOCjyKC == fKMawCwqWL){QwHuzFHwcc = true;}
      if(dADfHkzSUA == gJqVeaXURM){buwRDLoniX = true;}
      else if(gJqVeaXURM == dADfHkzSUA){xjVEPRJJsj = true;}
      if(AyxmTfuJNT == DyPMoTRwGt){wzQOzerZke = true;}
      else if(DyPMoTRwGt == AyxmTfuJNT){wwEBVACwun = true;}
      if(DEUcaouUsk == IGRBfRWjti){BZtQuCYMFJ = true;}
      if(VkxsFitOuE == KWUHLSdqyC){KkVdyRbdwl = true;}
      if(hJhoShMYGQ == LlbYbFExnt){hqUpnaWllS = true;}
      while(IGRBfRWjti == DEUcaouUsk){izDSQHGgzq = true;}
      while(KWUHLSdqyC == KWUHLSdqyC){CJbyXuQVWL = true;}
      while(LlbYbFExnt == LlbYbFExnt){ORkyBUrFYy = true;}
      if(kmrQtarppM == true){kmrQtarppM = false;}
      if(EnZhCfdIBV == true){EnZhCfdIBV = false;}
      if(pZyDhWerYa == true){pZyDhWerYa = false;}
      if(NYwAxWeDNj == true){NYwAxWeDNj = false;}
      if(anBkyXLsWu == true){anBkyXLsWu = false;}
      if(buwRDLoniX == true){buwRDLoniX = false;}
      if(wzQOzerZke == true){wzQOzerZke = false;}
      if(BZtQuCYMFJ == true){BZtQuCYMFJ = false;}
      if(KkVdyRbdwl == true){KkVdyRbdwl = false;}
      if(hqUpnaWllS == true){hqUpnaWllS = false;}
      if(flIqtBAfVB == true){flIqtBAfVB = false;}
      if(mbiYMKdmQh == true){mbiYMKdmQh = false;}
      if(DNcyxKFEbU == true){DNcyxKFEbU = false;}
      if(elZldXbeix == true){elZldXbeix = false;}
      if(QwHuzFHwcc == true){QwHuzFHwcc = false;}
      if(xjVEPRJJsj == true){xjVEPRJJsj = false;}
      if(wwEBVACwun == true){wwEBVACwun = false;}
      if(izDSQHGgzq == true){izDSQHGgzq = false;}
      if(CJbyXuQVWL == true){CJbyXuQVWL = false;}
      if(ORkyBUrFYy == true){ORkyBUrFYy = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class TCGACGMUHL
{ 
  void PLBhFnoSlk()
  { 
      bool QtdoTmgWGO = false;
      bool lgswEWAZQS = false;
      bool PRXZomRQAP = false;
      bool YZDUMOCTmz = false;
      bool bhWTrNhQnq = false;
      bool xKGRLyVEzf = false;
      bool VQVppLcXio = false;
      bool UfmRqhtEga = false;
      bool FAcJSOSlJN = false;
      bool syTGcKLgcf = false;
      bool kJgyWQXoac = false;
      bool DQWXaUnMWb = false;
      bool spOUwWMgof = false;
      bool VNbApuSdlC = false;
      bool pdLfKbaueH = false;
      bool WzJBmttMCA = false;
      bool EqAtHooVhe = false;
      bool CJZUkPeiKT = false;
      bool uQBgIWEaNu = false;
      bool BGblyzcQYU = false;
      string RZkeWJdVgS;
      string RiCclIFSnk;
      string EusQomgMbP;
      string AFaRcnipIn;
      string zzkGKQEhLx;
      string gBUsfVgTpi;
      string cQsQSLHkAx;
      string YOJAlFKlGX;
      string iyAdiykhlH;
      string HOAZLhFTny;
      string JThOXRrUGa;
      string KtNSAdQPhT;
      string ZYzroKOinD;
      string zpyHNPPlGR;
      string GQaLwMBhZg;
      string RplTYjUoYN;
      string DJfDaUwGEj;
      string NdJPNJxmwc;
      string JfIGlIEdhd;
      string uaMrOxjXjb;
      if(RZkeWJdVgS == JThOXRrUGa){QtdoTmgWGO = true;}
      else if(JThOXRrUGa == RZkeWJdVgS){kJgyWQXoac = true;}
      if(RiCclIFSnk == KtNSAdQPhT){lgswEWAZQS = true;}
      else if(KtNSAdQPhT == RiCclIFSnk){DQWXaUnMWb = true;}
      if(EusQomgMbP == ZYzroKOinD){PRXZomRQAP = true;}
      else if(ZYzroKOinD == EusQomgMbP){spOUwWMgof = true;}
      if(AFaRcnipIn == zpyHNPPlGR){YZDUMOCTmz = true;}
      else if(zpyHNPPlGR == AFaRcnipIn){VNbApuSdlC = true;}
      if(zzkGKQEhLx == GQaLwMBhZg){bhWTrNhQnq = true;}
      else if(GQaLwMBhZg == zzkGKQEhLx){pdLfKbaueH = true;}
      if(gBUsfVgTpi == RplTYjUoYN){xKGRLyVEzf = true;}
      else if(RplTYjUoYN == gBUsfVgTpi){WzJBmttMCA = true;}
      if(cQsQSLHkAx == DJfDaUwGEj){VQVppLcXio = true;}
      else if(DJfDaUwGEj == cQsQSLHkAx){EqAtHooVhe = true;}
      if(YOJAlFKlGX == NdJPNJxmwc){UfmRqhtEga = true;}
      if(iyAdiykhlH == JfIGlIEdhd){FAcJSOSlJN = true;}
      if(HOAZLhFTny == uaMrOxjXjb){syTGcKLgcf = true;}
      while(NdJPNJxmwc == YOJAlFKlGX){CJZUkPeiKT = true;}
      while(JfIGlIEdhd == JfIGlIEdhd){uQBgIWEaNu = true;}
      while(uaMrOxjXjb == uaMrOxjXjb){BGblyzcQYU = true;}
      if(QtdoTmgWGO == true){QtdoTmgWGO = false;}
      if(lgswEWAZQS == true){lgswEWAZQS = false;}
      if(PRXZomRQAP == true){PRXZomRQAP = false;}
      if(YZDUMOCTmz == true){YZDUMOCTmz = false;}
      if(bhWTrNhQnq == true){bhWTrNhQnq = false;}
      if(xKGRLyVEzf == true){xKGRLyVEzf = false;}
      if(VQVppLcXio == true){VQVppLcXio = false;}
      if(UfmRqhtEga == true){UfmRqhtEga = false;}
      if(FAcJSOSlJN == true){FAcJSOSlJN = false;}
      if(syTGcKLgcf == true){syTGcKLgcf = false;}
      if(kJgyWQXoac == true){kJgyWQXoac = false;}
      if(DQWXaUnMWb == true){DQWXaUnMWb = false;}
      if(spOUwWMgof == true){spOUwWMgof = false;}
      if(VNbApuSdlC == true){VNbApuSdlC = false;}
      if(pdLfKbaueH == true){pdLfKbaueH = false;}
      if(WzJBmttMCA == true){WzJBmttMCA = false;}
      if(EqAtHooVhe == true){EqAtHooVhe = false;}
      if(CJZUkPeiKT == true){CJZUkPeiKT = false;}
      if(uQBgIWEaNu == true){uQBgIWEaNu = false;}
      if(BGblyzcQYU == true){BGblyzcQYU = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class GASLSSSBLP
{ 
  void iyQtqPbibf()
  { 
      bool ZjIfUJpBXI = false;
      bool JddhArktdm = false;
      bool GOFieULjwi = false;
      bool BmZpGlBrWu = false;
      bool oLspnyMtaJ = false;
      bool qSYTRKdYBA = false;
      bool xGFoGVEmbN = false;
      bool prWiyuVPuf = false;
      bool HRJlrhOGYw = false;
      bool HcpWxhGEiR = false;
      bool tYSNSzwoVV = false;
      bool nNcVrfxTwY = false;
      bool GyOeGNBzlP = false;
      bool nRgdliywSs = false;
      bool lZkCHszUeR = false;
      bool kmHJKwKnGF = false;
      bool xwCGhWgzjJ = false;
      bool hBADmgMLWM = false;
      bool NgUTdBJWSo = false;
      bool XKoMuoWdUk = false;
      string hyEKyowbto;
      string tqzCkYwfCn;
      string ujdUEyMWmg;
      string fTqUWJyaiD;
      string FBRsMdtdcH;
      string XUJTuyTiGz;
      string cZnNJZAeeb;
      string ZUxzhtRkIa;
      string DhseIKKXYY;
      string oOUjaeMEil;
      string cPzBgDXKkA;
      string giZeutgJEy;
      string OcFGlKANXC;
      string hMBVxnirxF;
      string mAlYrGnonf;
      string yCAjXAKrOV;
      string DKliSTgEgH;
      string LoLyLOqfOf;
      string VKYrEeMRyx;
      string cpcaCbNDGX;
      if(hyEKyowbto == cPzBgDXKkA){ZjIfUJpBXI = true;}
      else if(cPzBgDXKkA == hyEKyowbto){tYSNSzwoVV = true;}
      if(tqzCkYwfCn == giZeutgJEy){JddhArktdm = true;}
      else if(giZeutgJEy == tqzCkYwfCn){nNcVrfxTwY = true;}
      if(ujdUEyMWmg == OcFGlKANXC){GOFieULjwi = true;}
      else if(OcFGlKANXC == ujdUEyMWmg){GyOeGNBzlP = true;}
      if(fTqUWJyaiD == hMBVxnirxF){BmZpGlBrWu = true;}
      else if(hMBVxnirxF == fTqUWJyaiD){nRgdliywSs = true;}
      if(FBRsMdtdcH == mAlYrGnonf){oLspnyMtaJ = true;}
      else if(mAlYrGnonf == FBRsMdtdcH){lZkCHszUeR = true;}
      if(XUJTuyTiGz == yCAjXAKrOV){qSYTRKdYBA = true;}
      else if(yCAjXAKrOV == XUJTuyTiGz){kmHJKwKnGF = true;}
      if(cZnNJZAeeb == DKliSTgEgH){xGFoGVEmbN = true;}
      else if(DKliSTgEgH == cZnNJZAeeb){xwCGhWgzjJ = true;}
      if(ZUxzhtRkIa == LoLyLOqfOf){prWiyuVPuf = true;}
      if(DhseIKKXYY == VKYrEeMRyx){HRJlrhOGYw = true;}
      if(oOUjaeMEil == cpcaCbNDGX){HcpWxhGEiR = true;}
      while(LoLyLOqfOf == ZUxzhtRkIa){hBADmgMLWM = true;}
      while(VKYrEeMRyx == VKYrEeMRyx){NgUTdBJWSo = true;}
      while(cpcaCbNDGX == cpcaCbNDGX){XKoMuoWdUk = true;}
      if(ZjIfUJpBXI == true){ZjIfUJpBXI = false;}
      if(JddhArktdm == true){JddhArktdm = false;}
      if(GOFieULjwi == true){GOFieULjwi = false;}
      if(BmZpGlBrWu == true){BmZpGlBrWu = false;}
      if(oLspnyMtaJ == true){oLspnyMtaJ = false;}
      if(qSYTRKdYBA == true){qSYTRKdYBA = false;}
      if(xGFoGVEmbN == true){xGFoGVEmbN = false;}
      if(prWiyuVPuf == true){prWiyuVPuf = false;}
      if(HRJlrhOGYw == true){HRJlrhOGYw = false;}
      if(HcpWxhGEiR == true){HcpWxhGEiR = false;}
      if(tYSNSzwoVV == true){tYSNSzwoVV = false;}
      if(nNcVrfxTwY == true){nNcVrfxTwY = false;}
      if(GyOeGNBzlP == true){GyOeGNBzlP = false;}
      if(nRgdliywSs == true){nRgdliywSs = false;}
      if(lZkCHszUeR == true){lZkCHszUeR = false;}
      if(kmHJKwKnGF == true){kmHJKwKnGF = false;}
      if(xwCGhWgzjJ == true){xwCGhWgzjJ = false;}
      if(hBADmgMLWM == true){hBADmgMLWM = false;}
      if(NgUTdBJWSo == true){NgUTdBJWSo = false;}
      if(XKoMuoWdUk == true){XKoMuoWdUk = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class COVVJDDOHV
{ 
  void kaNFlkxNJa()
  { 
      bool UTOepxuYNg = false;
      bool WiOUmqFgxO = false;
      bool VDuCSpgukm = false;
      bool DuWfEGcRfT = false;
      bool ytTyzCLdcl = false;
      bool dcypaQXLLz = false;
      bool PAmZcaMiPg = false;
      bool NnBcPbiwCQ = false;
      bool dDSzIawgEm = false;
      bool hRFfDyGsVX = false;
      bool pcNwXpbUZz = false;
      bool hbJtUMgnRC = false;
      bool eedcywhUYQ = false;
      bool UoaeaBoVbS = false;
      bool ybGFFGIKKq = false;
      bool RSbSsKykxa = false;
      bool HGuJdrAdTT = false;
      bool XLLzECYXXr = false;
      bool nWJhzCNIkQ = false;
      bool CDYBKqfxbM = false;
      string JpgnTAQSTr;
      string RyKDHJaBap;
      string IpckhSmbOP;
      string RwbrCUrAkF;
      string rETTpjUJKA;
      string nyCHQSMwFA;
      string NoEDaUVSAm;
      string RZQVxDWJux;
      string ycQNbStUbj;
      string IwtkrCzPrU;
      string OBJXiOggJM;
      string wCBHBjYqbI;
      string aBpQdUefcu;
      string DGRZiwTreM;
      string kNKOmFumiH;
      string dInjtNLrsN;
      string iBDOJokaYE;
      string QfMOWNpbdY;
      string ZrwVRtSgfV;
      string kayQcjJuEx;
      if(JpgnTAQSTr == OBJXiOggJM){UTOepxuYNg = true;}
      else if(OBJXiOggJM == JpgnTAQSTr){pcNwXpbUZz = true;}
      if(RyKDHJaBap == wCBHBjYqbI){WiOUmqFgxO = true;}
      else if(wCBHBjYqbI == RyKDHJaBap){hbJtUMgnRC = true;}
      if(IpckhSmbOP == aBpQdUefcu){VDuCSpgukm = true;}
      else if(aBpQdUefcu == IpckhSmbOP){eedcywhUYQ = true;}
      if(RwbrCUrAkF == DGRZiwTreM){DuWfEGcRfT = true;}
      else if(DGRZiwTreM == RwbrCUrAkF){UoaeaBoVbS = true;}
      if(rETTpjUJKA == kNKOmFumiH){ytTyzCLdcl = true;}
      else if(kNKOmFumiH == rETTpjUJKA){ybGFFGIKKq = true;}
      if(nyCHQSMwFA == dInjtNLrsN){dcypaQXLLz = true;}
      else if(dInjtNLrsN == nyCHQSMwFA){RSbSsKykxa = true;}
      if(NoEDaUVSAm == iBDOJokaYE){PAmZcaMiPg = true;}
      else if(iBDOJokaYE == NoEDaUVSAm){HGuJdrAdTT = true;}
      if(RZQVxDWJux == QfMOWNpbdY){NnBcPbiwCQ = true;}
      if(ycQNbStUbj == ZrwVRtSgfV){dDSzIawgEm = true;}
      if(IwtkrCzPrU == kayQcjJuEx){hRFfDyGsVX = true;}
      while(QfMOWNpbdY == RZQVxDWJux){XLLzECYXXr = true;}
      while(ZrwVRtSgfV == ZrwVRtSgfV){nWJhzCNIkQ = true;}
      while(kayQcjJuEx == kayQcjJuEx){CDYBKqfxbM = true;}
      if(UTOepxuYNg == true){UTOepxuYNg = false;}
      if(WiOUmqFgxO == true){WiOUmqFgxO = false;}
      if(VDuCSpgukm == true){VDuCSpgukm = false;}
      if(DuWfEGcRfT == true){DuWfEGcRfT = false;}
      if(ytTyzCLdcl == true){ytTyzCLdcl = false;}
      if(dcypaQXLLz == true){dcypaQXLLz = false;}
      if(PAmZcaMiPg == true){PAmZcaMiPg = false;}
      if(NnBcPbiwCQ == true){NnBcPbiwCQ = false;}
      if(dDSzIawgEm == true){dDSzIawgEm = false;}
      if(hRFfDyGsVX == true){hRFfDyGsVX = false;}
      if(pcNwXpbUZz == true){pcNwXpbUZz = false;}
      if(hbJtUMgnRC == true){hbJtUMgnRC = false;}
      if(eedcywhUYQ == true){eedcywhUYQ = false;}
      if(UoaeaBoVbS == true){UoaeaBoVbS = false;}
      if(ybGFFGIKKq == true){ybGFFGIKKq = false;}
      if(RSbSsKykxa == true){RSbSsKykxa = false;}
      if(HGuJdrAdTT == true){HGuJdrAdTT = false;}
      if(XLLzECYXXr == true){XLLzECYXXr = false;}
      if(nWJhzCNIkQ == true){nWJhzCNIkQ = false;}
      if(CDYBKqfxbM == true){CDYBKqfxbM = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HLONZEZYIE
{ 
  void OGheXVQCfz()
  { 
      bool WluoODVPnl = false;
      bool UisUPeTCVj = false;
      bool tOcOqtqxBD = false;
      bool ATRQVUlUMe = false;
      bool wSVzpKxFNt = false;
      bool UdtVUIIgxT = false;
      bool LYNjlARczu = false;
      bool rzoqnascXg = false;
      bool wkiHiWYuuN = false;
      bool LFWsTWFkNp = false;
      bool sUGRBUnDhs = false;
      bool OeIQWZoGBO = false;
      bool aVhkimcwXQ = false;
      bool VHPhnbeQGN = false;
      bool hUNByfxGVs = false;
      bool gyjyOWmUhN = false;
      bool rrfQWHPZcu = false;
      bool uXzmgGHfmD = false;
      bool pWadjXXQTQ = false;
      bool nJhrVpoxdp = false;
      string sPWwIGrxIr;
      string MFhJJlzqyF;
      string RHSlbfntpo;
      string yDTckIHhqT;
      string ZeZkUyrYDK;
      string jPkoDlxkUb;
      string RxHTsJkEYA;
      string eRAdflghQd;
      string nPqfFhReYy;
      string LVNOCbXGqL;
      string MmzCeRPGIQ;
      string EPUqmBGaOK;
      string rGfZoIEkbE;
      string OapFQBPnAm;
      string SKVItwBVaA;
      string VwqfSFHYNW;
      string NdeECdYPZl;
      string kRaargkXfs;
      string GhJbhRiJlL;
      string tqadZofQFP;
      if(sPWwIGrxIr == MmzCeRPGIQ){WluoODVPnl = true;}
      else if(MmzCeRPGIQ == sPWwIGrxIr){sUGRBUnDhs = true;}
      if(MFhJJlzqyF == EPUqmBGaOK){UisUPeTCVj = true;}
      else if(EPUqmBGaOK == MFhJJlzqyF){OeIQWZoGBO = true;}
      if(RHSlbfntpo == rGfZoIEkbE){tOcOqtqxBD = true;}
      else if(rGfZoIEkbE == RHSlbfntpo){aVhkimcwXQ = true;}
      if(yDTckIHhqT == OapFQBPnAm){ATRQVUlUMe = true;}
      else if(OapFQBPnAm == yDTckIHhqT){VHPhnbeQGN = true;}
      if(ZeZkUyrYDK == SKVItwBVaA){wSVzpKxFNt = true;}
      else if(SKVItwBVaA == ZeZkUyrYDK){hUNByfxGVs = true;}
      if(jPkoDlxkUb == VwqfSFHYNW){UdtVUIIgxT = true;}
      else if(VwqfSFHYNW == jPkoDlxkUb){gyjyOWmUhN = true;}
      if(RxHTsJkEYA == NdeECdYPZl){LYNjlARczu = true;}
      else if(NdeECdYPZl == RxHTsJkEYA){rrfQWHPZcu = true;}
      if(eRAdflghQd == kRaargkXfs){rzoqnascXg = true;}
      if(nPqfFhReYy == GhJbhRiJlL){wkiHiWYuuN = true;}
      if(LVNOCbXGqL == tqadZofQFP){LFWsTWFkNp = true;}
      while(kRaargkXfs == eRAdflghQd){uXzmgGHfmD = true;}
      while(GhJbhRiJlL == GhJbhRiJlL){pWadjXXQTQ = true;}
      while(tqadZofQFP == tqadZofQFP){nJhrVpoxdp = true;}
      if(WluoODVPnl == true){WluoODVPnl = false;}
      if(UisUPeTCVj == true){UisUPeTCVj = false;}
      if(tOcOqtqxBD == true){tOcOqtqxBD = false;}
      if(ATRQVUlUMe == true){ATRQVUlUMe = false;}
      if(wSVzpKxFNt == true){wSVzpKxFNt = false;}
      if(UdtVUIIgxT == true){UdtVUIIgxT = false;}
      if(LYNjlARczu == true){LYNjlARczu = false;}
      if(rzoqnascXg == true){rzoqnascXg = false;}
      if(wkiHiWYuuN == true){wkiHiWYuuN = false;}
      if(LFWsTWFkNp == true){LFWsTWFkNp = false;}
      if(sUGRBUnDhs == true){sUGRBUnDhs = false;}
      if(OeIQWZoGBO == true){OeIQWZoGBO = false;}
      if(aVhkimcwXQ == true){aVhkimcwXQ = false;}
      if(VHPhnbeQGN == true){VHPhnbeQGN = false;}
      if(hUNByfxGVs == true){hUNByfxGVs = false;}
      if(gyjyOWmUhN == true){gyjyOWmUhN = false;}
      if(rrfQWHPZcu == true){rrfQWHPZcu = false;}
      if(uXzmgGHfmD == true){uXzmgGHfmD = false;}
      if(pWadjXXQTQ == true){pWadjXXQTQ = false;}
      if(nJhrVpoxdp == true){nJhrVpoxdp = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KWXMCXXNJT
{ 
  void XXbKrHTuoc()
  { 
      bool AZKSJMLYEb = false;
      bool ZkQIxTJnpW = false;
      bool iGhqLesmDk = false;
      bool xOriRxdrQY = false;
      bool qQCtQaFWxb = false;
      bool XmbHnsrSGe = false;
      bool SXVTeKsgqz = false;
      bool pRXReREmZj = false;
      bool YWpaMJmjuX = false;
      bool FKkkFSuhOG = false;
      bool hXfxkFOJbV = false;
      bool oErWGwhtGU = false;
      bool XIlStefmLL = false;
      bool WVYTCBproG = false;
      bool ucHYiAROeJ = false;
      bool kAbthYlMDF = false;
      bool ZbxPzSfYTE = false;
      bool FoFzZtANjs = false;
      bool NRHCaVtdkc = false;
      bool OdoigSerYP = false;
      string AZcKiBtKth;
      string rEnBCiERSx;
      string RVLLqnjCXt;
      string kkhkORYueZ;
      string IdcoWGkNpi;
      string zKtnKeNQSu;
      string RqlzwGWClj;
      string KJlsnmrWnj;
      string CVLouaqtai;
      string EHbjHRqcrQ;
      string jiaFYpXAzW;
      string mqcirwexEV;
      string PbaDeDgClW;
      string wOdXzTtFPE;
      string DptEwNesqL;
      string lsYWBHHXZV;
      string IUObZYNEiX;
      string BYhzSYhWGR;
      string XnLZkDoGWq;
      string MWftwhbSdW;
      if(AZcKiBtKth == jiaFYpXAzW){AZKSJMLYEb = true;}
      else if(jiaFYpXAzW == AZcKiBtKth){hXfxkFOJbV = true;}
      if(rEnBCiERSx == mqcirwexEV){ZkQIxTJnpW = true;}
      else if(mqcirwexEV == rEnBCiERSx){oErWGwhtGU = true;}
      if(RVLLqnjCXt == PbaDeDgClW){iGhqLesmDk = true;}
      else if(PbaDeDgClW == RVLLqnjCXt){XIlStefmLL = true;}
      if(kkhkORYueZ == wOdXzTtFPE){xOriRxdrQY = true;}
      else if(wOdXzTtFPE == kkhkORYueZ){WVYTCBproG = true;}
      if(IdcoWGkNpi == DptEwNesqL){qQCtQaFWxb = true;}
      else if(DptEwNesqL == IdcoWGkNpi){ucHYiAROeJ = true;}
      if(zKtnKeNQSu == lsYWBHHXZV){XmbHnsrSGe = true;}
      else if(lsYWBHHXZV == zKtnKeNQSu){kAbthYlMDF = true;}
      if(RqlzwGWClj == IUObZYNEiX){SXVTeKsgqz = true;}
      else if(IUObZYNEiX == RqlzwGWClj){ZbxPzSfYTE = true;}
      if(KJlsnmrWnj == BYhzSYhWGR){pRXReREmZj = true;}
      if(CVLouaqtai == XnLZkDoGWq){YWpaMJmjuX = true;}
      if(EHbjHRqcrQ == MWftwhbSdW){FKkkFSuhOG = true;}
      while(BYhzSYhWGR == KJlsnmrWnj){FoFzZtANjs = true;}
      while(XnLZkDoGWq == XnLZkDoGWq){NRHCaVtdkc = true;}
      while(MWftwhbSdW == MWftwhbSdW){OdoigSerYP = true;}
      if(AZKSJMLYEb == true){AZKSJMLYEb = false;}
      if(ZkQIxTJnpW == true){ZkQIxTJnpW = false;}
      if(iGhqLesmDk == true){iGhqLesmDk = false;}
      if(xOriRxdrQY == true){xOriRxdrQY = false;}
      if(qQCtQaFWxb == true){qQCtQaFWxb = false;}
      if(XmbHnsrSGe == true){XmbHnsrSGe = false;}
      if(SXVTeKsgqz == true){SXVTeKsgqz = false;}
      if(pRXReREmZj == true){pRXReREmZj = false;}
      if(YWpaMJmjuX == true){YWpaMJmjuX = false;}
      if(FKkkFSuhOG == true){FKkkFSuhOG = false;}
      if(hXfxkFOJbV == true){hXfxkFOJbV = false;}
      if(oErWGwhtGU == true){oErWGwhtGU = false;}
      if(XIlStefmLL == true){XIlStefmLL = false;}
      if(WVYTCBproG == true){WVYTCBproG = false;}
      if(ucHYiAROeJ == true){ucHYiAROeJ = false;}
      if(kAbthYlMDF == true){kAbthYlMDF = false;}
      if(ZbxPzSfYTE == true){ZbxPzSfYTE = false;}
      if(FoFzZtANjs == true){FoFzZtANjs = false;}
      if(NRHCaVtdkc == true){NRHCaVtdkc = false;}
      if(OdoigSerYP == true){OdoigSerYP = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YUCRUKMMGG
{ 
  void InNeCtKdku()
  { 
      bool TDYhKplEbx = false;
      bool lpQBOTFRJH = false;
      bool HGZtQrapix = false;
      bool NRTNcgTegE = false;
      bool sKKOxLGFjS = false;
      bool pHVcYNdeQR = false;
      bool PwVeiTVCXg = false;
      bool zZkFOwuLwF = false;
      bool kVGPGGwqoP = false;
      bool DoNOowoeed = false;
      bool oDgCSPVPOb = false;
      bool RSiLftpGXb = false;
      bool XeNIdKLWQR = false;
      bool HQfRPhnDwq = false;
      bool kfbMyjFChu = false;
      bool GiHDFKoXYH = false;
      bool DzzwTAlQwy = false;
      bool FlJgqrCBKs = false;
      bool FJnVKYJbME = false;
      bool rPenmkffrx = false;
      string fuaVSHsCFx;
      string NwRsZBeYHm;
      string mZKqojSssU;
      string yfHZaAFgxO;
      string JVianFPbgV;
      string IGUAJGkigi;
      string ozJjzhnrSY;
      string wxarcRpjCj;
      string TqMXMrLHPT;
      string OVAOZKLzbG;
      string QXIHZKebWr;
      string PBgxOyllXG;
      string VgbyJZxMSS;
      string FqHuSwhExB;
      string NijXENoDza;
      string XZaUjiuKyz;
      string NRJEmXskdR;
      string QhnVrmxEtw;
      string ksMqtePmLj;
      string BXGymuYfkf;
      if(fuaVSHsCFx == QXIHZKebWr){TDYhKplEbx = true;}
      else if(QXIHZKebWr == fuaVSHsCFx){oDgCSPVPOb = true;}
      if(NwRsZBeYHm == PBgxOyllXG){lpQBOTFRJH = true;}
      else if(PBgxOyllXG == NwRsZBeYHm){RSiLftpGXb = true;}
      if(mZKqojSssU == VgbyJZxMSS){HGZtQrapix = true;}
      else if(VgbyJZxMSS == mZKqojSssU){XeNIdKLWQR = true;}
      if(yfHZaAFgxO == FqHuSwhExB){NRTNcgTegE = true;}
      else if(FqHuSwhExB == yfHZaAFgxO){HQfRPhnDwq = true;}
      if(JVianFPbgV == NijXENoDza){sKKOxLGFjS = true;}
      else if(NijXENoDza == JVianFPbgV){kfbMyjFChu = true;}
      if(IGUAJGkigi == XZaUjiuKyz){pHVcYNdeQR = true;}
      else if(XZaUjiuKyz == IGUAJGkigi){GiHDFKoXYH = true;}
      if(ozJjzhnrSY == NRJEmXskdR){PwVeiTVCXg = true;}
      else if(NRJEmXskdR == ozJjzhnrSY){DzzwTAlQwy = true;}
      if(wxarcRpjCj == QhnVrmxEtw){zZkFOwuLwF = true;}
      if(TqMXMrLHPT == ksMqtePmLj){kVGPGGwqoP = true;}
      if(OVAOZKLzbG == BXGymuYfkf){DoNOowoeed = true;}
      while(QhnVrmxEtw == wxarcRpjCj){FlJgqrCBKs = true;}
      while(ksMqtePmLj == ksMqtePmLj){FJnVKYJbME = true;}
      while(BXGymuYfkf == BXGymuYfkf){rPenmkffrx = true;}
      if(TDYhKplEbx == true){TDYhKplEbx = false;}
      if(lpQBOTFRJH == true){lpQBOTFRJH = false;}
      if(HGZtQrapix == true){HGZtQrapix = false;}
      if(NRTNcgTegE == true){NRTNcgTegE = false;}
      if(sKKOxLGFjS == true){sKKOxLGFjS = false;}
      if(pHVcYNdeQR == true){pHVcYNdeQR = false;}
      if(PwVeiTVCXg == true){PwVeiTVCXg = false;}
      if(zZkFOwuLwF == true){zZkFOwuLwF = false;}
      if(kVGPGGwqoP == true){kVGPGGwqoP = false;}
      if(DoNOowoeed == true){DoNOowoeed = false;}
      if(oDgCSPVPOb == true){oDgCSPVPOb = false;}
      if(RSiLftpGXb == true){RSiLftpGXb = false;}
      if(XeNIdKLWQR == true){XeNIdKLWQR = false;}
      if(HQfRPhnDwq == true){HQfRPhnDwq = false;}
      if(kfbMyjFChu == true){kfbMyjFChu = false;}
      if(GiHDFKoXYH == true){GiHDFKoXYH = false;}
      if(DzzwTAlQwy == true){DzzwTAlQwy = false;}
      if(FlJgqrCBKs == true){FlJgqrCBKs = false;}
      if(FJnVKYJbME == true){FJnVKYJbME = false;}
      if(rPenmkffrx == true){rPenmkffrx = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class VGQGQRDSUJ
{ 
  void qWAylTifAp()
  { 
      bool iqaarCVwrX = false;
      bool rmQepHOXoc = false;
      bool LDBaEiElKa = false;
      bool IJiYihPkjc = false;
      bool kTHxDBjPds = false;
      bool gggxlfpAVJ = false;
      bool xilRPhErwH = false;
      bool qdnLqqwEXC = false;
      bool znHQsbcklC = false;
      bool rPOafBhpgU = false;
      bool xrTWCDrUGu = false;
      bool jlcTJJbUXn = false;
      bool ESFsObPMfp = false;
      bool IFWCFGUkOp = false;
      bool iHepSGmuQV = false;
      bool ZMRsPelaHZ = false;
      bool neoPldPZND = false;
      bool nqGgExmOZI = false;
      bool nTwJPwncla = false;
      bool UtcnCDROEn = false;
      string OnwhOkByBl;
      string OsAABNSYOR;
      string BQAEbZkKTB;
      string QebBarirpE;
      string myVdpMSKMg;
      string NnRgWynrFn;
      string AqeAViRzZI;
      string dahJdDuWOP;
      string GZxmmbNtdc;
      string DqmCDwTVHG;
      string uTkDCmGHDT;
      string CRdpgZsqMs;
      string UWUxFmnfGh;
      string iZOHTmosjA;
      string VGqKZlNxUc;
      string CbIOlXynyP;
      string yZBUjbCVVt;
      string kzGLWNfKWS;
      string ZqLcfGQzUu;
      string oBuUBwFVax;
      if(OnwhOkByBl == uTkDCmGHDT){iqaarCVwrX = true;}
      else if(uTkDCmGHDT == OnwhOkByBl){xrTWCDrUGu = true;}
      if(OsAABNSYOR == CRdpgZsqMs){rmQepHOXoc = true;}
      else if(CRdpgZsqMs == OsAABNSYOR){jlcTJJbUXn = true;}
      if(BQAEbZkKTB == UWUxFmnfGh){LDBaEiElKa = true;}
      else if(UWUxFmnfGh == BQAEbZkKTB){ESFsObPMfp = true;}
      if(QebBarirpE == iZOHTmosjA){IJiYihPkjc = true;}
      else if(iZOHTmosjA == QebBarirpE){IFWCFGUkOp = true;}
      if(myVdpMSKMg == VGqKZlNxUc){kTHxDBjPds = true;}
      else if(VGqKZlNxUc == myVdpMSKMg){iHepSGmuQV = true;}
      if(NnRgWynrFn == CbIOlXynyP){gggxlfpAVJ = true;}
      else if(CbIOlXynyP == NnRgWynrFn){ZMRsPelaHZ = true;}
      if(AqeAViRzZI == yZBUjbCVVt){xilRPhErwH = true;}
      else if(yZBUjbCVVt == AqeAViRzZI){neoPldPZND = true;}
      if(dahJdDuWOP == kzGLWNfKWS){qdnLqqwEXC = true;}
      if(GZxmmbNtdc == ZqLcfGQzUu){znHQsbcklC = true;}
      if(DqmCDwTVHG == oBuUBwFVax){rPOafBhpgU = true;}
      while(kzGLWNfKWS == dahJdDuWOP){nqGgExmOZI = true;}
      while(ZqLcfGQzUu == ZqLcfGQzUu){nTwJPwncla = true;}
      while(oBuUBwFVax == oBuUBwFVax){UtcnCDROEn = true;}
      if(iqaarCVwrX == true){iqaarCVwrX = false;}
      if(rmQepHOXoc == true){rmQepHOXoc = false;}
      if(LDBaEiElKa == true){LDBaEiElKa = false;}
      if(IJiYihPkjc == true){IJiYihPkjc = false;}
      if(kTHxDBjPds == true){kTHxDBjPds = false;}
      if(gggxlfpAVJ == true){gggxlfpAVJ = false;}
      if(xilRPhErwH == true){xilRPhErwH = false;}
      if(qdnLqqwEXC == true){qdnLqqwEXC = false;}
      if(znHQsbcklC == true){znHQsbcklC = false;}
      if(rPOafBhpgU == true){rPOafBhpgU = false;}
      if(xrTWCDrUGu == true){xrTWCDrUGu = false;}
      if(jlcTJJbUXn == true){jlcTJJbUXn = false;}
      if(ESFsObPMfp == true){ESFsObPMfp = false;}
      if(IFWCFGUkOp == true){IFWCFGUkOp = false;}
      if(iHepSGmuQV == true){iHepSGmuQV = false;}
      if(ZMRsPelaHZ == true){ZMRsPelaHZ = false;}
      if(neoPldPZND == true){neoPldPZND = false;}
      if(nqGgExmOZI == true){nqGgExmOZI = false;}
      if(nTwJPwncla == true){nTwJPwncla = false;}
      if(UtcnCDROEn == true){UtcnCDROEn = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FSUQUVURVB
{ 
  void uKfNxLsIrg()
  { 
      bool mKLtMkYYUY = false;
      bool hjiilbMpHJ = false;
      bool awUWmwbYYe = false;
      bool XJSDFlfZwy = false;
      bool MHQdQYkkYw = false;
      bool KlhtppooGP = false;
      bool IAAoAYRfcA = false;
      bool wmtIwTepFN = false;
      bool LqAcUkbXNO = false;
      bool wpSfoMHbcr = false;
      bool HEHXAXrYxV = false;
      bool LzirDRzWUP = false;
      bool wsSOEFTNTi = false;
      bool MDMNFHibbw = false;
      bool sDZqCxHacX = false;
      bool NjHNuOlrTu = false;
      bool EdEOJIlVlw = false;
      bool iWtXVsSaLG = false;
      bool yrkwVbwzRq = false;
      bool VpIgrOyLht = false;
      string mEGdEIWRVF;
      string yMltRWreho;
      string KJXafsyweq;
      string CGpUOrLhPc;
      string zaqiukwbNX;
      string EGqzbdfexK;
      string KZldDrWQeZ;
      string KKPFpjRoeu;
      string wViHmuOpsk;
      string MgjESYnNeD;
      string NfxnMtqlJr;
      string LqzCNoqepH;
      string dfXsMdZKlR;
      string uDJbgQXutr;
      string EcgAYUghqd;
      string hJWXZhHDai;
      string TfzqwHhWVw;
      string QwjxDAzWUr;
      string wrTIjlgkCz;
      string JTTLYXcoEQ;
      if(mEGdEIWRVF == NfxnMtqlJr){mKLtMkYYUY = true;}
      else if(NfxnMtqlJr == mEGdEIWRVF){HEHXAXrYxV = true;}
      if(yMltRWreho == LqzCNoqepH){hjiilbMpHJ = true;}
      else if(LqzCNoqepH == yMltRWreho){LzirDRzWUP = true;}
      if(KJXafsyweq == dfXsMdZKlR){awUWmwbYYe = true;}
      else if(dfXsMdZKlR == KJXafsyweq){wsSOEFTNTi = true;}
      if(CGpUOrLhPc == uDJbgQXutr){XJSDFlfZwy = true;}
      else if(uDJbgQXutr == CGpUOrLhPc){MDMNFHibbw = true;}
      if(zaqiukwbNX == EcgAYUghqd){MHQdQYkkYw = true;}
      else if(EcgAYUghqd == zaqiukwbNX){sDZqCxHacX = true;}
      if(EGqzbdfexK == hJWXZhHDai){KlhtppooGP = true;}
      else if(hJWXZhHDai == EGqzbdfexK){NjHNuOlrTu = true;}
      if(KZldDrWQeZ == TfzqwHhWVw){IAAoAYRfcA = true;}
      else if(TfzqwHhWVw == KZldDrWQeZ){EdEOJIlVlw = true;}
      if(KKPFpjRoeu == QwjxDAzWUr){wmtIwTepFN = true;}
      if(wViHmuOpsk == wrTIjlgkCz){LqAcUkbXNO = true;}
      if(MgjESYnNeD == JTTLYXcoEQ){wpSfoMHbcr = true;}
      while(QwjxDAzWUr == KKPFpjRoeu){iWtXVsSaLG = true;}
      while(wrTIjlgkCz == wrTIjlgkCz){yrkwVbwzRq = true;}
      while(JTTLYXcoEQ == JTTLYXcoEQ){VpIgrOyLht = true;}
      if(mKLtMkYYUY == true){mKLtMkYYUY = false;}
      if(hjiilbMpHJ == true){hjiilbMpHJ = false;}
      if(awUWmwbYYe == true){awUWmwbYYe = false;}
      if(XJSDFlfZwy == true){XJSDFlfZwy = false;}
      if(MHQdQYkkYw == true){MHQdQYkkYw = false;}
      if(KlhtppooGP == true){KlhtppooGP = false;}
      if(IAAoAYRfcA == true){IAAoAYRfcA = false;}
      if(wmtIwTepFN == true){wmtIwTepFN = false;}
      if(LqAcUkbXNO == true){LqAcUkbXNO = false;}
      if(wpSfoMHbcr == true){wpSfoMHbcr = false;}
      if(HEHXAXrYxV == true){HEHXAXrYxV = false;}
      if(LzirDRzWUP == true){LzirDRzWUP = false;}
      if(wsSOEFTNTi == true){wsSOEFTNTi = false;}
      if(MDMNFHibbw == true){MDMNFHibbw = false;}
      if(sDZqCxHacX == true){sDZqCxHacX = false;}
      if(NjHNuOlrTu == true){NjHNuOlrTu = false;}
      if(EdEOJIlVlw == true){EdEOJIlVlw = false;}
      if(iWtXVsSaLG == true){iWtXVsSaLG = false;}
      if(yrkwVbwzRq == true){yrkwVbwzRq = false;}
      if(VpIgrOyLht == true){VpIgrOyLht = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class NZDFTBMVEW
{ 
  void PAsCaNyRyM()
  { 
      bool byAwqblsnd = false;
      bool aLRrKoZwVw = false;
      bool xRDhFfwGEN = false;
      bool MpUqHSJrlC = false;
      bool VSfhWrdZkG = false;
      bool hFPJBJuSqK = false;
      bool eenaQkqajr = false;
      bool fgKTZelUKN = false;
      bool CWVyeGZKoy = false;
      bool XpBbWqeFRk = false;
      bool ADosZBuwiW = false;
      bool XZIdHIIouJ = false;
      bool KfkkTpfhwi = false;
      bool zfDYMzgcXd = false;
      bool LdbSaxhXTO = false;
      bool hoJHaTREYj = false;
      bool VMzzWqWiQI = false;
      bool afRRygERKW = false;
      bool IIOhBKWuyr = false;
      bool gcEnIydbwi = false;
      string hSeyqcFCMX;
      string GRNGPFJVis;
      string eWQTCcmbxd;
      string uPwTiisHTn;
      string aTZgblPcHS;
      string rMQnKLBLuN;
      string ocEibpVsWf;
      string LCCtSKwjqQ;
      string NKUdliyHJX;
      string ghLGyyOqEk;
      string PWGaGpKVVz;
      string ORTeAeHZxs;
      string rNjyzdpuwU;
      string wFeFApDsXu;
      string XblJIONfGS;
      string pntKsClHbb;
      string zPZADJmOeJ;
      string bNCBVPbZpj;
      string QJoceHLULx;
      string EuRuwICkFz;
      if(hSeyqcFCMX == PWGaGpKVVz){byAwqblsnd = true;}
      else if(PWGaGpKVVz == hSeyqcFCMX){ADosZBuwiW = true;}
      if(GRNGPFJVis == ORTeAeHZxs){aLRrKoZwVw = true;}
      else if(ORTeAeHZxs == GRNGPFJVis){XZIdHIIouJ = true;}
      if(eWQTCcmbxd == rNjyzdpuwU){xRDhFfwGEN = true;}
      else if(rNjyzdpuwU == eWQTCcmbxd){KfkkTpfhwi = true;}
      if(uPwTiisHTn == wFeFApDsXu){MpUqHSJrlC = true;}
      else if(wFeFApDsXu == uPwTiisHTn){zfDYMzgcXd = true;}
      if(aTZgblPcHS == XblJIONfGS){VSfhWrdZkG = true;}
      else if(XblJIONfGS == aTZgblPcHS){LdbSaxhXTO = true;}
      if(rMQnKLBLuN == pntKsClHbb){hFPJBJuSqK = true;}
      else if(pntKsClHbb == rMQnKLBLuN){hoJHaTREYj = true;}
      if(ocEibpVsWf == zPZADJmOeJ){eenaQkqajr = true;}
      else if(zPZADJmOeJ == ocEibpVsWf){VMzzWqWiQI = true;}
      if(LCCtSKwjqQ == bNCBVPbZpj){fgKTZelUKN = true;}
      if(NKUdliyHJX == QJoceHLULx){CWVyeGZKoy = true;}
      if(ghLGyyOqEk == EuRuwICkFz){XpBbWqeFRk = true;}
      while(bNCBVPbZpj == LCCtSKwjqQ){afRRygERKW = true;}
      while(QJoceHLULx == QJoceHLULx){IIOhBKWuyr = true;}
      while(EuRuwICkFz == EuRuwICkFz){gcEnIydbwi = true;}
      if(byAwqblsnd == true){byAwqblsnd = false;}
      if(aLRrKoZwVw == true){aLRrKoZwVw = false;}
      if(xRDhFfwGEN == true){xRDhFfwGEN = false;}
      if(MpUqHSJrlC == true){MpUqHSJrlC = false;}
      if(VSfhWrdZkG == true){VSfhWrdZkG = false;}
      if(hFPJBJuSqK == true){hFPJBJuSqK = false;}
      if(eenaQkqajr == true){eenaQkqajr = false;}
      if(fgKTZelUKN == true){fgKTZelUKN = false;}
      if(CWVyeGZKoy == true){CWVyeGZKoy = false;}
      if(XpBbWqeFRk == true){XpBbWqeFRk = false;}
      if(ADosZBuwiW == true){ADosZBuwiW = false;}
      if(XZIdHIIouJ == true){XZIdHIIouJ = false;}
      if(KfkkTpfhwi == true){KfkkTpfhwi = false;}
      if(zfDYMzgcXd == true){zfDYMzgcXd = false;}
      if(LdbSaxhXTO == true){LdbSaxhXTO = false;}
      if(hoJHaTREYj == true){hoJHaTREYj = false;}
      if(VMzzWqWiQI == true){VMzzWqWiQI = false;}
      if(afRRygERKW == true){afRRygERKW = false;}
      if(IIOhBKWuyr == true){IIOhBKWuyr = false;}
      if(gcEnIydbwi == true){gcEnIydbwi = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BSAMWVATJI
{ 
  void eDpHEGNhUu()
  { 
      bool VExlbPylcm = false;
      bool xwhxTpJSNB = false;
      bool fczFqZSfjq = false;
      bool XqKYyobBfR = false;
      bool ejHIKfWwBl = false;
      bool ymlkosFquB = false;
      bool wSBlPGkJzd = false;
      bool OImwVwUpuK = false;
      bool DWdxsjFYiE = false;
      bool jkFdUMhVsE = false;
      bool tMxXXpApPr = false;
      bool XOujyZnDse = false;
      bool AMZtsdCIaJ = false;
      bool NzxqPpYFZC = false;
      bool jOJpgVoUft = false;
      bool XdcNDhmDes = false;
      bool mqOFeXRETH = false;
      bool AhdpulCiMX = false;
      bool ogcgkiVimh = false;
      bool DVafOszDLI = false;
      string gmsbqcXizT;
      string VzmKtPSyrO;
      string bgUtnZmTGA;
      string WqNywdKKLJ;
      string ENFcFNxJlO;
      string HcWOXzSzmR;
      string pEbzxdkoOP;
      string FHmhJObroa;
      string ccgrrtUzaw;
      string LKXtTAkGea;
      string bHuVIzonEI;
      string XaKMnMXjGD;
      string JlOSEujsOJ;
      string dVMNyUhKis;
      string EftRXjALFG;
      string cAzbQoscRN;
      string PwZteCNwOS;
      string eMBNydqxtT;
      string tYmHgdVkwI;
      string bEsUcHFAlV;
      if(gmsbqcXizT == bHuVIzonEI){VExlbPylcm = true;}
      else if(bHuVIzonEI == gmsbqcXizT){tMxXXpApPr = true;}
      if(VzmKtPSyrO == XaKMnMXjGD){xwhxTpJSNB = true;}
      else if(XaKMnMXjGD == VzmKtPSyrO){XOujyZnDse = true;}
      if(bgUtnZmTGA == JlOSEujsOJ){fczFqZSfjq = true;}
      else if(JlOSEujsOJ == bgUtnZmTGA){AMZtsdCIaJ = true;}
      if(WqNywdKKLJ == dVMNyUhKis){XqKYyobBfR = true;}
      else if(dVMNyUhKis == WqNywdKKLJ){NzxqPpYFZC = true;}
      if(ENFcFNxJlO == EftRXjALFG){ejHIKfWwBl = true;}
      else if(EftRXjALFG == ENFcFNxJlO){jOJpgVoUft = true;}
      if(HcWOXzSzmR == cAzbQoscRN){ymlkosFquB = true;}
      else if(cAzbQoscRN == HcWOXzSzmR){XdcNDhmDes = true;}
      if(pEbzxdkoOP == PwZteCNwOS){wSBlPGkJzd = true;}
      else if(PwZteCNwOS == pEbzxdkoOP){mqOFeXRETH = true;}
      if(FHmhJObroa == eMBNydqxtT){OImwVwUpuK = true;}
      if(ccgrrtUzaw == tYmHgdVkwI){DWdxsjFYiE = true;}
      if(LKXtTAkGea == bEsUcHFAlV){jkFdUMhVsE = true;}
      while(eMBNydqxtT == FHmhJObroa){AhdpulCiMX = true;}
      while(tYmHgdVkwI == tYmHgdVkwI){ogcgkiVimh = true;}
      while(bEsUcHFAlV == bEsUcHFAlV){DVafOszDLI = true;}
      if(VExlbPylcm == true){VExlbPylcm = false;}
      if(xwhxTpJSNB == true){xwhxTpJSNB = false;}
      if(fczFqZSfjq == true){fczFqZSfjq = false;}
      if(XqKYyobBfR == true){XqKYyobBfR = false;}
      if(ejHIKfWwBl == true){ejHIKfWwBl = false;}
      if(ymlkosFquB == true){ymlkosFquB = false;}
      if(wSBlPGkJzd == true){wSBlPGkJzd = false;}
      if(OImwVwUpuK == true){OImwVwUpuK = false;}
      if(DWdxsjFYiE == true){DWdxsjFYiE = false;}
      if(jkFdUMhVsE == true){jkFdUMhVsE = false;}
      if(tMxXXpApPr == true){tMxXXpApPr = false;}
      if(XOujyZnDse == true){XOujyZnDse = false;}
      if(AMZtsdCIaJ == true){AMZtsdCIaJ = false;}
      if(NzxqPpYFZC == true){NzxqPpYFZC = false;}
      if(jOJpgVoUft == true){jOJpgVoUft = false;}
      if(XdcNDhmDes == true){XdcNDhmDes = false;}
      if(mqOFeXRETH == true){mqOFeXRETH = false;}
      if(AhdpulCiMX == true){AhdpulCiMX = false;}
      if(ogcgkiVimh == true){ogcgkiVimh = false;}
      if(DVafOszDLI == true){DVafOszDLI = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class KFUJAZATJV
{ 
  void WGmRiGULJF()
  { 
      bool VrEesQYekb = false;
      bool xFJAKyAlig = false;
      bool ewtIITFkDz = false;
      bool JWHqUqYPsn = false;
      bool PzFTTwSaTa = false;
      bool lRqEdYyuVZ = false;
      bool KnakRZySfT = false;
      bool YjrtRfqaoM = false;
      bool AeQcNqfCNu = false;
      bool rdtTcaXUBt = false;
      bool yAuzBBYwlx = false;
      bool hXGJZfWyWZ = false;
      bool suFjaupYQp = false;
      bool IVYtOzjQaM = false;
      bool SjPPMUXksx = false;
      bool EYxaACuKXG = false;
      bool LAfuoLyLuZ = false;
      bool wFXKHJeoXG = false;
      bool WEeTljYysL = false;
      bool DZrUASEUaA = false;
      string pUXrXURGLI;
      string QQdGRgqEfH;
      string TYTboJUIcu;
      string epyTcJFSQa;
      string GJCSIXJFzo;
      string ISIrLQBuDN;
      string AIcQdZRJdH;
      string bXnFAKWBTR;
      string APJmlLhGIL;
      string IJPCaxflnh;
      string FzbqNjeDrA;
      string tzakMjzpdR;
      string rLARzSIkdD;
      string ekMkLBmSZB;
      string fdOHRIhdpp;
      string xoHWptyJqu;
      string btZnVImoZL;
      string xgPckNxsAp;
      string jdtmFodpuQ;
      string CcaeYlEZkI;
      if(pUXrXURGLI == FzbqNjeDrA){VrEesQYekb = true;}
      else if(FzbqNjeDrA == pUXrXURGLI){yAuzBBYwlx = true;}
      if(QQdGRgqEfH == tzakMjzpdR){xFJAKyAlig = true;}
      else if(tzakMjzpdR == QQdGRgqEfH){hXGJZfWyWZ = true;}
      if(TYTboJUIcu == rLARzSIkdD){ewtIITFkDz = true;}
      else if(rLARzSIkdD == TYTboJUIcu){suFjaupYQp = true;}
      if(epyTcJFSQa == ekMkLBmSZB){JWHqUqYPsn = true;}
      else if(ekMkLBmSZB == epyTcJFSQa){IVYtOzjQaM = true;}
      if(GJCSIXJFzo == fdOHRIhdpp){PzFTTwSaTa = true;}
      else if(fdOHRIhdpp == GJCSIXJFzo){SjPPMUXksx = true;}
      if(ISIrLQBuDN == xoHWptyJqu){lRqEdYyuVZ = true;}
      else if(xoHWptyJqu == ISIrLQBuDN){EYxaACuKXG = true;}
      if(AIcQdZRJdH == btZnVImoZL){KnakRZySfT = true;}
      else if(btZnVImoZL == AIcQdZRJdH){LAfuoLyLuZ = true;}
      if(bXnFAKWBTR == xgPckNxsAp){YjrtRfqaoM = true;}
      if(APJmlLhGIL == jdtmFodpuQ){AeQcNqfCNu = true;}
      if(IJPCaxflnh == CcaeYlEZkI){rdtTcaXUBt = true;}
      while(xgPckNxsAp == bXnFAKWBTR){wFXKHJeoXG = true;}
      while(jdtmFodpuQ == jdtmFodpuQ){WEeTljYysL = true;}
      while(CcaeYlEZkI == CcaeYlEZkI){DZrUASEUaA = true;}
      if(VrEesQYekb == true){VrEesQYekb = false;}
      if(xFJAKyAlig == true){xFJAKyAlig = false;}
      if(ewtIITFkDz == true){ewtIITFkDz = false;}
      if(JWHqUqYPsn == true){JWHqUqYPsn = false;}
      if(PzFTTwSaTa == true){PzFTTwSaTa = false;}
      if(lRqEdYyuVZ == true){lRqEdYyuVZ = false;}
      if(KnakRZySfT == true){KnakRZySfT = false;}
      if(YjrtRfqaoM == true){YjrtRfqaoM = false;}
      if(AeQcNqfCNu == true){AeQcNqfCNu = false;}
      if(rdtTcaXUBt == true){rdtTcaXUBt = false;}
      if(yAuzBBYwlx == true){yAuzBBYwlx = false;}
      if(hXGJZfWyWZ == true){hXGJZfWyWZ = false;}
      if(suFjaupYQp == true){suFjaupYQp = false;}
      if(IVYtOzjQaM == true){IVYtOzjQaM = false;}
      if(SjPPMUXksx == true){SjPPMUXksx = false;}
      if(EYxaACuKXG == true){EYxaACuKXG = false;}
      if(LAfuoLyLuZ == true){LAfuoLyLuZ = false;}
      if(wFXKHJeoXG == true){wFXKHJeoXG = false;}
      if(WEeTljYysL == true){WEeTljYysL = false;}
      if(DZrUASEUaA == true){DZrUASEUaA = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class YQUDMYIWCW
{ 
  void ZWNDHrNqEY()
  { 
      bool AUKjouNsfL = false;
      bool CZmFkGsetK = false;
      bool dmIjmjSueY = false;
      bool mCmuxMKluM = false;
      bool QzrUobRFsT = false;
      bool cjwLJNLZwF = false;
      bool bSNGYAmqIX = false;
      bool uGjgtetnuL = false;
      bool RNBKwCFaki = false;
      bool Ajookxkluo = false;
      bool naxUXVAORd = false;
      bool TXekmOQDJW = false;
      bool tLFYWLQtww = false;
      bool tIrFWqQglZ = false;
      bool CqJHPcFECV = false;
      bool ghbYubUXxM = false;
      bool EFRUjKfYOu = false;
      bool xjQiTCCyJt = false;
      bool FMqZPLfSKt = false;
      bool VriYVxYoWE = false;
      string WzRjJnuxjU;
      string xWzIPVpJSt;
      string WHjkFqeWPk;
      string uDMHnjdfQG;
      string rsHfqoylsE;
      string wBrklOOfdR;
      string JXhzZtKrUe;
      string qKrMCdAEsP;
      string rrdKotLaUZ;
      string VxBywUKIGx;
      string dGQNMkJfiI;
      string OJlNZQtWJf;
      string hjWtJsItPg;
      string SqywlkaPxD;
      string jfqMLLLmXe;
      string ZnzaOtjVDS;
      string cOPBpWKpbn;
      string oeZFuCrJiy;
      string hVVjHerXgq;
      string BELdaVPpWn;
      if(WzRjJnuxjU == dGQNMkJfiI){AUKjouNsfL = true;}
      else if(dGQNMkJfiI == WzRjJnuxjU){naxUXVAORd = true;}
      if(xWzIPVpJSt == OJlNZQtWJf){CZmFkGsetK = true;}
      else if(OJlNZQtWJf == xWzIPVpJSt){TXekmOQDJW = true;}
      if(WHjkFqeWPk == hjWtJsItPg){dmIjmjSueY = true;}
      else if(hjWtJsItPg == WHjkFqeWPk){tLFYWLQtww = true;}
      if(uDMHnjdfQG == SqywlkaPxD){mCmuxMKluM = true;}
      else if(SqywlkaPxD == uDMHnjdfQG){tIrFWqQglZ = true;}
      if(rsHfqoylsE == jfqMLLLmXe){QzrUobRFsT = true;}
      else if(jfqMLLLmXe == rsHfqoylsE){CqJHPcFECV = true;}
      if(wBrklOOfdR == ZnzaOtjVDS){cjwLJNLZwF = true;}
      else if(ZnzaOtjVDS == wBrklOOfdR){ghbYubUXxM = true;}
      if(JXhzZtKrUe == cOPBpWKpbn){bSNGYAmqIX = true;}
      else if(cOPBpWKpbn == JXhzZtKrUe){EFRUjKfYOu = true;}
      if(qKrMCdAEsP == oeZFuCrJiy){uGjgtetnuL = true;}
      if(rrdKotLaUZ == hVVjHerXgq){RNBKwCFaki = true;}
      if(VxBywUKIGx == BELdaVPpWn){Ajookxkluo = true;}
      while(oeZFuCrJiy == qKrMCdAEsP){xjQiTCCyJt = true;}
      while(hVVjHerXgq == hVVjHerXgq){FMqZPLfSKt = true;}
      while(BELdaVPpWn == BELdaVPpWn){VriYVxYoWE = true;}
      if(AUKjouNsfL == true){AUKjouNsfL = false;}
      if(CZmFkGsetK == true){CZmFkGsetK = false;}
      if(dmIjmjSueY == true){dmIjmjSueY = false;}
      if(mCmuxMKluM == true){mCmuxMKluM = false;}
      if(QzrUobRFsT == true){QzrUobRFsT = false;}
      if(cjwLJNLZwF == true){cjwLJNLZwF = false;}
      if(bSNGYAmqIX == true){bSNGYAmqIX = false;}
      if(uGjgtetnuL == true){uGjgtetnuL = false;}
      if(RNBKwCFaki == true){RNBKwCFaki = false;}
      if(Ajookxkluo == true){Ajookxkluo = false;}
      if(naxUXVAORd == true){naxUXVAORd = false;}
      if(TXekmOQDJW == true){TXekmOQDJW = false;}
      if(tLFYWLQtww == true){tLFYWLQtww = false;}
      if(tIrFWqQglZ == true){tIrFWqQglZ = false;}
      if(CqJHPcFECV == true){CqJHPcFECV = false;}
      if(ghbYubUXxM == true){ghbYubUXxM = false;}
      if(EFRUjKfYOu == true){EFRUjKfYOu = false;}
      if(xjQiTCCyJt == true){xjQiTCCyJt = false;}
      if(FMqZPLfSKt == true){FMqZPLfSKt = false;}
      if(VriYVxYoWE == true){VriYVxYoWE = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class JQMKSWCKMU
{ 
  void ROjLhcCzjY()
  { 
      bool qsykCxaGyg = false;
      bool zYfzqYGNaB = false;
      bool SCjYIheDGI = false;
      bool IdGquKUckJ = false;
      bool wXUlOQkdGX = false;
      bool uMqFFnIZbU = false;
      bool RXzoXUSoDf = false;
      bool DdcRFthyKq = false;
      bool wBxhNwmBkH = false;
      bool CuAPqXXeOs = false;
      bool yuZZTRiZAL = false;
      bool FJOjXQaQba = false;
      bool hQDwdIcMag = false;
      bool WDjQsBbWOu = false;
      bool MMBZCrfQra = false;
      bool xGCmKgrdwL = false;
      bool oWzToCZyqW = false;
      bool oDCgOUPctk = false;
      bool cLUAiPAxFL = false;
      bool IxaDGpNhkz = false;
      string lyHwYdWFwp;
      string oRySnPbLco;
      string eOfLrlUyVU;
      string TNaBccenzX;
      string BHbNClcYwf;
      string jeugwWRwEl;
      string JYjaWbqxpK;
      string jnLEnXmSJF;
      string JwBojoRsEF;
      string LiiNOcXApZ;
      string tfhlwwSpsY;
      string jymycMXtqg;
      string KxskzAEyoL;
      string AmbiKcNPsn;
      string VaAeUMwkjB;
      string FcIJPxmwlu;
      string qUBgYwVDmz;
      string zHBCKixNPl;
      string DSNmGXdSSN;
      string pRcxwBZSGY;
      if(lyHwYdWFwp == tfhlwwSpsY){qsykCxaGyg = true;}
      else if(tfhlwwSpsY == lyHwYdWFwp){yuZZTRiZAL = true;}
      if(oRySnPbLco == jymycMXtqg){zYfzqYGNaB = true;}
      else if(jymycMXtqg == oRySnPbLco){FJOjXQaQba = true;}
      if(eOfLrlUyVU == KxskzAEyoL){SCjYIheDGI = true;}
      else if(KxskzAEyoL == eOfLrlUyVU){hQDwdIcMag = true;}
      if(TNaBccenzX == AmbiKcNPsn){IdGquKUckJ = true;}
      else if(AmbiKcNPsn == TNaBccenzX){WDjQsBbWOu = true;}
      if(BHbNClcYwf == VaAeUMwkjB){wXUlOQkdGX = true;}
      else if(VaAeUMwkjB == BHbNClcYwf){MMBZCrfQra = true;}
      if(jeugwWRwEl == FcIJPxmwlu){uMqFFnIZbU = true;}
      else if(FcIJPxmwlu == jeugwWRwEl){xGCmKgrdwL = true;}
      if(JYjaWbqxpK == qUBgYwVDmz){RXzoXUSoDf = true;}
      else if(qUBgYwVDmz == JYjaWbqxpK){oWzToCZyqW = true;}
      if(jnLEnXmSJF == zHBCKixNPl){DdcRFthyKq = true;}
      if(JwBojoRsEF == DSNmGXdSSN){wBxhNwmBkH = true;}
      if(LiiNOcXApZ == pRcxwBZSGY){CuAPqXXeOs = true;}
      while(zHBCKixNPl == jnLEnXmSJF){oDCgOUPctk = true;}
      while(DSNmGXdSSN == DSNmGXdSSN){cLUAiPAxFL = true;}
      while(pRcxwBZSGY == pRcxwBZSGY){IxaDGpNhkz = true;}
      if(qsykCxaGyg == true){qsykCxaGyg = false;}
      if(zYfzqYGNaB == true){zYfzqYGNaB = false;}
      if(SCjYIheDGI == true){SCjYIheDGI = false;}
      if(IdGquKUckJ == true){IdGquKUckJ = false;}
      if(wXUlOQkdGX == true){wXUlOQkdGX = false;}
      if(uMqFFnIZbU == true){uMqFFnIZbU = false;}
      if(RXzoXUSoDf == true){RXzoXUSoDf = false;}
      if(DdcRFthyKq == true){DdcRFthyKq = false;}
      if(wBxhNwmBkH == true){wBxhNwmBkH = false;}
      if(CuAPqXXeOs == true){CuAPqXXeOs = false;}
      if(yuZZTRiZAL == true){yuZZTRiZAL = false;}
      if(FJOjXQaQba == true){FJOjXQaQba = false;}
      if(hQDwdIcMag == true){hQDwdIcMag = false;}
      if(WDjQsBbWOu == true){WDjQsBbWOu = false;}
      if(MMBZCrfQra == true){MMBZCrfQra = false;}
      if(xGCmKgrdwL == true){xGCmKgrdwL = false;}
      if(oWzToCZyqW == true){oWzToCZyqW = false;}
      if(oDCgOUPctk == true){oDCgOUPctk = false;}
      if(cLUAiPAxFL == true){cLUAiPAxFL = false;}
      if(IxaDGpNhkz == true){IxaDGpNhkz = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class REIBEZUQCA
{ 
  void MDiGpDTdal()
  { 
      bool UbkEErLSTO = false;
      bool XatGmWsXhM = false;
      bool SFWXJUkRyW = false;
      bool KiIfAOjyuh = false;
      bool FJAikkcDNL = false;
      bool xGeBWOFNti = false;
      bool HsTwDnRNGX = false;
      bool lubAHIEmlX = false;
      bool qgEmZDDHJk = false;
      bool JWELZMpDex = false;
      bool TPxoZCeetN = false;
      bool JRaVrangWz = false;
      bool jKDUNPpPZu = false;
      bool SBIoEbjONW = false;
      bool YYNwaruhTz = false;
      bool XJfeMJLbbK = false;
      bool LOEyuIJlQP = false;
      bool qYTzaLwHsU = false;
      bool TIuaWJUccu = false;
      bool QHmCLJUOAS = false;
      string JqSdblHVIj;
      string lECbQpJxzG;
      string wCfCHWtBZw;
      string WWtSOfhwMW;
      string ZXSwItyjZa;
      string qJyCyGKPVF;
      string qGwLNOJjNj;
      string HMjcgXijtN;
      string YKayEMXqTH;
      string USZHeSbHhs;
      string kNVpTdYVVa;
      string WaVIggfxHB;
      string pRPFOolcyN;
      string wbBAbKnkeZ;
      string mCVsjtxRZx;
      string mdzgCTyPae;
      string qwXIeuZUfV;
      string stSgNnwWVU;
      string qUCAZRbTxI;
      string MXJAhqdMOo;
      if(JqSdblHVIj == kNVpTdYVVa){UbkEErLSTO = true;}
      else if(kNVpTdYVVa == JqSdblHVIj){TPxoZCeetN = true;}
      if(lECbQpJxzG == WaVIggfxHB){XatGmWsXhM = true;}
      else if(WaVIggfxHB == lECbQpJxzG){JRaVrangWz = true;}
      if(wCfCHWtBZw == pRPFOolcyN){SFWXJUkRyW = true;}
      else if(pRPFOolcyN == wCfCHWtBZw){jKDUNPpPZu = true;}
      if(WWtSOfhwMW == wbBAbKnkeZ){KiIfAOjyuh = true;}
      else if(wbBAbKnkeZ == WWtSOfhwMW){SBIoEbjONW = true;}
      if(ZXSwItyjZa == mCVsjtxRZx){FJAikkcDNL = true;}
      else if(mCVsjtxRZx == ZXSwItyjZa){YYNwaruhTz = true;}
      if(qJyCyGKPVF == mdzgCTyPae){xGeBWOFNti = true;}
      else if(mdzgCTyPae == qJyCyGKPVF){XJfeMJLbbK = true;}
      if(qGwLNOJjNj == qwXIeuZUfV){HsTwDnRNGX = true;}
      else if(qwXIeuZUfV == qGwLNOJjNj){LOEyuIJlQP = true;}
      if(HMjcgXijtN == stSgNnwWVU){lubAHIEmlX = true;}
      if(YKayEMXqTH == qUCAZRbTxI){qgEmZDDHJk = true;}
      if(USZHeSbHhs == MXJAhqdMOo){JWELZMpDex = true;}
      while(stSgNnwWVU == HMjcgXijtN){qYTzaLwHsU = true;}
      while(qUCAZRbTxI == qUCAZRbTxI){TIuaWJUccu = true;}
      while(MXJAhqdMOo == MXJAhqdMOo){QHmCLJUOAS = true;}
      if(UbkEErLSTO == true){UbkEErLSTO = false;}
      if(XatGmWsXhM == true){XatGmWsXhM = false;}
      if(SFWXJUkRyW == true){SFWXJUkRyW = false;}
      if(KiIfAOjyuh == true){KiIfAOjyuh = false;}
      if(FJAikkcDNL == true){FJAikkcDNL = false;}
      if(xGeBWOFNti == true){xGeBWOFNti = false;}
      if(HsTwDnRNGX == true){HsTwDnRNGX = false;}
      if(lubAHIEmlX == true){lubAHIEmlX = false;}
      if(qgEmZDDHJk == true){qgEmZDDHJk = false;}
      if(JWELZMpDex == true){JWELZMpDex = false;}
      if(TPxoZCeetN == true){TPxoZCeetN = false;}
      if(JRaVrangWz == true){JRaVrangWz = false;}
      if(jKDUNPpPZu == true){jKDUNPpPZu = false;}
      if(SBIoEbjONW == true){SBIoEbjONW = false;}
      if(YYNwaruhTz == true){YYNwaruhTz = false;}
      if(XJfeMJLbbK == true){XJfeMJLbbK = false;}
      if(LOEyuIJlQP == true){LOEyuIJlQP = false;}
      if(qYTzaLwHsU == true){qYTzaLwHsU = false;}
      if(TIuaWJUccu == true){TIuaWJUccu = false;}
      if(QHmCLJUOAS == true){QHmCLJUOAS = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class DZNIBVUPCX
{ 
  void QkCmMpPMUM()
  { 
      bool ZkSPYAApoE = false;
      bool sDAMRQpThb = false;
      bool lxIgDKgLUV = false;
      bool TAjVNmZAMY = false;
      bool HBJbBFdbqa = false;
      bool GUjnAbckLN = false;
      bool SnUTjsTeye = false;
      bool qpGITknffy = false;
      bool fEUtOyQuUl = false;
      bool KnozFTgprI = false;
      bool lzdMjQNaUO = false;
      bool nwFNhOSaEk = false;
      bool bXJMTCAHCz = false;
      bool ExnWMgaIJy = false;
      bool kpaFxOhbQq = false;
      bool AFlGgNoQrg = false;
      bool PGXUKpHiBA = false;
      bool nKQFAlwEOF = false;
      bool TYpLzyLhpy = false;
      bool rJPbheRGtH = false;
      string nzLiXPCHph;
      string ubMqKIAtfR;
      string eACVlgRssX;
      string EtHzcYydwo;
      string oHlLOYQwuA;
      string Vzuekmwoxq;
      string yIOSdAUNOl;
      string JVypbWOqGM;
      string QUyrCFybNo;
      string bJgEqgrThN;
      string EDhJqzAECN;
      string YFhjacKWPX;
      string BrzYJzoszu;
      string duIsHNmnAC;
      string EtZzQVawcN;
      string VVDJLMDIzq;
      string KGFHjdwocN;
      string oVVNTHSzIm;
      string YsQkYIqzgu;
      string JeoJrxnRPm;
      if(nzLiXPCHph == EDhJqzAECN){ZkSPYAApoE = true;}
      else if(EDhJqzAECN == nzLiXPCHph){lzdMjQNaUO = true;}
      if(ubMqKIAtfR == YFhjacKWPX){sDAMRQpThb = true;}
      else if(YFhjacKWPX == ubMqKIAtfR){nwFNhOSaEk = true;}
      if(eACVlgRssX == BrzYJzoszu){lxIgDKgLUV = true;}
      else if(BrzYJzoszu == eACVlgRssX){bXJMTCAHCz = true;}
      if(EtHzcYydwo == duIsHNmnAC){TAjVNmZAMY = true;}
      else if(duIsHNmnAC == EtHzcYydwo){ExnWMgaIJy = true;}
      if(oHlLOYQwuA == EtZzQVawcN){HBJbBFdbqa = true;}
      else if(EtZzQVawcN == oHlLOYQwuA){kpaFxOhbQq = true;}
      if(Vzuekmwoxq == VVDJLMDIzq){GUjnAbckLN = true;}
      else if(VVDJLMDIzq == Vzuekmwoxq){AFlGgNoQrg = true;}
      if(yIOSdAUNOl == KGFHjdwocN){SnUTjsTeye = true;}
      else if(KGFHjdwocN == yIOSdAUNOl){PGXUKpHiBA = true;}
      if(JVypbWOqGM == oVVNTHSzIm){qpGITknffy = true;}
      if(QUyrCFybNo == YsQkYIqzgu){fEUtOyQuUl = true;}
      if(bJgEqgrThN == JeoJrxnRPm){KnozFTgprI = true;}
      while(oVVNTHSzIm == JVypbWOqGM){nKQFAlwEOF = true;}
      while(YsQkYIqzgu == YsQkYIqzgu){TYpLzyLhpy = true;}
      while(JeoJrxnRPm == JeoJrxnRPm){rJPbheRGtH = true;}
      if(ZkSPYAApoE == true){ZkSPYAApoE = false;}
      if(sDAMRQpThb == true){sDAMRQpThb = false;}
      if(lxIgDKgLUV == true){lxIgDKgLUV = false;}
      if(TAjVNmZAMY == true){TAjVNmZAMY = false;}
      if(HBJbBFdbqa == true){HBJbBFdbqa = false;}
      if(GUjnAbckLN == true){GUjnAbckLN = false;}
      if(SnUTjsTeye == true){SnUTjsTeye = false;}
      if(qpGITknffy == true){qpGITknffy = false;}
      if(fEUtOyQuUl == true){fEUtOyQuUl = false;}
      if(KnozFTgprI == true){KnozFTgprI = false;}
      if(lzdMjQNaUO == true){lzdMjQNaUO = false;}
      if(nwFNhOSaEk == true){nwFNhOSaEk = false;}
      if(bXJMTCAHCz == true){bXJMTCAHCz = false;}
      if(ExnWMgaIJy == true){ExnWMgaIJy = false;}
      if(kpaFxOhbQq == true){kpaFxOhbQq = false;}
      if(AFlGgNoQrg == true){AFlGgNoQrg = false;}
      if(PGXUKpHiBA == true){PGXUKpHiBA = false;}
      if(nKQFAlwEOF == true){nKQFAlwEOF = false;}
      if(TYpLzyLhpy == true){TYpLzyLhpy = false;}
      if(rJPbheRGtH == true){rJPbheRGtH = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class FJBQQERILY
{ 
  void PnGgwWQOdK()
  { 
      bool iIyXUuBYYF = false;
      bool wBBTtojWbx = false;
      bool XUHYVWWqXb = false;
      bool SnWAJszNnQ = false;
      bool ybNxKmgJwz = false;
      bool OAeZxOAYBy = false;
      bool YKAwmHeAKA = false;
      bool oVhKywGJOi = false;
      bool oMuXMRJxDu = false;
      bool raMprnaRKC = false;
      bool IgzuJsRTaB = false;
      bool IpJRILOcjU = false;
      bool ygwPHOIqSK = false;
      bool UnyLZINzNS = false;
      bool AoVBmCQthf = false;
      bool YygpEajimu = false;
      bool jaBPgTqxqA = false;
      bool yPOQTIKubF = false;
      bool yNJmlVHIHz = false;
      bool XdMShYmGUT = false;
      string nQSNCIcjRK;
      string GaXxeOFTGP;
      string GQnbBNEzOn;
      string CxpMbMRUTn;
      string RWEEsTTJNM;
      string ypgwOWcpLo;
      string EcUdIIkLVR;
      string wjzRxiMihg;
      string stlCoLQtyE;
      string jaIeinBZwk;
      string iInuQNpOye;
      string KsHewKqSnw;
      string qqMQhUbUiJ;
      string hllxyjFpka;
      string UCuTtpSdiY;
      string whkztquXMu;
      string lVfnwXHKfL;
      string EAgMaousWr;
      string BxlJMzAmDG;
      string ohdpozWGWV;
      if(nQSNCIcjRK == iInuQNpOye){iIyXUuBYYF = true;}
      else if(iInuQNpOye == nQSNCIcjRK){IgzuJsRTaB = true;}
      if(GaXxeOFTGP == KsHewKqSnw){wBBTtojWbx = true;}
      else if(KsHewKqSnw == GaXxeOFTGP){IpJRILOcjU = true;}
      if(GQnbBNEzOn == qqMQhUbUiJ){XUHYVWWqXb = true;}
      else if(qqMQhUbUiJ == GQnbBNEzOn){ygwPHOIqSK = true;}
      if(CxpMbMRUTn == hllxyjFpka){SnWAJszNnQ = true;}
      else if(hllxyjFpka == CxpMbMRUTn){UnyLZINzNS = true;}
      if(RWEEsTTJNM == UCuTtpSdiY){ybNxKmgJwz = true;}
      else if(UCuTtpSdiY == RWEEsTTJNM){AoVBmCQthf = true;}
      if(ypgwOWcpLo == whkztquXMu){OAeZxOAYBy = true;}
      else if(whkztquXMu == ypgwOWcpLo){YygpEajimu = true;}
      if(EcUdIIkLVR == lVfnwXHKfL){YKAwmHeAKA = true;}
      else if(lVfnwXHKfL == EcUdIIkLVR){jaBPgTqxqA = true;}
      if(wjzRxiMihg == EAgMaousWr){oVhKywGJOi = true;}
      if(stlCoLQtyE == BxlJMzAmDG){oMuXMRJxDu = true;}
      if(jaIeinBZwk == ohdpozWGWV){raMprnaRKC = true;}
      while(EAgMaousWr == wjzRxiMihg){yPOQTIKubF = true;}
      while(BxlJMzAmDG == BxlJMzAmDG){yNJmlVHIHz = true;}
      while(ohdpozWGWV == ohdpozWGWV){XdMShYmGUT = true;}
      if(iIyXUuBYYF == true){iIyXUuBYYF = false;}
      if(wBBTtojWbx == true){wBBTtojWbx = false;}
      if(XUHYVWWqXb == true){XUHYVWWqXb = false;}
      if(SnWAJszNnQ == true){SnWAJszNnQ = false;}
      if(ybNxKmgJwz == true){ybNxKmgJwz = false;}
      if(OAeZxOAYBy == true){OAeZxOAYBy = false;}
      if(YKAwmHeAKA == true){YKAwmHeAKA = false;}
      if(oVhKywGJOi == true){oVhKywGJOi = false;}
      if(oMuXMRJxDu == true){oMuXMRJxDu = false;}
      if(raMprnaRKC == true){raMprnaRKC = false;}
      if(IgzuJsRTaB == true){IgzuJsRTaB = false;}
      if(IpJRILOcjU == true){IpJRILOcjU = false;}
      if(ygwPHOIqSK == true){ygwPHOIqSK = false;}
      if(UnyLZINzNS == true){UnyLZINzNS = false;}
      if(AoVBmCQthf == true){AoVBmCQthf = false;}
      if(YygpEajimu == true){YygpEajimu = false;}
      if(jaBPgTqxqA == true){jaBPgTqxqA = false;}
      if(yPOQTIKubF == true){yPOQTIKubF = false;}
      if(yNJmlVHIHz == true){yNJmlVHIHz = false;}
      if(XdMShYmGUT == true){XdMShYmGUT = false;}
    } 
}; 

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class HEWAROOKNR
{ 
  void jfXEeTsBWV()
  { 
      bool euxyGWLQKw = false;
      bool zYxbIBFiwu = false;
      bool CgbEmLTKyx = false;
      bool pcOquOfnGx = false;
      bool GuVTuiBLNI = false;
      bool rgHahxTrKu = false;
      bool DARNFVJGgt = false;
      bool URbyoGKHJD = false;
      bool JIxpEJUnIx = false;
      bool uncKtFcIOY = false;
      bool YteTmaEJlb = false;
      bool mLhHltCaIw = false;
      bool AVxiygnRDi = false;
      bool cBsjdOccDd = false;
      bool ELCgKiFVYz = false;
      bool CBzSYDwwUG = false;
      bool llFCykSxcW = false;
      bool RZXeqoTQXP = false;
      bool aSsEfbBZLa = false;
      bool RgIxqSFziY = false;
      string bONWMBKDkh;
      string AWdctuEIkS;
      string WUeOkAtyXt;
      string etgDEexITG;
      string ouNxaZDWlP;
      string ANGtyGiylL;
      string GLeqDGpZXO;
      string xwoecaHgmL;
      string wXMHsQmlFx;
      string uzWIAcQWsU;
      string jdzWREYaQW;
      string zCKZOiyXxm;
      string SDTZTyZfeA;
      string KknSaSXppo;
      string qMViysnwtR;
      string esYGCgKYCT;
      string erwQMNOrzu;
      string KHgIuGhkaa;
      string KclNURLgQt;
      string IznWJyPEVj;
      if(bONWMBKDkh == jdzWREYaQW){euxyGWLQKw = true;}
      else if(jdzWREYaQW == bONWMBKDkh){YteTmaEJlb = true;}
      if(AWdctuEIkS == zCKZOiyXxm){zYxbIBFiwu = true;}
      else if(zCKZOiyXxm == AWdctuEIkS){mLhHltCaIw = true;}
      if(WUeOkAtyXt == SDTZTyZfeA){CgbEmLTKyx = true;}
      else if(SDTZTyZfeA == WUeOkAtyXt){AVxiygnRDi = true;}
      if(etgDEexITG == KknSaSXppo){pcOquOfnGx = true;}
      else if(KknSaSXppo == etgDEexITG){cBsjdOccDd = true;}
      if(ouNxaZDWlP == qMViysnwtR){GuVTuiBLNI = true;}
      else if(qMViysnwtR == ouNxaZDWlP){ELCgKiFVYz = true;}
      if(ANGtyGiylL == esYGCgKYCT){rgHahxTrKu = true;}
      else if(esYGCgKYCT == ANGtyGiylL){CBzSYDwwUG = true;}
      if(GLeqDGpZXO == erwQMNOrzu){DARNFVJGgt = true;}
      else if(erwQMNOrzu == GLeqDGpZXO){llFCykSxcW = true;}
      if(xwoecaHgmL == KHgIuGhkaa){URbyoGKHJD = true;}
      if(wXMHsQmlFx == KclNURLgQt){JIxpEJUnIx = true;}
      if(uzWIAcQWsU == IznWJyPEVj){uncKtFcIOY = true;}
      while(KHgIuGhkaa == xwoecaHgmL){RZXeqoTQXP = true;}
      while(KclNURLgQt == KclNURLgQt){aSsEfbBZLa = true;}
      while(IznWJyPEVj == IznWJyPEVj){RgIxqSFziY = true;}
      if(euxyGWLQKw == true){euxyGWLQKw = false;}
      if(zYxbIBFiwu == true){zYxbIBFiwu = false;}
      if(CgbEmLTKyx == true){CgbEmLTKyx = false;}
      if(pcOquOfnGx == true){pcOquOfnGx = false;}
      if(GuVTuiBLNI == true){GuVTuiBLNI = false;}
      if(rgHahxTrKu == true){rgHahxTrKu = false;}
      if(DARNFVJGgt == true){DARNFVJGgt = false;}
      if(URbyoGKHJD == true){URbyoGKHJD = false;}
      if(JIxpEJUnIx == true){JIxpEJUnIx = false;}
      if(uncKtFcIOY == true){uncKtFcIOY = false;}
      if(YteTmaEJlb == true){YteTmaEJlb = false;}
      if(mLhHltCaIw == true){mLhHltCaIw = false;}
      if(AVxiygnRDi == true){AVxiygnRDi = false;}
      if(cBsjdOccDd == true){cBsjdOccDd = false;}
      if(ELCgKiFVYz == true){ELCgKiFVYz = false;}
      if(CBzSYDwwUG == true){CBzSYDwwUG = false;}
      if(llFCykSxcW == true){llFCykSxcW = false;}
      if(RZXeqoTQXP == true){RZXeqoTQXP = false;}
      if(aSsEfbBZLa == true){aSsEfbBZLa = false;}
      if(RgIxqSFziY == true){RgIxqSFziY = false;}
    } 
}; 
