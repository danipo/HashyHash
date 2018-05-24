// Sean Szumlanski
// COP 3502, Fall 2016

// =======================
// HashyHash: testcase06.c
// =======================
// Boundary check to make sure your difficultyRating() and hoursSpent()
// functions are implemented correctly.


#include "HashyHash.h"
#include <stdio.h>

int main(void)
{
	int success = 1;

	if (difficultyRating() < 1.0 || difficultyRating() > 5.0)
		success = 0;
	if (hoursSpent() <= 0.0)
		success = 0;

	printf("%s\n", success ? "Everything seems to be unfolding according to plan..." : "fail whale :(");

	return 0;
}
