#include "../com/ecmcAsynDataItem.h"
#include "../com/ecmcAsynPortDriver.h"
#include <stdio.h>
#include <string.h>

ecmcAsynDataItem::ecmcAsynDataItem (ecmcAsynPortDriver *asynPortDriver, const char *paramName,asynParamType asynParType)
{
  asynPortDriver_=asynPortDriver;  
  data_=0;
  bytes_=0;
  asynUpdateCycleCounter_=0;
  supportedTypesCounter_=0;
  allowWriteToEcmc_=false;
  validated_=false;
  for(int i=0;i<ERROR_ASYN_MAX_SUPPORTED_TYPES_COUNT;i++) {
    supportedTypes_[i]=asynParamNotDefined;
  }
  paramInfo_= new ecmcParamInfo();
  memset(paramInfo_,0,sizeof(ecmcParamInfo));
  paramInfo_->name=strdup(paramName);
  paramInfo_->asynType=asynParType;
  paramInfo_->ecmcDataIsArray = asynTypeIsArray(asynParType);
  addSupportedAsynType(asynParType);
}

ecmcAsynDataItem::~ecmcAsynDataItem ()
{
  delete paramInfo_->recordName;
  delete paramInfo_->recordType;
  delete paramInfo_->scan;
  delete paramInfo_->dtyp;
  delete paramInfo_->inp;
  delete paramInfo_->out;
  delete paramInfo_->drvInfo;
  delete paramInfo_->asynTypeStr;
  delete paramInfo_->name;
  delete paramInfo_;
}

int ecmcAsynDataItem::setEcmcDataPointer(uint8_t *data,size_t bytes)
{
  data_=data;
  paramInfo_->ecmcSize=bytes;
  paramInfo_->ecmcMaxSize=bytes;
  paramInfo_->ecmcDataPointerValid =  data && bytes>0;
  return 0;
}

int ecmcAsynDataItem::refreshParamRT(int force)
{
  if(!asynPortDriver_->getAllowRtThreadCom()){
    return ERROR_ASYN_NOT_REFRESHED_RETURN;
  }
  return refreshParam(force,data_,paramInfo_->ecmcSize);
}

int ecmcAsynDataItem::refreshParam(int force)
{
  return refreshParam(force,data_,paramInfo_->ecmcSize);
}

int ecmcAsynDataItem::refreshParamRT(int force, size_t bytes)
{
  if(!asynPortDriver_->getAllowRtThreadCom()){
    return ERROR_ASYN_NOT_REFRESHED_RETURN;
  }
  return refreshParam(force,data_,bytes);
}

int ecmcAsynDataItem::refreshParam(int force, size_t bytes)
{
  return refreshParam(force,data_,bytes);
}

int ecmcAsynDataItem::refreshParamRT(int force,uint8_t *data, size_t bytes)
{
  if(!asynPortDriver_->getAllowRtThreadCom()){
    return ERROR_ASYN_NOT_REFRESHED_RETURN;
  }
  return refreshParam(force,data,bytes);
}

/*
* Returns 0 if refreshed.
* Retrun -1 or error code if not refreshed. 
*/
int ecmcAsynDataItem::refreshParam(int force,uint8_t *data, size_t bytes)
{
  if(!paramInfo_->initialized) {
    return 0;
  }

  if(!validated_) {
    return ERROR_ASYN_PARAM_NOT_VALIDATED;
  }

  if(asynUpdateCycleCounter_< paramInfo_->sampleTimeCycles-1 && !force){ //Only update at desired samplerate
    asynUpdateCycleCounter_++;
    return ERROR_ASYN_NOT_REFRESHED_RETURN;  //Not refreshed
  }

  if(data==0 || bytes<0){
    return ERROR_ASYN_DATA_NULL;
  }

  if(bytes > paramInfo_->ecmcMaxSize) {
    bytes = paramInfo_->ecmcMaxSize;
  }

  data_=data;
  paramInfo_->ecmcSize=bytes;  //Last refresh size
 
  switch(paramInfo_->asynType){
    case asynParamUInt32Digital:
      asynPortDriver_->setUIntDigitalParam(paramInfo_->index,*((epicsInt32*)data),0xFFFFFFFF);
      break;
    case asynParamInt32:
      asynPortDriver_->setIntegerParam(paramInfo_->index,*((epicsInt32*)data));
      break;
    case asynParamFloat64:
      asynPortDriver_->setDoubleParam(paramInfo_->index,*((epicsFloat64*)data));
      break;
    case asynParamInt8Array:
      asynPortDriver_->doCallbacksInt8Array((epicsInt8*)data,bytes, paramInfo_->index, 0);
      break;
    case asynParamInt16Array:
      asynPortDriver_->doCallbacksInt16Array((epicsInt16*)data,bytes/sizeof(epicsInt16), paramInfo_->index, 0);
      break;
    case asynParamInt32Array:
      asynPortDriver_->doCallbacksInt32Array((epicsInt32*)data,bytes/sizeof(epicsInt32), paramInfo_->index, 0);
      break;
    case asynParamFloat32Array:
      asynPortDriver_->doCallbacksFloat32Array((epicsFloat32*)data,bytes/sizeof(epicsFloat32), paramInfo_->index, 0);
      break;
    case asynParamFloat64Array:
      asynPortDriver_->doCallbacksFloat64Array((epicsFloat64*)data,bytes/sizeof(epicsFloat64), paramInfo_->index, 0);
      break;
    default:
      return ERROR_ASYN_DATA_TYPE_NOT_SUPPORTED;
      break;
  }

  asynUpdateCycleCounter_=0;
  return 0;
}

int ecmcAsynDataItem::createParam()
{    
  return createParam(paramInfo_->name,paramInfo_->asynType);
}

int ecmcAsynDataItem::createParam(const char *paramName,asynParamType asynParType,uint8_t *data,size_t bytes)
{
  setEcmcDataPointer(data,bytes);
  return createParam(paramName,asynParType);
}

int ecmcAsynDataItem::createParam(const char *paramName,asynParamType asynParType)
{
  if(asynPortDriver_==0){
    return ERROR_ASYN_PORT_NULL;
  }
  paramInfo_->name=strdup(paramName);  
  paramInfo_->asynType=asynParType;  
  asynStatus status = asynPortDriver_->createParam(paramName,paramInfo_->asynType,&paramInfo_->index);
  return (status==asynSuccess) ? 0 : ERROR_ASYN_CREATE_PARAM_FAIL;
}

int ecmcAsynDataItem::getAsynParameterIndex()
{
  return paramInfo_->index;
}

int ecmcAsynDataItem::setAsynParameterType(asynParamType parType)
{
  paramInfo_->asynType=parType;
  return 0;
}

int ecmcAsynDataItem::getAsynParameterType()
{
  return paramInfo_->asynType;
}

int ecmcAsynDataItem::setAsynPortDriver(ecmcAsynPortDriver *asynPortDriver)
{
  asynPortDriver_=asynPortDriver;
  return 0;
}

int ecmcAsynDataItem::setAsynParSampleTimeMS(double sampleTime)
{
  paramInfo_->sampleTimeMS=sampleTime;
  paramInfo_->sampleTimeCycles=(int32_t)(sampleTime/1000.0*(double)MCU_FREQUENCY);
  return 0;
}

ecmcParamInfo *ecmcAsynDataItem::getParamInfo()
{
  return paramInfo_;
}

int ecmcAsynDataItem::validate() {

  if(asynPortDriver_==0){
    return ERROR_ASYN_PORT_NULL;
  }

  validated_=true;
  return 0;
}

bool ecmcAsynDataItem::initialized() {
  return paramInfo_->initialized;
}

int32_t ecmcAsynDataItem::getSampleTimeCycles() {
  return paramInfo_->sampleTimeCycles;
}

char *ecmcAsynDataItem::getName() {  
  return paramInfo_->name;
}

int ecmcAsynDataItem::addSupportedAsynType(asynParamType type) {
  
  //check so not already in list
  if(asynTypeSupported(type)) {
    return 0;
  }
  
  if(supportedTypesCounter_<ERROR_ASYN_MAX_SUPPORTED_TYPES_COUNT-1) {
    supportedTypes_[supportedTypesCounter_]=type;
    supportedTypesCounter_++;
    return 0;
  }
  return ERROR_ASYN_SUPPORTED_TYPES_ARRAY_FULL;
}

bool ecmcAsynDataItem::asynTypeSupported(asynParamType type) {
  for(int i=0;i<supportedTypesCounter_;i++) {
   
   if (supportedTypes_[i]==type) {
     return true;
   }
  }
  return false;
}

int ecmcAsynDataItem::getSupportedAsynTypeCount() {
  return supportedTypesCounter_;
}

asynParamType ecmcAsynDataItem::getSupportedAsynType(int index) {
  if(index<supportedTypesCounter_) {
    return supportedTypes_[index];
  }
  return asynParamNotDefined;
}

void ecmcAsynDataItem::allowWriteToEcmc(bool allowWrite) {
  allowWriteToEcmc_ = allowWrite;
}

bool ecmcAsynDataItem::writeToEcmcAllowed() {
  return allowWriteToEcmc_;
}

int ecmcAsynDataItem::writeParam(uint8_t *dataToWrite, size_t bytes) {

  if(!paramInfo_->initialized || !allowWriteToEcmc_) {
    return 0;
  }
  
  if(!validated_) {
     return ERROR_ASYN_PARAM_NOT_VALIDATED;
  }

  if(!dataToWrite || !data_ || bytes<0){
    return ERROR_ASYN_DATA_NULL;
  }
  
  int bytesToWrite=bytes;
  if(bytes > paramInfo_->ecmcMaxSize) {
    bytesToWrite = paramInfo_->ecmcMaxSize;
  }
  
  //Need to check type...
  memcpy(data_,dataToWrite,bytesToWrite);

  return 0;
}

bool ecmcAsynDataItem::willRefreshNext() {
  return asynUpdateCycleCounter_>= paramInfo_->sampleTimeCycles-2;
}

/** Set parameter alarm state.
 *
 * \param[in] paramInfo Parameter information.
 * \param[in] alarm Alarm type (EPICS def).
 * \param[in] severity Alarm severity (EPICS def).
 *
 * \return asynSuccess or asynError.
 */
asynStatus ecmcAsynDataItem::setAlarmParam(int alarm,int severity)
{
  asynStatus stat;
  int oldAlarmStatus=0;
  stat = asynPortDriver_->getParamAlarmStatus(getAsynParameterIndex(),&oldAlarmStatus);
  if(stat!=asynSuccess){
    //asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s: getParamAlarmStatus failed for parameter %s (%d).\n", driverName, functionName,paramInfo_->drvInfo,paramInfo_->paramIndex);
    return asynError;
  }

  bool doCallbacks=false;

  if(oldAlarmStatus!=alarm){
    stat = asynPortDriver_->setParamAlarmStatus(getAsynParameterIndex(),alarm);
    if(stat!=asynSuccess){
      //asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s: Failed set alarm status for parameter %s (%d).\n", driverName, functionName,paramInfo_->drvInfo,paramInfo_->paramIndex);
      return asynError;
    }
    paramInfo_->alarmStatus=alarm;
    doCallbacks=true;
  }

  int oldAlarmSeverity=0;
  stat = asynPortDriver_->getParamAlarmSeverity(getAsynParameterIndex(),&oldAlarmSeverity);
  if(stat!=asynSuccess){
    //asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s: getParamAlarmStatus failed for parameter %s (%d).\n", driverName, functionName,paramInfo_->drvInfo,paramInfo_->paramIndex);
    return asynError;
  }

  if(oldAlarmSeverity!=severity){
    stat = asynPortDriver_->setParamAlarmSeverity(getAsynParameterIndex(),severity);
    if(stat!=asynSuccess){
      //asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s: Failed set alarm severity for parameter %s (%d).\n", driverName, functionName,paramInfo_->drvInfo,paramInfo_->paramIndex);
      return asynError;
    }
    paramInfo_->alarmSeverity=severity;
    doCallbacks=true;
  }

  if(!doCallbacks || !asynPortDriver_->getAllowRtThreadCom()){
    return asynSuccess;
  }
  
  //Alarm status or severity changed=>Do callbacks with old buffered data (if nElemnts==0 then no data in record...)
  if(paramInfo_->ecmcDataIsArray && paramInfo_->ecmcSize>0){
    refreshParamRT(1);
  }
  else{
      stat = asynPortDriver_->callParamCallbacks();
  }

  return stat;
}

int ecmcAsynDataItem::getAlarmStatus() {
  return paramInfo_->alarmStatus;
}

int ecmcAsynDataItem::getAlarmSeverity() {
  return paramInfo_->alarmSeverity;
}

int ecmcAsynDataItem::asynTypeIsArray(asynParamType asynParType) {

  switch(asynParType){
    case asynParamInt8Array:
      return 1;      
      break;
    case asynParamInt16Array:
      return 1;
      break;
    case asynParamInt32Array:
      return 1;
      break;
    case asynParamFloat32Array:
      return 1;
      break;
    case asynParamFloat64Array:
      return 1;
      break;
    default:
      return 0;
      break;
  }
  return 0;
}