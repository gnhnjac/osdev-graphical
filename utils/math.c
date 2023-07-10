int remap(int OldValue, int OldMin, int OldMax, int NewMin, int NewMax)
{

	return (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;

}

int max(int a, int b)
{

	if (a > b)
		return a;

	return b;

}

int min(int a, int b)
{

	if (a < b)
		return a;

	return b;

}
