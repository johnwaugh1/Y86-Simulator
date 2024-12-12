#ifndef CONDITIONCODES_H
#define CONDITIONCODES_H

class ConditionCodes 
{
   private:
      static ConditionCodes * ccInstance;
      ConditionCodes();
      uint64_t codes;
   public:
      static const int SF = 7;
      static const int OF = 3;
      static const int ZF = 2;
      static ConditionCodes * getInstance();      
      bool getConditionCode(int32_t ccNum, bool & error);
      void setConditionCode(bool value, int32_t ccNum, bool & error);
      void dump();
}; 

#endif
