#include "Stage.h"
#include "Status.h"
#include "PipeRegArray.h"
#include "WritebackStage.h"
#include "RegisterFile.h"
#include "W.h"
 
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool WritebackStage::doClockLow(PipeRegArray * pipeRegs)
{
	PipeReg * wreg = pipeRegs -> getWritebackReg();	
	uint64_t stat = wreg -> get(W_STAT);

	if (stat != Status::SAOK){
		return true;
	}
	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void WritebackStage::doClockHigh(PipeRegArray * pipeRegs)
{
	PipeReg * wreg = pipeRegs -> getWritebackReg();
	RegisterFile * reg = RegisterFile::getInstance();

	uint64_t W_valE = wreg -> get(W_VALE);
	uint64_t W_valM = wreg -> get(W_VALM);
	uint64_t W_dstE = wreg -> get(W_DSTE);
	uint64_t W_dstM = wreg -> get(W_DSTM);

	bool error = false;
	reg->writeRegister(W_valE, W_dstE, error);
	reg->writeRegister(W_valM, W_dstM, error);
}
