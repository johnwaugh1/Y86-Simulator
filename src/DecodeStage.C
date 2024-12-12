#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "RegisterFile.h"
#include "Instruction.h"
#include "Stage.h"
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool DecodeStage::doClockLow(PipeRegArray * pipeRegs)
{
	PipeReg * dreg = pipeRegs -> getDecodeReg();
	PipeReg * ereg = pipeRegs -> getExecuteReg();
	PipeReg * mreg = pipeRegs -> getMemoryReg();
	PipeReg * wreg = pipeRegs -> getWritebackReg();
	
	uint64_t stat = dreg -> get(D_STAT);
	uint64_t icode = dreg -> get(D_ICODE);
	uint64_t ifun = dreg -> get(D_IFUN);
	uint64_t valC = dreg -> get(D_VALC);
	uint64_t valP = dreg -> get(D_VALP);

	RegisterFile * rf = RegisterFile::getInstance();
	bool error = false;

	uint64_t rA = dreg -> get(D_RA);
	uint64_t rB = dreg -> get(D_RB);

	Stage::d_srcA = DecodeStage::srcA(icode, rA);
	Stage::d_srcB = DecodeStage::srcB(icode, rB);
	uint64_t e_dstE = DecodeStage::dstE(icode, rB);
	uint64_t dstM = DecodeStage::dstM(icode, rA);

	uint64_t d_rvalA = rf -> RegisterFile::readRegister(Stage::d_srcA, error);
	uint64_t d_rvalB = rf -> RegisterFile::readRegister(Stage::d_srcB, error);
	
	uint64_t valA = DecodeStage::SelFwdA(icode, valP, Stage::d_srcA, d_rvalA, mreg, wreg);
	uint64_t valB = DecodeStage::FwdB(Stage::d_srcB, d_rvalB, mreg, wreg);

	E_bubble = calculateControlSignals(ereg);

	setEInput(ereg, stat, icode, ifun, valC, valA, valB, e_dstE, dstM, Stage::d_srcA, Stage::d_srcB);

	return false;
}
/*
 * setEInput
 *
 * Updates the Execute register (E) with the specified inputs, 
 * preparing the data for the next pipeline stage.
 *
 * @param ereg - The Execute register to update.
 * @param stat - Status code.
 * @param icode - Instruction code.
 * @param ifun - Instruction function.
 * @param valC - Immediate value from instruction.
 * @param valA - Source A value.
 * @param valB - Source B value.
 * @param dstE - Destination register for computed value.
 * @param dstM - Destination register for memory.
 * @param srcA - Source register A identifier.
 * @param srcB - Source register B identifier.
 */
void DecodeStage::setEInput (PipeReg * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t  dstM, uint64_t srcA, uint64_t srcB) {

	ereg -> set(E_STAT, stat);
	ereg -> set(E_ICODE, icode);	
	ereg -> set(E_IFUN, ifun);
	ereg -> set(E_VALC, valC);
	ereg -> set(E_VALA, valA);
	ereg -> set(E_VALB, valB);
	ereg -> set(E_DSTE, dstE);
	ereg -> set(E_DSTM, dstM);
	ereg -> set(E_SRCA, srcA);
	ereg -> set(E_SRCB, srcB);
}
/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void DecodeStage::doClockHigh(PipeRegArray * pipeRegs)
{
	PipeReg * ereg = pipeRegs->getExecuteReg();
	if(!E_bubble){
		ereg->normal();
	} else {
		((E *)ereg)->bubble();
	}
}
/*
 * srcA
 *
 * Determines the source register A based on the instruction code.
 *
 * @param D_icode - Instruction code from the Decode stage.
 * @param D_rA - Register A identifier.
 * @return The determined source register A identifier.
 */
uint64_t DecodeStage::srcA (uint64_t D_icode, uint64_t D_rA)
{
	if(D_icode == Instruction::IRRMOVQ || D_icode == Instruction::IRMMOVQ || D_icode == Instruction::IOPQ || D_icode == Instruction::IPUSHQ)
	{
		return D_rA;
	}
	else if (D_icode == Instruction::IPOPQ || D_icode == Instruction::IRET)
	{
		return RegisterFile::rsp;
	}
	else
	{
		return RegisterFile::RNONE;
	}
}
/*
 * srcB
 *
 * Determines the source register B based on the instruction code.
 *
 * @param D_icode - Instruction code from the Decode stage.
 * @param D_rB - Register B identifier.
 * @return The determined source register B identifier.
 */
uint64_t DecodeStage::srcB (uint64_t D_icode, uint64_t D_rB)
{
	if(D_icode == Instruction::IOPQ || D_icode == Instruction::IRMMOVQ || D_icode == Instruction::IMRMOVQ)
	{
		return D_rB;
	}
	else if (D_icode == Instruction::IPUSHQ || D_icode == Instruction::IPOPQ || D_icode == Instruction::ICALL || D_icode == Instruction::IRET)
	{
		return RegisterFile::rsp;
	}
	else
	{
		return RegisterFile::RNONE;
	}
}
/*
 * dstE
 *
 * Determines the destination register for the computed value in the Execute stage.
 *
 * @param D_icode - Instruction code from the Decode stage.
 * @param D_rB - Register B identifier.
 * @return The determined destination register identifier for Execute stage.
 */
uint64_t DecodeStage::dstE (uint64_t D_icode, uint64_t D_rB)
{
	if(D_icode == Instruction::IRRMOVQ || D_icode == Instruction::IIRMOVQ || D_icode == Instruction::IOPQ)
	{
		return D_rB;
	}
	else if (D_icode == Instruction::IPUSHQ || D_icode == Instruction::IPOPQ || D_icode == Instruction::ICALL || D_icode == Instruction::IRET)
	{
		return RegisterFile::rsp;
	}
	else
	{
		return RegisterFile::RNONE;
	}
}
/*
 * dstM
 *
 * Determines the destination register for memory-related instructions.
 *
 * @param D_icode - Instruction code from the Decode stage.
 * @param D_rA - Register A identifier.
 * @return The determined destination register identifier for memory.
 */
uint64_t DecodeStage::dstM (uint64_t D_icode, uint64_t D_rA)
{
	if(D_icode == Instruction::IMRMOVQ || D_icode == Instruction::IPOPQ)
	{
		return  D_rA;
	}
	else
	{
		return RegisterFile::RNONE;
	}
}
/*
 * SelFwdA
 *
 * Selects the forwarded value for register A in the Decode stage. This method 
 * implements data forwarding logic to avoid stalls in the pipeline.
 *
 * @param D_icode - Instruction code from the Decode stage.
 * @param D_valP - Value of the next instruction address.
 * @param srcA - Source register A identifier.
 * @param d_rvalA - Original value read from register A.
 * @param mreg - Memory register.
 * @param wreg - Writeback register.
 * @return The selected forwarded value for register A.
 */
uint64_t DecodeStage::SelFwdA (uint64_t D_icode, uint64_t D_valP, uint64_t srcA, uint64_t d_rvalA, PipeReg * mreg, PipeReg * wreg)
{
	uint64_t M_dstE = mreg -> get(M_DSTE);
	uint64_t M_dstM = mreg -> get(M_DSTM);
	uint64_t M_valE = mreg -> get(M_VALE);
	uint64_t m_valM = Stage::m_valM;
	uint64_t W_dstE = wreg -> get(W_DSTE);
	uint64_t W_dstM = wreg -> get(W_DSTM);
	uint64_t W_valE = wreg -> get(W_VALE);
	uint64_t W_valM = wreg -> get(W_VALM);
	
	if(D_icode == Instruction::ICALL || D_icode == Instruction::IJXX) {
		return D_valP;
	} else if (srcA == RegisterFile::RNONE) {
		return 0;
	} else if (srcA == Stage::e_dstE) {
		return Stage::e_valE;
	} else if (srcA == M_dstM) {
		return m_valM;
	} else if (srcA == M_dstE) {
		return M_valE;
	} else if (srcA == W_dstM) {
		return W_valM;
	} else if (srcA == W_dstE) {
		return W_valE;
	} else {
		return d_rvalA;
	}
}
/*
 * FwdB
 *
 * Selects the forwarded value for register B in the Decode stage. This method
 * implements data forwarding logic to avoid stalls in the pipeline.
 *
 * @param srcB - Source register B identifier.
 * @param d_rvalB - Original value read from register B.
 * @param mreg - Memory register.
 * @param wreg - Writeback register.
 * @return The selected forwarded value for register B.
 */
uint64_t DecodeStage::FwdB (uint64_t srcB, uint64_t d_rvalB, PipeReg * mreg, PipeReg * wreg)
{
	uint64_t M_dstE = mreg -> get(M_DSTE);
	uint64_t M_dstM = mreg -> get(M_DSTM);
	uint64_t M_valE = mreg -> get(M_VALE);
	uint64_t W_dstE = wreg -> get(W_DSTE);
	uint64_t W_dstM = wreg -> get(W_DSTM);
	uint64_t W_valE = wreg -> get(W_VALE);
	uint64_t W_valM = wreg -> get(W_VALM);

	if (srcB == RegisterFile::RNONE) {
		return 0;
	} else if (srcB == Stage::e_dstE) {
		return Stage::e_valE;
	} else if (srcB == M_dstM) {
		return Stage::m_valM;
	} else if (srcB == M_dstE) {
		return M_valE;
	} else if (srcB == W_dstM) {
		return W_valM;
	} else if (srcB == W_dstE) {
		return W_valE;
	} else {
		return d_rvalB;
	}
}
/*
 * calculateControlSignals
 *
 * Determines whether a bubble (stall or flush) is required for the Execute 
 * register (E) based on dependencies and the current instruction flow.
 *
 * @param ereg - The Execute register.
 * @return True if a bubble is required, otherwise false.
 */
bool DecodeStage::calculateControlSignals(PipeReg * ereg){
	uint64_t E_icode = ereg -> get(E_ICODE);
	uint64_t E_dstM = ereg -> get(E_DSTM);

	return (E_icode == Instruction::IJXX && !Stage::e_Cnd) || ((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB));
}
