// to compile, use the following command:
// $(CXX) -Wall -O2 -std=c++11 -L. -I. -I../include -Isat/inc -o example example.cc -lhitman

#include "hit_minimal.hh"
#include "hit_minimum.hh"

// using HitMinimal
//=============================================================================
void example1()
{
	HitMinimal q;
	q.set_param_dcall();

	int arr1[] = { 1, 2, 3 };
	vector<int> set1(arr1, arr1 + sizeof(arr1) / sizeof(int));
	q.hit(set1);

	int arr2[] = { 1, 4 };
	vector<int> set2(arr2, arr2 + sizeof(arr2) / sizeof(int));
	q.hit(set2);

	int arr3[] = { 5, 6, 7 };
	vector<int> set3(arr3, arr3 + sizeof(arr3) / sizeof(int));
	q.hit(set3);

	while (true) {
		vector<int> mhs = q.get();

		if (mhs.empty())
			break;

		cout << "example1 mhs:";
		for (size_t i = 0; i < mhs.size(); ++i)
			cout << " " << mhs[i];
		cout << endl;

		q.block(mhs);
	};
}

// using HitMinimum
//=============================================================================
void example2()
{
	HitMinimum q;
	q.set_param_1call();  // do at most one SAT call per iteration
	q.set_param_trim(5);  // trim unsatisfiable cores at most 5 times

	int arr1[] = { 1, 2, 3 };
	vector<int> set1(arr1, arr1 + sizeof(arr1) / sizeof(int));
	q.hit(set1);

	int arr2[] = { 1, 4 };
	vector<int> set2(arr2, arr2 + sizeof(arr2) / sizeof(int));
	q.hit(set2);

	int arr3[] = { 5, 6, 7 };
	vector<int> set3(arr3, arr3 + sizeof(arr3) / sizeof(int));
	q.hit(set3);

	int hard[] = { -1, 4 };
	vector<int> cl(hard, hard + sizeof(hard) / sizeof(int));
	q.add_hard(cl);

	while (true) {
		vector<int> mhs = q.enumerate();

		if (mhs.empty())
			break;

		cout << "example2 mhs:";
		for (size_t i = 0; i < mhs.size(); ++i)
			cout << " " << mhs[i];
		cout << endl;
	};
}

// using HitMinimum (weighted)
//=============================================================================
void example3()
{
	HitMinimum q;
	q.set_param_1call();  // do at most one SAT call per iteration
	q.set_param_trim(5);  // trim unsatisfiable cores at most 5 times

	int arr1[] = { 1, 2, 3 };
	vector<int> set1(arr1, arr1 + sizeof(arr1) / sizeof(int));
	q.hit(set1);

	int arr2[] = { 1, 4 };
	vector<int> set2(arr2, arr2 + sizeof(arr2) / sizeof(int));
	q.hit(set2);

	int arr3[] = { 5, 6, 7 };
	vector<int> set3(arr3, arr3 + sizeof(arr3) / sizeof(int));
	q.hit(set3);

	// consider non-unit weights for elements 1, 2, and 7
	map<int, long> wghts; wghts[1] = 5; wghts[2] = 3; wghts[7] = 4;
	q.weigh(wghts);

	while (true) {
		vector<int> mhs = q.enumerate();

		if (mhs.empty())
			break;

		cout << "example3 mhs:";
		for (size_t i = 0; i < mhs.size(); ++i)
			cout << " " << mhs[i];
		cout << " (cost: " << q.cost() << ")" << endl;
	};
}

// using HitMinimum
//=============================================================================
void example4()
{
	HitMinimum q;

	int arr1[] = { 1, 2, 3, 4, 5, 7, 10, 9, 8 };
	vector<int> set1(arr1, arr1 + sizeof(arr1) / sizeof(int));
	q.hit(set1);

	int arr2[] ={ 1, 2, 3, 4, 5, 7, 10, 6, 9 } ;
	vector<int> set2(arr2, arr2 + sizeof(arr2) / sizeof(int));
	q.hit(set2);

	int arr3[] = { 9, 2, 3, 4, 5, 7, 10, 6, 8 };
	vector<int> set3(arr3, arr3 + sizeof(arr3) / sizeof(int));
	q.hit(set3);

	int arr4[] = { 1, 3, 4, 5, 9, 7, 10, 6, 8 };
	vector<int> set4(arr4, arr4 + sizeof(arr4) / sizeof(int));
	q.hit(set4);

	int arr5[] = { 1, 2, 3, 4, 5, 9, 7, 6, 8 };
	vector<int> set5(arr5, arr5 + sizeof(arr5) / sizeof(int));
	q.hit(set5);

	int arr6[] = { 1, 2, 3, 4, 5, 7, 10, 6, 8 };
	vector<int> set6(arr6, arr6 + sizeof(arr6) / sizeof(int));
	q.hit(set6);

	int arr7[] = { 1, 2, 3, 4, 9, 7, 10, 6, 8 };
	vector<int> set7(arr7, arr7 + sizeof(arr7) / sizeof(int));
	q.hit(set7);

	int arr8[] = { 1, 2, 9, 4, 5, 7, 10, 6, 8 };
	vector<int> set8(arr8, arr8 + sizeof(arr8) / sizeof(int));
	q.hit(set8);

	int arr9[] = { 1, 2, 3, 5, 9, 7, 10, 6, 8 };
	vector<int> set9(arr9, arr9 + sizeof(arr9) / sizeof(int));
	q.hit(set9);

	int arr10[] = { 1, 2, 3, 4, 5, 9, 10, 6, 8 };
	vector<int> set10(arr10, arr10 + sizeof(arr10) / sizeof(int));
	q.hit(set10);

	while (true) {
		vector<int> mhs = q.enumerate();

		if (mhs.empty())
			break;

		cout << "example4 mhs:";
		for (size_t i = 0; i < mhs.size(); ++i)
			cout << " " << mhs[i];
		cout << endl;
	};
}

//
//=============================================================================
int main(int argc, char *argv[])
{
	example1();  // HitMinimal
	example2();  // HitMinimum (simple)
	example3();  // HitMinimum (weighted)
	example4();  // HitMinimum (larger)

	return 0;
}
