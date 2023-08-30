// Preprocess.h

#ifndef _PREPROCESS_h
#define _PREPROCESS_h
//Sử dụng các phép tính cơ sở như cộng, trừ nhân, chia từ bộ nhớ hoặc từ chính giá trị đưa vào
#define USING_CALC_BASE 0
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "Memory.h" 

class PreprocessClass
{
protected:

	//Event khi thực hiện execute 1 biểu thức
	void(*_onRun)();
	void(*_onListen)();
public:
	MemoryClass memoryEx;
	const String Math = ",REPLACE,TRIM,PADLEFT,PADRIGHT,SUBSTRING,REMOVE,INDEXOF,LASTINDEXOF,";
	void init(MemoryClass& memory);
#if USING_CALC_BASE	
	//Phép cộng
	String Add(String varA, String varB, uint8_t decimalplace);
	//Phép trừ
	String Sub(String varA, String varB, uint8_t decimalplace);
	//Phép nhân
	String Mul(String varA, String varB, uint8_t decimalplace);
	//Phép chia
	String Div(String varA, String varB, uint8_t decimalplace);
#endif
	//Hàm tìm nhỏ nhất
	double Min(double  vara, double  varb);
	//Nhỏ nhất và lớn hơn =0
	double Min(double  vara, double  varb, bool  less_zero);
	//Hàm tính max 
	double Max(double  vara, double  varb);

	//Phép cộng số tĩnh
	inline String valueAdd(String varA, String varB, uint8_t decimalplace);
	//Phép trừ số tĩnh
	inline String valueSub(String varA, String varB, uint8_t decimalplace);
	//Phép nhân số tĩnh
	inline String valueMul(String varA, String varB, uint8_t decimalplace);
	//Phép chia số tĩnh
	inline String valueDiv(String varA, String varB, uint8_t decimalplace);
	//Quét biểu thức so sánh trong logic
	String DeepExpression(String& logic);
	//Quét phép tính trong ngoặc
	String DeepOC(String logic);
	//Hàm thực hiện tính toán khi đã loại bỏ ngoặc ()
	String Calculate(String logic);
	//Thay thế biến bằng giá trị
	String replaceVariables(String logic);
	//Lấy giá trị từ biến nhớ đưa vào tính toán, nếu ko có thì trả về dạng số
	String GetValue(String name);
	//Dùng cho các hàm tính như Pi, logarit,...
	String CheckStaticFunction(String name);
	//Thực thi 1 chuỗi lệnh tính toán 
	String Execute(String logic);
	//Event xảy ra khi tính 1 biểu thức ở Execute
	void onRun(void(*callback)());
	// hoạt động như 1 timer
	void onListen(void(*callback)());
	//đánh giá biểu thức điều kiện IF, WHILE, FOR 
	bool ExecuteExpression(String logic);
	String CompareOperan(String logic);
	//So sánh điều kiện
	String Compare(String logic);
	//Tìm biểu thức tính, thực hiện tính và thay thế kết quả trước khi so sánh
	String replaceExpression(String& logic);
	//Kiểm tra cặp ngoặc đóng mở
	String checkOpen(String logic, String ck, bool reverse);
	////Biểu thức If
	//bool If(String logic);
	////Biểu thức vòng lặp có điều kiện
	//bool While(String logic);
	////Biểu thức vòng lặp có điều kiện
	//bool For(String logic);
	//Join các chuối và biến lại với nhau
	String Join(String logic);
};

extern PreprocessClass Preprocess;

#endif

