/*********************************************************************************
*
*	Содержит простой анализатор сигналов, который проводит 
*		вычисление сразличных спектров и моментов сигнала.
*			*)	11.1.1.	Спектральные преобразования сигнала
*			*)	11.1.2.	Определение диапазона (по амплитуде) сигнала
*			*)	11.1.3.	Определение спектра сигнала
*			*)	11.1.4.	Определение релаксационных характеристик интегратора
*			*)	11.1.5.	Определение корректности формы калибровочного сигнала
*			*)	11.1.6.	Определение корректности частоты калибровочного сигнала
*			*)	11.1.7.	Определения параметров шума (как параметров шумового сигнала)
*			*)	11.1.8.	Определение соотношения сигнал/шум ?
*
*		Ничего не хранит - просто статический набор функий 
*
*		Решено, что, анализатор будет работать с классическими типами данных,
*		в виде vector<тип данных>*  чтобы совсем не замедлять работу 
*		преобразованиями из одного типа данных в другие
*
*********************************************************************************/


#include <MBN_SignalProcessor.h>
#include <vector>
#include <fftw3.h>
#include <assert.h>
#include <PagedArray.cpp>
#include <complex>

#define _USE_MATH_DEFINES

#include <math.h>

/// Вычисляет фурье-спектр сигнала data (массив float) в области [from_idx, to_idx]
/// ресайзит cos_out, sin_out и magnitude_out и записывает туда данные преобразования
/// в magnitude_out записываются результаты mag_out[i] = sqrt(sin_out[i]*sin_out[i] + cos_out[i]*cos_out[i]);
/// data остаётся нетронутым
/// если указатель на какой-то массив равен 0, то этот массив игнорируется
/// ГАРМОНИКИ ВЫХОДНЫХ СПКТРОВ: f(j) =  (-2 * PI * i) * j * k / N
/// где j - номер точки (ось абсцисс), k - номер гармоники
void MBN_SignalProcessor1D::CalculateFourierSpectrum(vector<float> &data, int from_idx, int to_idx, vector<float>* cos_out, vector<float>* sin_out, vector<float>* magnitude_out)
{
	// размер преобразования
		// размер исходного массива 
	int N = to_idx - from_idx + 1;
		// размер комплекстного массива коэффициентов 
		// он равен половине по размеру от исходного, т.к. кч имеет два независимых 
		// параметра, и, следовательно, суммарное число нехависимых параметров сохраняется
		// есть ещё объяснение: сигнал не может содержать информации о частотах с длиной волны
		// меньше, чем два шага решётки. 
	int outN = N/2 + 1;

	// подготавливаем выходные массивы
	if (cos_out) cos_out->resize(outN);
	if (sin_out) sin_out->resize(outN);
	if (magnitude_out) magnitude_out->resize(outN);

	// хранилище результатов преобразования
	fftwf_complex *out; 
	out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * outN);

	// план одномерного float преобразования (без пред-расчёта наиболее эффективного алгоритма)
	fftwf_plan transform = fftwf_plan_dft_r2c_1d(N, &(data[from_idx]), out, FFTW_ESTIMATE);

	// выполнить преобразование
	fftwf_execute(transform);

	// нормировочный коэффициент
	float norm_coeff = 1.0f/sqrt((float)N);

	// сохранить данные в выводные массивы
		// если есть все массивы
	if (cos_out && sin_out && magnitude_out)
	{
		for(int i = 0; i < outN; i++)
		{
			(*cos_out)[i] = out[i][0]*norm_coeff;
			(*sin_out)[i] = out[i][1]*norm_coeff;
			(*magnitude_out)[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1])*norm_coeff;
		};
	}
	else if (cos_out && sin_out)
	{
		for(int i = 0; i < outN; i++)
		{
			(*cos_out)[i] = out[i][0]*norm_coeff;
			(*sin_out)[i] = out[i][1]*norm_coeff;
		};
	}
	else if (cos_out && magnitude_out)
	{
		for(int i = 0; i < outN; i++)
		{
			(*cos_out)[i] = out[i][0]*norm_coeff;
			(*magnitude_out)[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1])*norm_coeff;
		};
	}
	else if (sin_out && magnitude_out)
	{
		for(int i = 0; i < outN; i++)
		{
			(*sin_out)[i] = out[i][1]*norm_coeff;
			(*magnitude_out)[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1])*norm_coeff;
		};
	}
	else if (sin_out) 
		for(int i = 0; i < outN; i++) (*sin_out)[i] = out[i][1]*norm_coeff;
	else if (cos_out) 
		for(int i = 0; i < outN; i++) (*cos_out)[i] = out[i][0]*norm_coeff;
	else if (magnitude_out)
		for(int i = 0; i < outN; i++) (*magnitude_out)[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1])*norm_coeff;	


	// удалить наши данные
	fftwf_destroy_plan(transform);
	fftwf_free(out);
};


/// Фильтрует сигнал data в диапазоне [from_idx, to_idx] фурье фильтром filter (filter[idx] - коэффициент на который умножается гармоника частоты idx.
/// гармоника: f(j) =  (-2 * PI * i) * j * k / N
/// при преобразовании)
/// результат записывается в out начиная с 0-индекса
/// filter иеет размер N/2 + 1, где N - размер data
void MBN_SignalProcessor1D::CalculateFilteredSignal(vector<float> &data, int from_idx, int to_idx, vector<float> &filter, vector<float> &result)
{
	// размер преобразования см. CalculateFourierSpectrum
	int N = to_idx - from_idx + 1;
	int outN = N/2 + 1;

	// подготавливаем выходные массивы
	result.resize(N);

	// хранилище результатов прямого преобразования
	fftwf_complex *image; 
	image = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * outN);

	// план одномерного float преобразования (без пред-расчёта наиболее эффективного алгоритма)
	fftwf_plan transform = fftwf_plan_dft_r2c_1d(N, &(data[from_idx]), image, FFTW_ESTIMATE);

	// выполнить преобразование
	fftwf_execute(transform);
	// тут в image хранится Фурье-образ сигнала

	// отфильтровать сигнал, помножаем коэффициенты фурье ряда на функцию фильтра (плюс, одновременно делаем нормировку на N)
	for (int i = 0; i < outN; i++)
	{
		image[i][0] *= filter[i] / (float)N;
		image[i][1] *= filter[i] / (float)N;
	}

	// преобразуем модифицированный ряд фурье обратно в сигнал
		// план обратного одномерного float преобразования (без пред-расчёта наиболее эффективного алгоритма)
	fftwf_plan rev_transform = fftwf_plan_dft_c2r_1d(N, image, &(result[from_idx]), FFTW_ESTIMATE);

	// выполнить обратное преобразование
	fftwf_execute(rev_transform);

	// удалить наши данные
	fftwf_destroy_plan(transform);
	fftwf_destroy_plan(rev_transform);
	fftwf_free(image);
};

/// Максимум сигнала data в области [from_idx, to_idx] и его положение.
/// Возвращает значение максимума, в max_idx записывает индекс первого максимума
float MBN_SignalProcessor1D::FindSignalMax(vector<float> &data, int from_idx, int to_idx, int *max_idx_ptr)
{
	assert(from_idx >= 0);
	assert(to_idx < (int)(data.size()));

	if (from_idx > to_idx)
	return 0.0;

	// максимальное значение
	float max_val = data[from_idx];
	// его индекс
	int max_index = from_idx;

	// ищем их
	for (int i = from_idx + 1; i <= to_idx; i++)
	{
		if (max_val < data[i])
		{
			max_val = data[i];
			max_index = i;
		};
	};

	if (max_idx_ptr)
		*max_idx_ptr = max_index;

	return max_val;
};

/// Минимум сигнала data в области [from_idx, to_idx] и его положение.
/// Возвращает значение минимума, в max_idx записывает индекс первого минимума
float MBN_SignalProcessor1D::FindSignalMin(vector<float> &data, int from_idx, int to_idx, int *min_idx_ptr)
{
	assert(from_idx >= 0);
	assert(to_idx < (int)(data.size()));

	if (from_idx > to_idx)
		return 0.0;

	// минимальное значение
	float min_val = data[from_idx];
	// его индекс
	int min_index = from_idx;

	// ищем их
	for (int i = from_idx + 1; i <= to_idx; i++)
	{
		if (min_val > data[i])
		{
			min_val = data[i];
			min_index = i;
		};
	};

	if (min_idx_ptr)
		*min_idx_ptr = min_index;

	return min_val;
};

/// Вычисляет границы диапазона сигнала data в области [from_idx, to_idx] и записывает их в low_bound и hi_bound
void MBN_SignalProcessor1D::CalculateSignalBounds(vector<float> &data, int from_idx, int to_idx, float *low_bound, float *hi_bound)
{
	assert(from_idx <= to_idx);
	assert(from_idx >= 0);
	assert(to_idx < data.size());
	assert(low_bound);
	assert(hi_bound);

	// минимальные и максимальные значения
	float min_val = data[from_idx];
	float max_val = data[from_idx];

	// ищем их
	for (int i = from_idx + 1; i <= to_idx; i++)
	{
		if (min_val > data[i])
			min_val = data[i];
		if (max_val < data[i])
			max_val = data[i];
	};

	// выводим результат
	*hi_bound = max_val;
	*low_bound = min_val;
};


/// Вычисляет среднее значение сигнала от from до to включительно
//template <class vector_type>
float MBN_SignalProcessor1D::CalculateAverage(vector<float> &data, int from, int to)
{
	assert(from <= to);
	assert(from >= 0);
	assert (to < data.size());

	float average = data[from];

	int N = to - from + 1;

	for (int i = from + 1; i <= to; i++)
		average += data[i];

	average /= (float)N;

	return average;
};

/// Вычисляет дисперсию сигнала
float MBN_SignalProcessor1D::CalculateDispersion(vector<float> &data, int from, int to)
{
	assert(from <= to);
	assert(from >= 0);
	assert (to < data.size());

	// среднее значение
	float average = CalculateAverage(data, from, to);

	float dispersion = (data[from] - average) * (data[from] - average);

	int N = to - from + 1;

	for (int i = from + 1; i <= to; i++)
		dispersion += (data[i] - average) * (data[i] - average);

	dispersion /= (float)N;

	return dispersion;
};

// возвращает достаточно ли линейна зависимость разности графика и функции 
// при параметрах f_params по параметру param_idx, который меняется от основного значения на +-range.
// Число точек проверки - points_count;
bool MBN_SignalProcessor1D::IsLinearAlongParameter(vector<float> &data, float a11, float c1, int param_idx, vector<float> &f_params, float (*f_ptr)(float arg, vector<float> &f_params), int points_count, float range)
{
	// значения разниц в точках
	vector<float> diffs; 
	diffs.resize(points_count);

	// текущие параметры
	vector<float> params;
	params.resize(f_params.size());
	for (int i = 0; i < params.size(); i++)
		params[i] = f_params[i];

	// вычисляем значение разностей в points_count точках вокруг f_params[param_idx] в диапазоне [f_params[param_idx] - range; f_params[param_idx] + range]
	for (int i = 0; i < points_count; i++)
	{
		params[param_idx] = (f_params[param_idx] - range) + 2.0f * range * (float)i / (float)points_count;
		diffs[i] = SquareDiff(data, params, a11, c1, f_ptr); 
	};

	// проверяем линейность

	// средний наклон
	float mean_bias = diffs[diffs.size() - 1] - diffs[0];

	// дисперсия
	float dispersion = 0.0;

	for (int i = 1; i < points_count; i++)
		dispersion += (mean_bias - (diffs[i] - diffs[i-1])) * (mean_bias - (diffs[i] - diffs[i-1]));

	// среднеквадратичное отклонение
	float st_deviation = sqrt(dispersion);

	// мера нелинейности
	float non_linearity = st_deviation/mean_bias;

	if (non_linearity < 0.01) return true;
	else return false;
};

// выполняет поиск оптимального шага по param_idx-ому параметру в точке параметров f_params для функции f_ptr
// data - график оригинала на который натягиваем функцию
float MBN_SignalProcessor1D::GetParamStep(vector<float> &data, float a11, float c1, int param_idx, vector<float> &f_params, float (*f_ptr)(float arg, vector<float> &f_params))
{
	// макс кол-во итераций
	int max_iter = 20;

	// начальный шаг параметра
	float range = 1.0;
	// на сколько увеличиваем-уменьшаем диапазон за один прогон ( > 1.0)
	float rate = 1.2;
	// флаг останова
	bool flag = true;

	// если при выделенном диапазоне зависимость разности от параметра нелинейна,
	// то будем уменьшать шаг пока не начнётся область линейности
	if (!IsLinearAlongParameter(data, a11, c1, param_idx, f_params, f_ptr, 10, range))
	{
		rate = 1.0/rate;
		flag = false;
	};

	// число итераций
	int iter = 0;

	// изменяем пока можно диапазон, подгоняем шаг
	while (IsLinearAlongParameter(data, a11, c1, param_idx, f_params, f_ptr, 10, range) == flag)
	{
		range *= rate;
			
		iter++;
		if (iter > max_iter) break;
	};

	// если мы расширяли область и вышли на область нелинейности - делаем один шаг назад.
	if (flag == false)
		range /= rate;

	return range;
};

// вычисляет сумму квадратов разности указанной функции f_ptr с параметрами f_params и представленного графика data
// в точках [0; data.size() - 1]
float MBN_SignalProcessor1D::SquareDiff(vector<float> &data, vector<float> &f_params, float a11, float c1, float (*f_ptr)(float arg, vector<float> &f_params))
{
	assert(f_ptr);

	// сумма квадратов отклонений
	float sdiff = 0.0;

	// вычисляем её
	for (int i = 0; i < data.size(); i++)
	{
		float diff = (f_ptr(((float)i) * a11 + c1, f_params)) - (data[i]);
		sdiff += diff*diff;
	}

	return sdiff;
};


// натягивает параметрическую функцию на предоставленный график с минимизацией по сумме квадратов отклонения от оригинала
// Итерация ведётся методом градиентного спуска в многопараметрическом пр-ве,
// как мера спуска используется сумма квадратов отклонения функции от оригинала
// предел: f(i*a11 + c11) = data(i)
//		f_params - вектор параметров функции
//		flush_params - сбрасывать параметры f_params в 0, или стартовать с тем что передали
//		data - оригинал сигнала
//		max_iterations_count - максимальное число итераций процесса
//
//		f_ptr - указатель на функцию f(i, params), которая принимает аргумент i и вектор параметров
//		first_adjust_fptr - опциональный указатель на функцию, которая инифиализирует стартовые параметры
//	
//		a11, c1 - преобразование индекса массива в координатное пространство,
//		т.е. функция f(i * a11 + c1) приближается к data(i)
//		a11 != 0;
//
//		возвращает - успешность выполнения аппроксимации
bool MBN_SignalProcessor1D::FitFunc(vector<float> &data, int max_iterations_count, vector<float> &f_params, bool flush_params, float a11, float c1, float (*f_ptr)(float arg, vector<float> &f_params), void (*first_adjust_fptr)(vector<float> &data, vector<float> &f_params, float a11, float c1))
{
	assert(f_ptr != 0);
	assert(max_iterations_count > 0);
	assert(data.size() > 0);

	// получаем min и max переданных данных
	float fmin = data[0];
	float fmax = data[0];

	for (int i = 0; i < data.size(); i++)
	{
		if (fmin > data[i]) fmin = data[i];
		if (fmax < data[i]) fmax = data[i];
	};

	// характерная величина разброса значений данных по Y
	float characteristic_Y_scale = fmax - fmin;

	// характерная величина для суммы квадратов разности функции и данных
	float characteristic_sdiff = characteristic_Y_scale * characteristic_Y_scale * ((float)data.size());

	// если указана инициализирующая функция, вызываем её для получения стартовых параметров,
	// иначе - стартуем с нулевым вектором параметров
	if (first_adjust_fptr)
		(*first_adjust_fptr)(data, f_params, a11, c1);
	else 
		// если надо обнулить параметры
		if (flush_params)
			for (int i = 0; i < f_params.size(); i++)
				f_params[i] = 0.0;

	// так, параметры инициализировали,
	// теперь организуем градиентный спуск	

	// вектор изменения параметров: d_params = {d diff/d p1; d diff/d p2; ...} * koeff, для получения параметров следующего шага
	vector<float> d_params;
	d_params.resize(f_params.size(), 0.0);

	// вектор параметров с одним изменённым параметром
	vector<float> one_modifyed_param;
	one_modifyed_param.resize(f_params.size());	

	// начинаем градиентный спуск

	// доп критерий остановки (относительное изменение diff за последний шаг) - если меньше - то выходим
	float epsilon = 0.1;

	// текущая мера разности графика и функции
	float curr_diff = SquareDiff(data, f_params, a11, c1, f_ptr);
	
	// предыдущее значение разности (выбирается так, чтобы условие первого входа в цикл всегда выполнялось
	float prev_diff = curr_diff - 0.5 * abs(curr_diff * epsilon);

	// счётчик итераций
	int iters = 0;

	// пока разность разностей убывает достаточно быстро или величина разности велика - работаем
	while ((abs(prev_diff - curr_diff) < epsilon * abs(curr_diff)) || (abs(curr_diff) > epsilon * abs(characteristic_sdiff)))
	{
		// обновляем массив для изменённого параметра
		for (int i = 0; i < f_params.size(); i++)
			one_modifyed_param[i] = f_params[i];

		// вычисляем направление градиента (с минусом, т.к. идём против градиента)
		for (int i = 0; i < f_params.size(); i++)
		{
			// вычисляем шаг изменения текущего параметра
			float dp = GetParamStep(data, a11, c1, i, f_params, f_ptr);

			// изменяем временный вектор-параметр
			one_modifyed_param[i] += dp;

			// вычисляем компоненту градиента
			d_params[i] = - (SquareDiff(data, one_modifyed_param, a11, c1, f_ptr) - curr_diff) / dp;

			// восстанавливаем вектор-параметр
			one_modifyed_param[i] -= dp;
		};

		// обновляем параметры функции для следующего шага
		for (int i = 0; i < f_params.size(); i++)
			f_params[i] += d_params[i];

		// обновляем число сделанных итераций
		iters++;

		// если число итераций больше максимального - не получилось натянуть функцию
		if (iters > max_iterations_count)
			return false;
	};

	return true;
};

// функция: a*sin(k*arg + phase) + c;
// параметры f_params синуса
// [0] = a
// [1] = k
// [2] = phase
// [3] = c
float MBN_SignalProcessor1D::SineFuncToFit(float arg, vector<float> &f_params)
{
	return f_params[0]*sin(f_params[1]*arg + f_params[2]) + f_params[3];
};

// натягивает синус (со смещением по Y) на предоставленный график с минимизацией по сумме квадратов отклонения
// натянутого синуса от графика
// Параметры синуса:   f(x) = a*sin(k*x + phase) + c;
// Итерация ведётся методом градиентного спуска в многопараметрическом пр-ве,
// как мера спуска используется сумма квадратов отклонения функции от оригинала
// Идеальное приближение:  f(i*a11 + c1) = data(i)
//		a11, c1 - преобразование индекса массива в координатное пространство,
//		т.е. функция f(i * a11 + c1) приближается к data(i)
//		возвращает - успешность выполнения аппроксимации
bool MBN_SignalProcessor1D::FitSine(vector<float> &data, float a11, float c1, int max_iterations_count, float* a_out, float* k_out, float* c_out, float *phase_out)
{
	// параметры синуса
	// [0] = a
	// [1] = k
	// [2] = phase
	// [3] = c
	
	vector<float> params;
	params.resize(4);

	// рассчёт стартовых параметров
		// с (смещение) - примерно равно среднему по массиву
	params[3] = 0.0;
	for (int i = 0; i < data.size(); i++)
		params[3] += data[i];
	params[3] /= (float)data.size();
		// фаза = 0
	params[2] = 0.0;
		// волновое число (или круговая частота)
			// смотрим, сколько раз прямая y = c пересекает data
	int cross_count = 0;
	for (int i = 0; i < data.size() - 1; i++)
		if ((data[i] - params[3]) * (data[i + 1] - params[3]) < 0.0)
			cross_count++;
			// по количеству пересечений определяем возможную частоту
	params[1] = (2.0f * M_PI * 0.5f * (float)cross_count) / (data.size() * a11);
		// амплитуда
			// минимум данных
	float dmin = data[0];
			// максимум данных
	float dmax = data[0]; 
	CalculateSignalBounds(data, 0, data.size() - 1, &dmin, &dmax);
	params[0] = 0.5f * (dmax - dmin);
	
	// фит
	if (FitFunc(data, max_iterations_count, params, false, a11, c1, &(MBN_SignalProcessor1D::SineFuncToFit), 0))
	{
		if (a_out) *a_out = params[0];
		if (k_out) *k_out = params[1];
		if (c_out) *c_out = params[2];
		if (phase_out) *phase_out = params[3];

		return true;
	}
	else return false;
};

// вычисление нормированной корреляционной функции набора данных
void MBN_SignalProcessor1D::CalcAutoCorrelationFunc(vector<float> &data, vector<float> &out)
{
	CalcCrossCorrelationFunc(data, data, out);
};

// вычисление корреляционной функции между наборами данных
void MBN_SignalProcessor1D::CalcCrossCorrelationFunc(vector<float> &data1, vector<float> &data2, vector<float> &out)
{
	// для вычисляния корр.ф. надо, чтобы среднее от функции было = 0,
	// поэтому мы используем значения функции смещённые на среднее значение
	// Данные в случае выхода за границы друг друга сичтаются равными 0.
	// т.е. идёт экстраполяция сигналов нулями

	// ресайзим выходной массив
	out.resize(max(data1.size(), data2.size()));

	// размер массива
	int L = max(data1.size(), data2.size());

	/// указатели на самый длинный и самый короткий массив
	vector<float>* max_len_data = 0;
	vector<float>* min_len_data = 0;
	if (data1.size() > data2.size())
	{
		max_len_data = &data1;
		min_len_data = &data2;
	}
	else // случай равенства массивов тоже сюда входит
	{
		max_len_data = &data2;
		min_len_data = &data1;
	}
	
	// вычисляем среднее по массиву данных, будем использовать data[i]-<data[i]> чтобы мат ожидание было равно 0
	float mean_val_max = MBN_SignalProcessor1D::CalculateAverage(*max_len_data, 0, max_len_data->size() - 1);
	float mean_val_min = MBN_SignalProcessor1D::CalculateAverage(*min_len_data, 0, min_len_data->size() - 1);

	// вычисляем сначала ненормированную корр функцию
	// по всем корреляционным дистанциям (i - корр смещение)
	for (int i = 0; i < L; i++)
	{
		// значение корр функции 
		float corr_func_val = 0.0;
		// максимальное значение - для нормировки
		float corr_func_val_max1 = 0.0;
		float corr_func_val_max2 = 0.0;

		// вычисляем значение корр ф-ции   INT[0; min(Lmax - i - 1, Lmin - 1)]((f1(j) - <f1>)(f2(j + i) - <f2>))
		for (int j = 0; j < min((int)(L - i), (int)(min_len_data->size())); j++)
		{
			corr_func_val += ((*min_len_data)[j] - mean_val_min)*((*max_len_data)[j + i] - mean_val_max);
			corr_func_val_max1 += ((*max_len_data)[j + i] - mean_val_max) * ((*max_len_data)[j + i] - mean_val_max);
			corr_func_val_max2 += ((*min_len_data)[j] - mean_val_min) * ((*min_len_data)[j] - mean_val_min);
		};

		// сохраняем значение
		out[i] = corr_func_val/ sqrt(corr_func_val_max1 * corr_func_val_max2);
	};

	// всё.
};

