#ifndef REF_DATA_ARRAY_H
#define REF_DATA_ARRAY_H

#include "ref_data_defines.hpp"
#include  <stdexcept>


class RefDataArray {

public:
	RefDataArray(int size);
	~RefDataArray();
	double get_by_index(int index);
	double get_by_wavelen(int wavelen);
	void append(double value);

private:
	double* data = nullptr;
	int size = 0;
	int curr_index = 0;
	bool is_valid_index(int index);
};
#endif // !REF_DATA_ARRAY_H
