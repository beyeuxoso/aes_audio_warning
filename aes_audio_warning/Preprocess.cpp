// 
// 
// 

#include "Preprocess.h"
#include "stdlib_noniso.h"


PreprocessClass Preprocess;

void PreprocessClass::init(MemoryClass& memory)
{
	memoryEx = memory;
}
#pragma region using calc base
#if USING_CALC_BASE	

String PreprocessClass::Add(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(memoryEx.GetValue(varA).c_str()) + atof(memoryEx.GetValue(varB).c_str());
	return String(c, decimalplace);
}

String PreprocessClass::Sub(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(memoryEx.GetValue(varA).c_str()) - atof(memoryEx.GetValue(varB).c_str());
	return String(c, decimalplace);
}

String PreprocessClass::Mul(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(memoryEx.GetValue(varA).c_str()) * atof(memoryEx.GetValue(varB).c_str());
	return String(c, decimalplace);
}

String PreprocessClass::Div(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(memoryEx.GetValue(varA).c_str()) / atof(memoryEx.GetValue(varB).c_str());
	return String(c, decimalplace);
}
#endif  
#pragma endregion

#pragma region Quét biểu thức và phương pháp tính
/*Thực hiện tính*/
inline double PreprocessClass::Min(double  vara, double  varb)
{
	return vara >= varb ? varb : vara;
}
inline double PreprocessClass::Min(double  vara, double  varb, bool  less_zero)
{
	if (!less_zero)
	{
		if (vara < 0 && varb < 0) return -1;
		if (vara < 0) return varb;
		if (varb < 0) return vara;
	}
	return vara >= varb ? varb : vara;
}
inline double PreprocessClass::Max(double  vara, double  varb)
{
	return vara >= varb ? vara : varb;
}
inline String PreprocessClass::valueAdd(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(varA.c_str()) + atof(varB.c_str());
	return String(c, decimalplace);
}
inline String PreprocessClass::valueSub(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(varA.c_str()) - atof(varB.c_str());
	return String(c, decimalplace);
}
inline String PreprocessClass::valueMul(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(varA.c_str()) * atof(varB.c_str());
	return String(c, decimalplace);
}
inline String PreprocessClass::valueDiv(String varA, String varB, uint8_t decimalplace = 6)
{
	double c = atof(varA.c_str()) / atof(varB.c_str());
	return String(c, decimalplace);
}
inline String PreprocessClass::DeepExpression(String& logic)
{
	int iO = -1, iC = -1;
	for (int i = 0; i < logic.length(); i++)
	{
		if (logic[i] == '(')
			iO = i;
		if (logic[i] == ')')
		{
			iC = i;
			break;
		}
	}
	String newLogic = logic.subString(iO + 1, iC - iO - 1);

	logic.replace("(" + newLogic + ")", Compare(newLogic));
	return logic;
}
inline String PreprocessClass::DeepOC(String logic)
{

	int iO = -1, iC = -1;
	for (int i = 0; i < logic.length(); i++)
	{
		if (logic[i] == '(')
			iO = i;
		if (logic[i] == ')')
		{
			iC = i;
			break;
		}
	}
	if (logic.indexOf(":") >= 0 || logic.indexOf(";") >= 0 || logic.indexOf("|") >= 0
		|| logic.indexOf("@") >= 0 || logic.indexOf("#") >= 0 || logic.indexOf("&") >= 0 || logic.indexOf("~") >= 0 || logic.indexOf("$") >= 0)
	{
		return logic;
	}
	String newLogic = logic.subString(iO + 1, iC - iO - 1);
	logic.replace("(" + newLogic + ")", Calculate(newLogic));
	return logic;
}
#pragma endregion

String PreprocessClass::Calculate(String logic)
{
	if (logic.indexOf(":") >= 0 || logic.indexOf(";") >= 0 || logic.indexOf("|") >= 0
		|| logic.indexOf("@") >= 0 || logic.indexOf("#") >= 0 || logic.indexOf("&") >= 0 || logic.indexOf("~") >= 0 || logic.indexOf("$") >= 0)
	{
		return logic;
	}
	bool mul = false, div = false;
	int ind = -1;
	String left = "", right = "";
	if (logic.indexOf('*') >= 0 || logic.indexOf('/') >= 0)
	{
		for (int i = 0; i < logic.length(); i++)
		{
			if (logic[i] == '*')
			{
				mul = true;
				ind = i;
				break;
			}
			if (logic[i] == '/')
			{
				div = true;
				ind = i;
				break;
			}
		}
		for (int i = ind - 1; i >= 0; i--)
		{
			if (logic[i] == '+' || logic[i] == '/' || logic[i] == '*' || logic[i] == '-') break;
			left = logic[i] + left;
		}
		for (int i = ind + 1; i < logic.length(); i++)
		{
			if ((logic[i] == '+' || logic[i] == '/' || logic[i] == '*' || logic[i] == '-') && right != "") break;
			right += logic[i];
		}
		if (mul)
		{
			String vleft = left;
			vleft.replace("?", "-");
			///Phân tách 2 nửa logic
			String pleft = logic.subString(0, ind - (left.length()));
			String pright = logic.substring(ind + (right.length() + 1));
			logic = pleft + valueMul(vleft, right) + pright;
			if (left.indexOf('?') >= 0 && !(logic.indexOf('+') >= 0 || logic.indexOf('-') > 0 || logic.indexOf('*') > 0 || logic.indexOf('/') > 0)) left = "";
		}
		if (div)
		{
			String vleft = left;
			vleft.replace("?", "-");
			///Phân tách 2 nửa logic
			String pleft = logic.subString(0, ind - (left.length()));
			String pright = logic.substring(ind + (right.length() + 1));
			logic = pleft + valueDiv(vleft, right) + pright;
			if (left.indexOf('?') >= 0 && !(logic.indexOf('+') >= 0 || logic.indexOf('-') > 0 || logic.indexOf('*') > 0 || logic.indexOf('/') > 0)) left = "";
		}
	}
	else// if (logic.indexOf('+') >= 0 || logic.indexOf('-') > 0)
	{
		for (int i = 0; i < logic.length(); i++)
		{
			if (logic[i] == '+')
			{
				mul = true;
				ind = i;
				break;
			}
			if (logic[i] == '-')
			{
				div = true;
				ind = i;
				break;
			}
		}
		for (int i = ind - 1; i >= 0; i--)
		{
			if (logic[i] == '+' || logic[i] == '/' || logic[i] == '*' || logic[i] == '-') break;
			left = logic[i] + left;
		}
		for (int i = ind + 1; i < logic.length(); i++)
		{
			if ((logic[i] == '+' || logic[i] == '/' || logic[i] == '*' || logic[i] == '-') && right != "") break;
			right += logic[i];
		}
		if (mul)
		{
			String vleft = left;
			vleft.replace("?", "-");
			///Phân tách 2 nửa logic
			String pleft = logic.subString(0, ind - (left.length()));
			String pright = logic.substring(ind + (right.length() + 1));
			logic = pleft + valueAdd(vleft, right) + pright;
			if (left.indexOf('?') >= 0 && !(logic.indexOf('+') >= 0 || logic.indexOf('-') > 0 || logic.indexOf('*') > 0 || logic.indexOf('/') > 0)) left = "";

		}
		if (div)
		{
			if (left != "")
			{
				String vleft = left;
				vleft.replace("?", "-");
				///Phân tách 2 nửa logic
				String pleft = logic.subString(0, ind - (left.length()));
				String pright = logic.substring(ind + (right.length() + 1));
				logic = pleft + valueSub(vleft, right) + pright;
				if (left.indexOf('?') >= 0 && !(logic.indexOf('+') >= 0 || logic.indexOf('-') > 0 || logic.indexOf('*') > 0 || logic.indexOf('/') > 0)) left = "";
			}
			else
			{
				if (logic.indexOf('+') >= 0 || (logic.indexOf('-') >= 0 && logic.lastIndexOf('-') > 0 && logic.indexOf('-') != logic.lastIndexOf('-')) || logic.indexOf('*') >= 0 || logic.indexOf('/') >= 0)
				{

					//logic = logic.Replace(left + "-" + right, "?" + (Convert.ToDouble(right)).ToString());
					String pleft = "";//logic.Substring(0, ind - (left.Length + 1));
					String pright = logic.substring(ind + (right.length() + 1));
					logic = pleft + "?" + String(right.toDouble()) + pright;
					left = "?";
				}
			}
		}
	}
	if (logic.indexOf("--") == 0) logic.replace("--", "");
	if (logic.indexOf("--") > 0) logic.replace("--", "+");
	if ((logic.indexOf('+') >= 0 || logic.indexOf('-') >= 0 || logic.indexOf('*') >= 0 || logic.indexOf('/') >= 0) && left != "")
	{
		return Calculate(logic);
	}
	if (logic.indexOf('-') != logic.lastIndexOf('-'))
		return Calculate(logic);
	return logic;
}

#pragma region Thay thế biến vào biểu thức và phương pháp tính

//((SoA>=3||SoB==6) && (SoA/SoB+2!=5))
String PreprocessClass::replaceExpression(String& logic)
{
	logic.replace(" ", "");
	double iG = logic.indexOf('>'), iL = logic.indexOf('<'), iI = logic.indexOf('!'), iE = logic.indexOf('=');
	double cA = logic.indexOf('&'), cO = logic.indexOf('|');
	double iO = logic.indexOf('('), iC = logic.indexOf(')');
	bool	less_zero = true;
	double first = Min(Min(cA, cO, false), Min(Min(iG, iL, false), Min(iI, iE, false), false), false);
	int i = 0;
	String newLogic = "";
	while (logic != "")
	{
		logic.trim();
		iG = logic.indexOf('>'); iL = logic.indexOf('<'); iI = logic.indexOf('!'); iE = logic.indexOf('=');
		cA = logic.indexOf('&'); cO = logic.indexOf('|');
		first = Min(Min(cA, cO, false), Min(Min(iG, iL, false), Min(iI, iE, false), false), false);
		String check_O = "";
		String check_I = "";
		if (first >= 0)
		{
			String vars = logic.subString(0, first);
			if (vars.indexOf('*') >= 0 || vars.indexOf('/') >= 0 || vars.indexOf('+') >= 0 || vars.indexOf('-') >= 0)
			{
				check_O = checkOpen(vars, "(", false);
				check_I = checkOpen(vars, ")", false);
				vars = GetValue(vars);
			}
			iO = vars.indexOf('('); iC = vars.indexOf(')');
			String strO = iO >= 0 ? vars.subString(0, vars.lastIndexOf("(") + 1) : "";
			String strC = iC >= 0 ? vars.subString(iC, vars.length() - iC) : "";
			vars.replace("(", "");
			vars.replace(")", "");

			String ope = logic.subString(first + 1, 1);
			if (ope == ">" || ope == "<" || ope == "=" || ope == "!" || ope == "&" || ope == "|")
			{
				ope = logic.subString(first, 1) + ope;
			}
			else ope = logic.subString(first, 1);
			newLogic += check_O + check_I + strO + GetValue(vars) + strC + ope;
			logic.remove(0, first + ope.length());
		}
		else
		{
			String vars = logic;
			iO = vars.indexOf('('); iC = vars.indexOf(')');
			String strO = iO >= 0 ? vars.subString(0, iO + 1) : "";
			String strC = iC >= 0 ? vars.subString(iC, vars.length() - iC) : "";

			vars.replace("(", "");
			vars.replace(")", "");
			newLogic += check_O + check_I + strO + GetValue(vars) + strC;
			logic = "";
		}
	}
	return newLogic;
}
String PreprocessClass::replaceVariables(String logic)
{
	double iP = logic.indexOf('+'), iS = logic.indexOf('-'), iM = logic.indexOf('*'), iD = logic.indexOf('/');
	double iO = logic.indexOf('('), iC = logic.indexOf(')');
	double first = Min(Min(iP, iS, false), Min(iD, iM, false), false);
	//memoryEx.Debug("replaceVariables-->opers: " + String(first), false);

	int i = 0;
	String newLogic = "";
	while (logic != "")
	{
		logic.trim();
		iP = logic.indexOf('+'); iS = logic.indexOf('-'); iM = logic.indexOf('*'); iD = logic.indexOf('/');
		first = Min(Min(iP, iS, false), Min(iD, iM, false), false);
		if (first >= 0)
		{
			bool hasSub = false;
			if (first == 0 && iS == 0) {
				first = Min(iP, Min(iD, iM, false), false);
				if (first == 0) hasSub = true;
				newLogic += logic;
				return newLogic;
				//first = logic.subString(first, )
			}
			String vars = logic.subString(0, first);
			if (hasSub)logic = "";
			iO = vars.indexOf('('); iC = vars.indexOf(')');
			String strO = iO >= 0 ? vars.subString(0, vars.lastIndexOf("(") + 1) : "";
			String strC = iC >= 0 ? vars.subString(iC, vars.length() - iC) : "";
			vars.replace("(", "");
			vars.replace(")", "");
			vars.trim();
			newLogic += strO + GetValue(vars) + strC + logic.subString(first, 1);
			if (!hasSub)
				logic.remove(0, first + 1);
			else
			{
				break;
			}
		}
		else
		{
			String vars = logic;
			iO = vars.indexOf('('); iC = vars.indexOf(')');
			String strO = iO >= 0 ? vars.subString(0, iO + 1) : "";
			String strC = iC >= 0 ? vars.subString(iC, vars.length() - iC) : "";

			vars.replace("(", "");
			vars.replace(")", "");
			vars.trim();
			newLogic += strO + GetValue(vars) + strC;
			logic = "";
		}
	}
	return newLogic;
}
#pragma endregion

String PreprocessClass::GetValue(String name)
{
	String comm = name.subString(0, name.indexOf("["));
	String val = CheckStaticFunction(name);
	if (Math.indexOf("," + comm + ",") >= 0)
		return val;
	else
		if (val != "" && memoryEx.isNumberic(val)) return val;
	val = *memoryEx.GetValue(name);
	if (val != "") return val.c_str();
	if (name == "") name = "";
	if (name.indexOf('+') >= 0 || name.indexOf('-') >= 0 || name.indexOf('/') >= 0 || name.indexOf('*') >= 0)
	{
		name = checkOpen(name, ")", true) + name;
		name = name + checkOpen(name, "(", true);
		return Execute(name);
	}
	return name;
}
String PreprocessClass::CheckStaticFunction(String name) {
	name.trim();
	//SEND WS PI;
	if (name == "PI") return String(PI, 20);
	//SEND WS POW[2,4];
	if (name.startsWith("POW["))
	{
		name.remove(0, String("POW[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String r = name.split(",", 1);
		String vl = Execute(l);
		String vr = Execute(r);
		return String(pow(atof(vl.c_str()), atof(vr.c_str())), 6);
	}
	//SEND WS INDEXOF['hun','Pham quang hung'];
	if (name.startsWith("INDEXOF["))
	{
		name.remove(0, String("INDEXOF[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = String(Execute(name.split(",", 0)));
		String m = name.split(",", 1);
		return String(l.indexOf(m));
	}
	if (name.startsWith("LASTINDEXOF["))
	{
		name.remove(0, String("LASTINDEXOF[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = String(Execute(name.split(",", 0)));
		String m = name.split(",", 1);
		return String(l.lastIndexOf(m));
	}
	if (name.startsWith("PADLEFT["))
	{
		name.remove(0, String("PADLEFT[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = String(Execute(name.split(",", 0)));
		String m = name.split(",", 1);
		String r = name.split(",", 2);
		r.replace("'", "");
		return l.padleftString(m.toInt(), r);
	}
	if (name.startsWith("PADRIGHT["))
	{
		name.remove(0, String("PADRIGHT[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String m = name.split(",", 1);
		String r = name.split(",", 2);
		return l.padrightString(m.toInt(), r);
	}
	if (name.startsWith("TRIM["))
	{
		name.remove(0, String("TRIM[").length());
		name.remove(name.lastIndexOf("]"), 1);
		if (name.indexOf("'") >= 0) {
			name.trim();
		}
		else
		{
			name.trim();
			name = String(Execute(name));
		}
		name.trim();
		return name;
	}
	if (name.startsWith("CALC["))
	{
		name.remove(0, String("CALC[").length());
		name.remove(name.lastIndexOf("]"), 1);
		name.trim();
		name = String(Execute(name));
		name.trim();
		return name;
	}
	if (name.startsWith("REPLACE["))
	{
		name.remove(0, String("REPLACE[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String m = name.split(",", 1);
		String r = name.split(",", 2);
		String vm = m;
		String rl = "";
		if (l.indexOf("'") >= 0) {
			l.replace("'", "");
		}
		if (vm.indexOf("'") >= 0) {
			vm = m;
			vm.replace("'", "");
		}
		else {
			vm = String(Execute(r));
		}
		if (r.indexOf("'") >= 0) {
			r.replace("'", "");
			l.replace(vm, r);
		}
		else {
			rl = Execute(r);
			l.replace(vm, String(rl));
		}
		return l;
	}
	if (name.startsWith("REMOVE["))
	{
		name.remove(0, String("REMOVE[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String m = name.split(",", 1);
		String r = name.split(",", 2);
		String vm = Execute(m);
		String vr = Execute(r);
		l.remove(vm.toInt(), vr.toInt());
		return l;
	}
	if (name.startsWith("FMAX["))
	{
		name.remove(0, String("FMAX[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String r = name.split(",", 1);
		String vl = Execute(l);
		String vr = Execute(r);
		return String(fmax(atof(vl.c_str()), atof(vr.c_str())), 6);
	}
	if (name.startsWith("FMIN["))
	{
		name.remove(0, String("FMIN[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String l = name.split(",", 0);
		String r = name.split(",", 1);
		String vl = Execute(l);
		String vr = Execute(r);
		return String(fmin(atof(vl.c_str()), atof(vr.c_str())), 6);
	}
	if (name.startsWith("ROUND["))
	{
		name.remove(0, String("ROUND[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String vl = Execute(name);
		return String(round(atof(vl.c_str())), 20);
	}
	if (name.startsWith("EXP(["))
	{
		name.remove(0, String("EXP[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String vl = Execute(name);
		return String(exp(atof(vl.c_str())), 20);
	}
	if (name.startsWith("LOG["))
	{
		name.remove(0, String("LOG[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(log(atof(vl.c_str())), 6);
	}
	if (name.startsWith("LOG10["))
	{
		name.remove(0, String("LOG10[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(log(atof(vl.c_str())), 6);
	}
	if (name.startsWith("ABS["))
	{
		name.remove(0, String("ABS[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(abs(atof(vl.c_str())), 6);
	}
	if (name.startsWith("SQRT["))
	{
		name.remove(0, String("SQRT[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(sqrt(atof(vl.c_str())), 6);
	}
	if (name.startsWith("CBRT["))
	{
		name.remove(0, String("CBRT[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(cbrt(atof(vl.c_str())), 6);
	}
	if (name.startsWith("CEIL["))
	{
		name.remove(0, String("CEIL[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(ceil(atof(vl.c_str())), 6);
	}
	if (name.startsWith("SIN["))
	{
		name.remove(0, String("SIN[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String vl = Execute(name);
		return String(sin(atof(vl.c_str())), 6);
	}
	if (name.startsWith("ASIN["))
	{
		name.remove(0, String("ASIN[").length());
		name.remove(name.lastIndexOf("]"), 1);
		String vl = Execute(name);
		return String(asin(atof(vl.c_str())), 6);
	}
	if (name.startsWith("ATAN["))
	{
		name.remove(0, String("ATAN[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(atan(atof(vl.c_str())), 6);
	}
	if (name.startsWith("COS["))
	{
		name.remove(0, String("COS[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(cos(atof(vl.c_str())), 6);
	}
	if (name.startsWith("ACOS["))
	{
		name.remove(0, String("ACOS[").length());
		name.remove(name.lastIndexOf("]"), 1);

		String vl = Execute(name);
		return String(acos(atof(vl.c_str())), 6);
	}
	return "";
}
String PreprocessClass::Execute(String logic)
{
	if (_onRun != NULL)  _onRun();
	String opers = replaceVariables(logic);
	if (opers[0] == '-')opers = "0" + opers;
	if (logic.indexOf(":") >= 0 || logic.indexOf(";") >= 0 || logic.indexOf("|") >= 0
		|| logic.indexOf("@") >= 0 || logic.indexOf("#") >= 0 || logic.indexOf("&") >= 0 || logic.indexOf("~") >= 0 || logic.indexOf("$") >= 0)
	{
	}
	else
		opers.replace(" ", "");
	//memoryEx.Debug("replaceVariables-->opers: " + opers, false);
	while (opers.indexOf('(') >= 0)
	{
		opers = DeepOC(opers);
	}
	//memoryEx.Debug("DeepOC-->opers: " + opers, false);
	String kq = "";
	kq = Calculate(opers);
	return kq;
}

void PreprocessClass::onRun(void(*callback)())
{
	_onRun = callback;
}

void PreprocessClass::onListen(void(*callback)())
{
}

bool PreprocessClass::ExecuteExpression(String logic)
{
	String kg = replaceExpression(logic);

	kg = DeepExpression(kg);
	while (kg.indexOf('(') >= 0 || kg.indexOf(')') >= 0)
	{
		kg = DeepExpression(kg);
	}
	while (kg.indexOf('>') >= 0 || kg.indexOf('<') >= 0 || kg.indexOf('=') >= 0 || kg.indexOf('!') >= 0)
	{
		kg = Compare(kg); ;
	}
	while (kg.indexOf('&') >= 0 || kg.indexOf('|') >= 0)
	{
		kg = CompareOperan(kg);
	}
	return kg == "1" ? true : false;
}
String PreprocessClass::CompareOperan(String logic)
{
	double iG = logic.indexOf('&'), iL = logic.indexOf('|');
	double  first = Min(iG, iL, false);
	String l = logic.subString(0, first);
	String ope = logic.subString(first, 2);
	String r = logic.subString(l.length() + ope.length(), 1);
	logic.remove(0, l.length() + ope.length() + r.length());
	if (ope == "&&") logic = l.toInt() == 1 && r.toInt() == 1 ? "1" + logic : "0" + logic;
	if (ope == "||") logic = l.toInt() == 1 || r.toInt() == 1 ? "1" + logic : "0" + logic;
	if (logic.indexOf('&') >= 0 || logic.indexOf('|') >= 0)
	{
		logic = CompareOperan(logic);
	}
	//Cần thực hiện các phép so sánh để trả về kết quả cuối cùng.
	return logic;
}
String PreprocessClass::Compare(String logic)
{
	//0&&1||1&&0
	if (logic.indexOf('<') < 0 && logic.indexOf('!') < 0 && logic.indexOf('=') < 0 && logic.indexOf('>') < 0)
	{
		return CompareOperan(logic);
	}
	double iG = logic.indexOf('>'), iL = logic.indexOf('<'), iI = logic.indexOf('!'), iE = logic.indexOf('=');
	double  first = Min(Min(iG, iL, false), Min(iI, iE, false), false);
	//-5>=9
	String l = logic.subString(0, first);
	String oran = "";
	String oranr = "";
	//0&&1==1&&2>3                 0||1>9      1>0&&1==1||3>2
	if (l.indexOf('&') >= 0)
	{
		oran = l.subString(0, l.lastIndexOf('&') + 1);
		l = l.substring(l.lastIndexOf('&') + 1);
	}
	if (l.indexOf('|') >= 0)
	{
		oran = l.subString(0, l.lastIndexOf('|') + 1);
		l = l.substring(l.lastIndexOf('|') + 1);
	}
	String ope = logic.subString(first + 1, 1);
	if (ope == ">" || ope == "<" || ope == "=" || ope == "!" || ope == "&" || ope == "|")
	{
		ope = logic.subString(first, 1) + ope;
	}
	else ope = logic.subString(first, 1);

	String r = logic.substring(oran.length() + l.length() + ope.length());
	String br = "";
	if (r.indexOf('&') >= 0 || r.indexOf('|') >= 0 || r.indexOf('=') >= 0 || r.indexOf('>') >= 0 || r.indexOf('<') >= 0 || r.indexOf('!') >= 0)
	{
		for (int i = 1; i < r.length(); i++)
		{
			if (!memoryEx.isNumberic(r.subString(0, i)))
			{
				br = r.subString(0, i - 1);
				break;
			}
		}
		oranr = r;
		oranr.remove(0, br.length());
	}
	else br = r;
	if (ope == ">") return oran + (atof(l.c_str()) > atof(br.c_str()) ? "1" : "0") + oranr;
	if (ope == "<") return oran + (atof(l.c_str()) < atof(br.c_str()) ? "1" : "0") + oranr;
	if (ope == ">=") return oran + (atof(l.c_str()) >= atof(br.c_str()) ? "1" : "0") + oranr;
	if (ope == "<=") return oran + (atof(l.c_str()) <= atof(br.c_str()) ? "1" : "0") + oranr;
	if (ope == "==") return oran + (atof(l.c_str()) == atof(br.c_str()) ? "1" : "0") + oranr;
	if (ope == "!=") return oran + (atof(l.c_str()) != atof(br.c_str()) ? "1" : "0") + oranr;
	return oran + "0" + oranr;
}

String PreprocessClass::checkOpen(String logic, String ck, bool reverse)
{
	int countO = 0;
	int countI = 0;
	for (int i = 0; i < logic.length(); i++)
	{
		if (logic[i] == '(') countO++;
		if (logic[i] == ')') countI++;
	}
	String c = "";
	if (ck == "(")
		if (countO - countI > 0)
		{
			for (int i = 0; i < countO - countI; i++)
			{
				c += (reverse ? ")" : "(");
			}
		}
	if (ck == ")")
		if (countI - countO > 0)
		{
			for (int i = 0; i < countI - countO; i++)
			{
				c += (reverse ? "(" : ")"); ;
			}
		}
	return c;
}

//bool PreprocessClass::If(String logic)
//{
//	return false;
//}
//
//bool PreprocessClass::While(String logic)
//{
//	return false;
//}
//
//bool PreprocessClass::For(String logic)
//{
//	return false;
//}

String PreprocessClass::Join(String logic)
{
	uint index = 0;
	String tmp = "";
	String newLogic = "";
	if (_onRun != NULL)
	{
		_onRun();
	}
	//'pham quang hung' + SoA + SoB + dateToday;
	while (logic.length() > 0)
	{
		index = logic.indexOf("+");
		if (index >= 0 && index < 429496) {
			tmp = logic.subString(0, index);
			while (tmp.indexOf("'") >= 0 && tmp.lastIndexOf("'") <= 0) {
				index = logic.indexOf("+", index + 1);
				tmp = logic.subString(0, index);
			}
			tmp.trim();//'pham quang hung' 
			logic.remove(0, index + 1);
			logic.trim();
		}
		else
		{
			tmp = logic;
		}
		if (tmp.indexOf("'") == 0 && tmp.lastIndexOf("'") > 0) {
			tmp.remove(tmp.lastIndexOf("'"), 1); tmp.remove(0, 1);
			newLogic += tmp;
			if (index < 0 || index >= 429496)
				logic = "";
		}
		else
		{
			if (index < 0 || index > 429496) {
				index = tmp.length();
				logic = "";
			}//gặp vấn đề ở đây với LCD nhé
			if (*memoryEx.GetValue(tmp) != "")
				newLogic += *memoryEx.GetValue(tmp);
			else
				newLogic += GetValue(tmp);
		}
	}
	return newLogic;
}
