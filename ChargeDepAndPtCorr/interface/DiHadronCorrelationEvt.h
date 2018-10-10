#ifndef DiHadronCorrelationEvt_
#define DiHadronCorrelationEvt_
#include <vector>
#include <TVector3.h>
#include <TLorentzVector.h>

class DiHadronCorrelationEvt {                                                                                  

 public:                                                                                           
   std::vector< std:: vector<TLorentzVector> >  pVect_trg;
   std::vector<double>                    nMultCorrVect_trg;
   std::vector< std:: vector<double> >    chgVect_trg;
   std::vector< std:: vector<double> >    effVect_trg;
   std::vector< std:: vector<double> >    phiWVect_trg;
   std::vector< std:: vector<TLorentzVector> >  pVect_ass;
   std::vector<double>                    nMultCorrVect_ass;
   std::vector< std:: vector<double> >    chgVect_ass;
   std::vector< std:: vector<double> >    effVect_ass;
   std::vector< std:: vector<double> >    phiWVect_ass;
   int    run;
   int    event;
   double zvtx;
   double cent;
   double noff;
 
   bool operator<(const DiHadronCorrelationEvt & b) const{
     if (zvtx != b.zvtx) return zvtx < b.zvtx;
        else return false;
   }

   DiHadronCorrelationEvt()
   {
     pVect_trg.resize(10); 
     nMultCorrVect_trg.resize(10); 
     chgVect_trg.resize(10);
     effVect_trg.resize(10);
     phiWVect_trg.resize(10);

     pVect_ass.resize(10); 
     nMultCorrVect_ass.resize(10); 
     chgVect_ass.resize(10);
     effVect_ass.resize(10);
     phiWVect_ass.resize(10);
   
   }

   DiHadronCorrelationEvt(unsigned int size_trg, unsigned int size_ass)
   {
     pVect_trg.resize(size_trg); 
     nMultCorrVect_trg.resize(size_trg); 
     chgVect_trg.resize(size_trg);
     effVect_trg.resize(size_trg);
     phiWVect_trg.resize(size_trg);

     pVect_ass.resize(size_ass); 
     nMultCorrVect_ass.resize(size_ass); 
     chgVect_ass.resize(size_ass);
     effVect_ass.resize(size_ass);
     phiWVect_ass.resize(size_ass);

   }

   ~DiHadronCorrelationEvt()
   {
     pVect_trg.clear(); 
     nMultCorrVect_trg.clear(); 
     chgVect_trg.clear();
     effVect_trg.clear();
     phiWVect_trg.clear();
     
     std::vector< std:: vector<TLorentzVector> >().swap(pVect_trg); 
     std::vector<double>().swap(nMultCorrVect_trg); 
     std::vector< std:: vector<double> >().swap(chgVect_trg);
     std::vector< std:: vector<double> >().swap(effVect_trg);
     std::vector< std:: vector<double> >().swap(phiWVect_trg);

     pVect_ass.clear(); 
     nMultCorrVect_ass.clear(); 
     chgVect_ass.clear();
     effVect_ass.clear();
     phiWVect_ass.clear();
     
     std::vector< std:: vector<TLorentzVector> >().swap(pVect_ass); 
     std::vector<double>().swap(nMultCorrVect_ass); 
     std::vector< std:: vector<double> >().swap(chgVect_ass);
     std::vector< std:: vector<double> >().swap(effVect_ass);
     std::vector< std:: vector<double> >().swap(phiWVect_ass);
   }

   void reset()
   {
     unsigned int size = 0;
     
     size = pVect_trg.size();
     pVect_trg.clear();
     std::vector< std:: vector<TLorentzVector> >().swap(pVect_trg);
     pVect_trg.resize(size);

     size = nMultCorrVect_trg.size();
     nMultCorrVect_trg.clear(); 
     std::vector<double>().swap(nMultCorrVect_trg);
     nMultCorrVect_trg.resize(size);

     size = chgVect_trg.size();
     chgVect_trg.clear();
     std::vector< std:: vector<double> >().swap(chgVect_trg);
     chgVect_trg.resize(size);

     size = effVect_trg.size();
     effVect_trg.clear();
     std::vector< std:: vector<double> >().swap(effVect_trg);
     effVect_trg.resize(size);

     size = phiWVect_trg.size();
     phiWVect_trg.clear();
     std::vector< std:: vector<double> >().swap(phiWVect_trg);
     phiWVect_trg.resize(size);

     size = pVect_ass.size();
     pVect_ass.clear();
     std::vector< std:: vector<TLorentzVector> >().swap(pVect_ass);
     pVect_ass.resize(size);
 
     size = nMultCorrVect_ass.size();
     nMultCorrVect_ass.clear(); 
     std::vector<double>().swap(nMultCorrVect_ass);
     nMultCorrVect_ass.resize(size);

     size = chgVect_ass.size();
     chgVect_ass.clear();
     std::vector< std:: vector<double> >().swap(chgVect_ass);
     chgVect_ass.resize(size);

     size = effVect_ass.size();
     effVect_ass.clear();
     std::vector< std:: vector<double> >().swap(effVect_ass);
     effVect_ass.resize(size);

     size = phiWVect_ass.size();
     phiWVect_ass.clear();
     std::vector< std:: vector<double> >().swap(phiWVect_ass);
     phiWVect_ass.resize(size);

     run   = -999;
     event = -999;
     zvtx  = -999.;
   }
};
   
#endif  // DiHadronCorrelationEvt_
