int remap(int OldValue, int OldMin, int OldMax, int NewMin, int NewMax)
{

	return (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;

}

int max(int a, int b)
{

	return (a > b) ? a : b;

}

int min(int a, int b)
{

	return (a < b) ? a : b;

}

int abs(int n)
{

    return (n < 0) ? -n : n;

}

float root(float number)
{

	if (number < 0)
		return 0;

    int start = 0, end = number;
    int mid;
 
    // To store the answer
    float ans;
 
    // To find integral part of square
    // root of number
    while (start <= end) {
 
        // Find mid
        mid = (start + end) / 2;
 
        // If number is perfect square
        // then break
        if (mid * mid == number) {
            ans = mid;
            break;
        }
 
        // Increment start if integral
        // part lies on right side
        // of the mid
        if (mid * mid < number) {
          //first start value should be added to answer
            ans=start;
          //then start should be changed
            start = mid + 1;
        }
 
        // Decrement end if integral part
        // lies on the left side of the mid
        else {
            end = mid - 1;
        }
    }
 
    // To find the fractional part
    // of square root upto 5 decimal
    float increment = 0.1;
    for (int i = 0; i < 5; i++) {
        while (ans * ans <= number) {
            ans += increment;
        }
 
        // Loop terminates,
        // when ans * ans > number
        ans = ans - increment;
        increment = increment / 10;
    }
    return ans;
}