#include "validate.h"

bool validate_bs(char *strike, char *volatility, char *under, char *risk, char *maturity)
{					


	if(strnlen(strike,16)>12)
	{
		DEBUG("input strike is very long");
		return false;
	}

	if(strnlen(volatility,16)>8)
	{
		DEBUG("input volatility is very long");
		return false;
	}


	if(strnlen(under,16)>12)
	{
		DEBUG("input under is very long");
		return false;
	}

	if(strnlen(risk,16)>4)
	{
		DEBUG("input risk is very long");
		return false;
	}


	if(strnlen(maturity,16)>4)
	{
		DEBUG("input maturity is very long");
		return false;
	}


	return true;
}


					
bool validate_compound(char *value,char *years,char *percent)
{					
	if(strnlen(value,16)>12)
	{
		DEBUG("input value is very long");
		return false;
	}



	if(strnlen(years,4)>3)
	{
		DEBUG("input years is very long");
		return false;
	}


	if(strnlen(percent,4)>3)
	{
		DEBUG("input percent is very long");
		return false;
	}

	return true;						
}
