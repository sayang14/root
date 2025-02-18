/*
 * Project: RooFit
 * Authors:
 *   Jonas Rembser, CERN 2021
 *   Emmanouil Michalainas, CERN 2021
 *
 * Copyright (c) 2021, CERN
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted according to the terms
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)
 */

#ifndef RooFit_RooFitDriver_h
#define RooFit_RooFitDriver_h

#include <RooBatchCompute/DataKey.h>
#include <RooGlobalFunc.h>
#include <RooHelpers.h>

#include <chrono>
#include <memory>
#include <stack>

class RooAbsArg;
class RooAbsReal;
class RooAbsCategory;
class RooAbsData;
namespace RooFit {
class NormalizationIntegralUnfolder;
}

namespace ROOT {
namespace Experimental {

struct NodeInfo;

class RooFitDriver {
public:
   ////////////////////
   // Enums and aliases

   using DataSpansMap = std::map<const TNamed *, RooSpan<const double>>;

   //////////////////////////
   // Public member functions

   RooFitDriver(const RooAbsReal &absReal, RooArgSet const &normSet,
                RooFit::BatchModeOption batchMode = RooFit::BatchModeOption::Cpu);

   void setData(RooAbsData const &data, std::string_view rangeName = "",
                RooAbsCategory const *indexCatForSplitting = nullptr);
   void setData(DataSpansMap const &dataSpans);

   ~RooFitDriver();
   std::vector<double> getValues();
   double getVal();
   RooAbsReal const &topNode() const;

private:
   ///////////////////////////
   // Private member functions

   double getValHeterogeneous();
   std::chrono::microseconds simulateFit(std::chrono::microseconds h2dTime, std::chrono::microseconds d2hTime,
                                         std::chrono::microseconds diffThreshold);
   void markGPUNodes();
   void assignToGPU(const RooAbsArg *node);
   void computeCPUNode(const RooAbsArg *node, NodeInfo &info);
   void setOperMode(RooAbsArg *arg, RooAbsArg::OperMode opMode);
   void determineOutputSizes();
   bool isInComputationGraph(RooAbsArg const *arg) const;

   ///////////////////////////
   // Private member variables

   const RooFit::BatchModeOption _batchMode = RooFit::BatchModeOption::Off;
   int _getValInvocations = 0;
   double *_cudaMemDataset = nullptr;

   // used for preserving static info about the computation graph
   RooBatchCompute::DataMap _dataMapCPU;
   RooBatchCompute::DataMap _dataMapCUDA;
   std::map<RooBatchCompute::DataKey, NodeInfo> _nodeInfos;

   // the ordered computation graph
   RooArgList _orderedNodes;

   // used for preserving resources
   std::vector<double> _nonDerivedValues;
   std::stack<std::vector<double>> _vectorBuffers;
   std::unique_ptr<RooFit::NormalizationIntegralUnfolder> _integralUnfolder;

   // RAII structures to reset state of computation graph after driver destruction
   std::stack<RooHelpers::ChangeOperModeRAII> _changeOperModeRAIIs;
};

} // end namespace Experimental
} // end namespace ROOT

#endif
