#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "RegisterFile.h"
#include "W.h"
#include "M.h"
#include "Instruction.h"
#include "Memory.h"
#include "Status.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool MemoryStage::doClockLow(PipeRegArray * pipeRegs)
{
	PipeReg * mreg = pipeRegs -> getMemoryReg();
	PipeReg * wreg = pipeRegs -> getWritebackReg();

	uint64_t stat = mreg -> get(M_STAT);
	uint64_t icode = mreg -> get(M_ICODE);
	uint64_t valE = mreg -> get(M_VALE);
	uint64_t valA = mreg -> get(M_VALA);
	uint64_t dstE = mreg -> get(M_DSTE);
	uint64_t dstM = mreg -> get(M_DSTM);

	Stage::m_valM = 0;

	Memory * mem = Memory::getInstance();
	bool error = false;
	uint64_t mem_address = mem_addr(icode, valE, valA);

	if(mem_read(icode)) Stage::m_valM = mem -> getLong(mem_address, error);

	if(mem_write(icode)) mem -> putLong(valA, mem_address, error);

	if(error){
		Stage::m_stat = Status::SADR;
	} else {
		Stage::m_stat = stat;
	}

	setWInput(wreg, Stage::m_stat, icode, valE, Stage::m_valM, dstE, dstM);
	return false;
}
/*
 * setWInput
 *
 * Provides the input values to be stored in the Writeback register during the Memory stage.
 *
 * @param: wreg - pointer to the Writeback register instance
 * @param: stat - status value to be set in the Writeback register
 * @param: icode - instruction code to be set in the Writeback register
 * @param: valE - value to be set in the valE pipeline register within Writeback
 * @param: valM - value to be set in the valM pipeline register within Writeback
 * @param: dstE - destination register for the instruction in Writeback
 * @param: dstM - destination register for the memory operation in Writeback
 * 
 * This function sets the values of the Writeback register (W) using the status, icode,
 * and calculated values from the Memory stage (valE, valM, dstE, dstM).
 */
void MemoryStage::setWInput(PipeReg * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM)
{
	wreg -> set(W_STAT, stat);
	wreg -> set(W_ICODE, icode);
	wreg -> set(W_VALE, valE);	
	wreg -> set(W_VALM, valM);	
	wreg -> set(W_DSTE, dstE);	
	wreg -> set(W_DSTM, dstM);
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void MemoryStage::doClockHigh(PipeRegArray * pipeRegs)
{
	PipeReg * wreg = pipeRegs->getWritebackReg();
	wreg -> normal();
}
/*
 * mem_addr
 *
 * Calculates the memory address based on the current instruction opcode and its operands.
 *
 * @param: M_icode - the icode value of the current instruction
 * @param: M_valE - the value from the Execute stage
 * @param: M_valA - the value from the Address (A) stage
 * 
 * This function calculates the appropriate memory address based on the opcode of the instruction.
 * It handles different opcodes like IRMMOVQ, IPUSHQ, ICALL, IMRMOVQ, IPOPQ, and IRET, which
 * use different operands to compute the memory address.
 * 
 * @return: uint64_t - the computed memory address.
 */
uint64_t MemoryStage::mem_addr(uint64_t M_icode, uint64_t M_valE, uint64_t M_valA)
{
	if(M_icode == Instruction::IRMMOVQ || M_icode == Instruction::IPUSHQ || M_icode == Instruction::ICALL || M_icode == Instruction::IMRMOVQ) {
		return M_valE;
	}
	if(M_icode == Instruction::IPOPQ || M_icode == Instruction::IRET) {
		return M_valA;
	}
	else {
		return 0;
	}
}
/*
 * mem_read
 *
 * Checks if the current instruction requires a memory read operation.
 *
 * @param: M_icode - the icode value of the current instruction
 * 
 * This function checks if the current instruction needs to read from memory. It returns true 
 * if the opcode is one of the read instructions (IMRMOVQ, IPOPQ, IRET).
 * 
 * @return: bool - true if a memory read is required, false otherwise.
 */
bool MemoryStage::mem_read(uint64_t M_icode)
{
	return M_icode == Instruction::IMRMOVQ || M_icode == Instruction::IPOPQ || M_icode == Instruction::IRET;
}
/*
 * mem_write
 *
 * Checks if the current instruction requires a memory write operation.
 *
 * @param: M_icode - the icode value of the current instruction
 * 
 * This function checks if the current instruction needs to write to memory. It returns true 
 * if the opcode is one of the write instructions (IRMMOVQ, IPUSHQ, ICALL).
 * 
 * @return: bool - true if a memory write is required, false otherwise.
 */
bool MemoryStage::mem_write(uint64_t M_icode)
{
	return M_icode == Instruction::IRMMOVQ || M_icode == Instruction::IPUSHQ || M_icode == Instruction::ICALL;
}

