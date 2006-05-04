////////////////////////////////////////////////////////////////// // (c) Copyright 2003-  by Jeongnim Kim
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//   National Center for Supercomputing Applications &
//   Materials Computation Center
//   University of Illinois, Urbana-Champaign
//   Urbana, IL 61801
//   e-mail: jnkim@ncsa.uiuc.edu
//   Tel:    217-244-6319 (NCSA) 217-333-3324 (MCC)
//
// Supported by 
//   National Center for Supercomputing Applications, UIUC
//   Materials Computation Center, UIUC
//////////////////////////////////////////////////////////////////
// -*- C++ -*-
#include "Utilities/OhmmsInfo.h"
#include "Particle/DistanceTableData.h"
#include "Particle/DistanceTable.h"
#include "QMCWaveFunctions/NJABBuilder.h"
#include "QMCWaveFunctions/PadeJastrow.h"
#include "QMCWaveFunctions/NoCuspJastrow.h"
#include "QMCWaveFunctions/OneBodyJastrowFunction.h"
#include "QMCFactory/OneDimGridFactory.h"

namespace qmcplusplus {


  /** constructor
   * @param p target ParticleSet whose wave function is to be initialized
   *@param psi the wavefunction
   *@param psets a vector containing the ParticleSets
   * 
   *Jastrow wavefunctions chose the needed distance tables and the 
   *DistanceTableData objects are initialized based on the source 
   *and target particle sets.
   */
  NJABBuilder::NJABBuilder(ParticleSet& p, TrialWaveFunction& psi, PtclPoolType& psets):
    OrbitalBuilderBase(p,psi), ptclPool(psets), gridPtr(0), sourcePtcl(0)
  { }

  NJABBuilder::InFuncType* 
  NJABBuilder::createInFunc(const string& jastfunction) {
    if(jastfunction == "nocusp") {
      return new NoCuspJastrow<ValueType>;
    } else if(jastfunction == "pade") {
      return new PadeJastrow<ValueType>;
    } else if(jastfunction == "pade2") {
      return new PadeJastrow2<ValueType>;
    }
    return 0;
  }

  bool NJABBuilder::putInFunc(xmlNodePtr cur) {

    string corr_tag("correlation");
    string jastfunction("pade");
    int	ng=1;

    const xmlChar *ftype = xmlGetProp(cur, (const xmlChar *)"function");
    if(ftype != NULL) jastfunction = (const char*) ftype;
    const xmlChar* s=xmlGetProp(cur,(const xmlChar*)"source");
    if(s != NULL) {
      map<string,ParticleSet*>::iterator pa_it(ptclPool.find((const char*)s));
      if(pa_it == ptclPool.end()) return false;
      sourcePtcl = (*pa_it).second;
      ng=sourcePtcl->getSpeciesSet().getTotalNum();
    }

    int ia=0, ib=0, iab=0;
    cur = cur->children;
    while(cur != NULL) {
      string cname((const char*)(cur->name));
      if(cname == "grid") {
        gridPtr=cur; //save the pointer
      } else if(cname == dtable_tag) {
      	string source_name((const char*)(xmlGetProp(cur,(const xmlChar *)"source")));
        map<string,ParticleSet*>::iterator pa_it(ptclPool.find(source_name));
        if(pa_it == ptclPool.end()) return false;
	sourcePtcl=(*pa_it).second;
	ng = sourcePtcl->getSpeciesSet().getTotalNum();
	XMLReport("Number of sources " << ng)
        InFunc.resize(ng,0);
      } else if(cname ==corr_tag) {
        if(sourcePtcl==0) return false;
	string spA((const char*)(xmlGetProp(cur,(const xmlChar *)"speciesA")));
        ia = sourcePtcl->getSpeciesSet().findSpecies(spA);
	if(!(InFunc[ia])) {
          InFuncType *j1=createInFunc(jastfunction);
	  InFunc[ia]= j1;
	  app_log() <<"   Added Jastrow Correlation between " <<spA<<" and "<<targetPtcl.getName() << endl;
	}
	InFunc[ia]->put(cur,targetPsi.VarList);
      }
      cur = cur->next;
    } // while cur
    
    return true;
  }
  
  bool NJABBuilder::put(xmlNodePtr cur) {

    //create analytic functions 
    bool success = putInFunc(cur);

    //create grid and initialize CubicSplineFunctions
    OneDimGridFactory::GridType* agrid = OneDimGridFactory::createGrid(gridPtr);

    //get the cutoff value to smooth the function
    RealType rcut = OneDimGridFactory::setSmoothCutoff(agrid,gridPtr);

    OneBodyJastrow<FuncType> *J1 = new OneBodyJastrow<FuncType>(*sourcePtcl, targetPtcl);
    for(int i=0; i<InFunc.size(); i++) {
      if(InFunc[i]) {
        FuncType* ofunc= new FuncType;
        ofunc->setInFunc(InFunc[i]);
        ofunc->setOutFunc(new OutFuncType(agrid));
        ofunc->setCutoff(rcut,agrid->rmax());
        ofunc->reset();
        J1->addFunc(i,ofunc);
      } else {
        J1->addFunc(i,0);
      }
    }

    J1->setOptimizable(true);
    targetPsi.addOrbital(J1);
    XMLReport("Added a One-Body Jastrow Function")
    return success;
  }
}
/***************************************************************************
 * $RCSfile$   $Author$
 * $Revision$   $Date$
 * $Id$ 
 ***************************************************************************/
