/*************************************************************************
*
*	Содержит простой одномерный анализатор сигналов, который проводит 
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
*************************************************************************/

#ifndef MBN_SIGNAL_PROCESSOR
#define MBN_SIGNAL_PROCESSOR

#include <vector>
//#include <DataProviders.h>
#include <PagedArray.cpp>

using namespace std;



class MBN_SignalProcessor1D
{
public:
	
	/// Вычисляет фурье-спектр сигнала data в указанной им области
	/// cos_out, sin_out и magnitude_out ресайзит до нужного размера
	/// в cos_out, sin_out и magnitude_out записывает данные преобразования
	/// в magnitude_out записываются результаты mag_out[i] = sqrt(sin_out[i]*sin_out[i] + cos_out[i]*cos_out[i]);
	/// data остаётся нетронутым
	/// если указатель на какой-то массив равен 0, то этот массив игнорируется
	static void CalculateFourierSpectrum(vector<float> &data, int from_idx, int to_idx, vector<float>* cos_out, vector<float>* sin_out, vector<float>* magnitude_out);

	/// Фильтрует сигнал data в диапазоне [from_idx, to_idx] фурье фильтром filter (filter[idx] - коэффициент на который умножается гармоника частоты idx
	/// при преобразовании)
	/// результат записывается в out начиная с 0-индекса
	static void CalculateFilteredSignal(vector<float> &data, int from_idx, int to_idx, vector<float> &filter, vector<float> &result);

	/// Вычисляет границы диапазона сигнала data в области [from_idx, to_idx] и записывает их в low_bound и hi_bound
	static void CalculateSignalBounds(vector<float> &data, int from_idx, int to_idx, float *low_bound, float *hi_bound);

	
/*
	/// Вычисляет параметр релаксации для указанного места сигнала
	static float CalculateRelaxationParameter(vector<float> &data, int from_idx, int to_idx);

	/// Вычисляет коэффициент похожести сигнала data в области [from_idx, to_idx] на прямоугольный 
	/// Коэффициент похожести [0.0; 1.0]
	static float CalculateSimilarityToRectagularSignal(vector<float> &data, int from_idx, int to_idx, float amplitude, float period);

	/// Вычисляет период калибровочного сигнала data в области [from_idx, to_idx].
	static float CalculatePeriodOfSignal(vector<float> &data, int from_idx, int to_idx);*/

	
	/// Вычисляет среднее значение сигнала от from до to
	//template <class vector_type>
	static float CalculateAverage(vector<float> &data, int from, int to);

	/// Вычисляет дисперсию на заданном отрезке
	static float CalculateDispersion(vector<float> &data, int from, int to);

	/// Максимум сигнала data в области [from_idx, to_idx] и его положение.
	/// Возвращает значение максимума, в max_idx записывает индекс первого максимума
	static float FindSignalMax(vector<float> &data, int from_idx, int to_idx, int *max_idx_ptr);

	/// Минимум сигнала data в области [from_idx, to_idx] и его положение.
	/// Возвращает значение минимума, в max_idx записывает индекс первого минимума
	static float FindSignalMin(vector<float> &data, int from_idx, int to_idx, int *min_idx_ptr);

	// возвращает достаточно ли линейна зависимость разности графика и функции 
	// при параметрах f_params по параметру param_idx, который меняется от основного значения на +-range.
	// Число точек проверки - points_count;
	static bool IsLinearAlongParameter(vector<float> &data, float a11, float c1, int param_idx, vector<float> &f_params, float (*f_ptr)(float arg, vector<float> &f_params), int points_count, float range);

	// выполняет поиск оптимального шага по param_idx-ому параметру в точке параметров f_params для функции f_ptr
	static float GetParamStep(vector<float> &data, float a11, float c1, int param_idx, vector<float> &f_params, float (*f_ptr)(float arg, vector<float> &f_params));

	// вычисляет сумму квадратов разности указанной функции f_ptr с параметрами f_params и представленного графика data
	// в точках [0; data.size() - 1]
	static float SquareDiff(vector<float> &data, vector<float> &f_params, float a11, float c1, float (*f_ptr)(float arg, vector<float> &f_params));

	// натягивает параметрическую функцию на предоставленный график с минимизацией по сумме квадратов отклонения от оригинала
	// Итерация ведётся методом градиентного спуска в многопараметрическом пр-ве,
	// как мера спуска используется сумма квадратов отклонения функции от оригинала
	// предел: f(i*a11 + c11) = data(i)
	//		f_params - вектор параметров функции
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
	static bool FitFunc(vector<float> &data, int max_iterations_count, vector<float> &f_params, bool flush_params, float a11, float c1, float (*f_ptr)(float arg, vector<float> &f_params), void (*first_adjust_fptr)(vector<float> &data, vector<float> &f_params, float a11, float c1));


	// функция: a*sin(k*arg + phase) + c;
	// параметры f_params синуса
	// [0] = a
	// [1] = k
	// [2] = phase
	// [3] = c
    static float SineFuncToFit(float arg, vector<float> &f_params);

	// натягивает синус (со смещением по Y) на предоставленный график с минимизацией по сумме квадратов отклонения
	// натянутого синуса от графика
	// Параметры синуса:   f(i) = a*sin(k*i) + c;
	// Итерация ведётся методом градиентного спуска в многопараметрическом пр-ве,
	// как мера спуска используется сумма квадратов отклонения функции от оригинала
	//		возвращает - успешность выполнения аппроксимации
	static bool FitSine(vector<float> &data, float a11, float c1, int max_iterations_count, float* a_out, float* k_out, float* c_out, float *phase_out);

	// вычисление само-корреляционной функции набора данных
	static void CalcAutoCorrelationFunc(vector<float> &data, vector<float> &out);

	// вычисление корреляционной функции между наборами данных
	static void CalcCrossCorrelationFunc(vector<float> &data1, vector<float> &data2, vector<float> &out);
};


#endif
