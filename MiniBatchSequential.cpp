#include <iostream>
#include "bits/stdc++.h"
#include <random>
#include <math.h>
#include <algorithm>
#include <omp.h>

using namespace std;

#define RAND_FLOAT() (((float)rand()) / ((float)RAND_MAX))
#define inputN 784      // Number of neurons in input layer
#define hN 50           // Number of neurons in hidden layer
#define outN 10         // Number of neurons in output layer
#define num 50000       // Number of Training Images to Read
#define test_num 1000   // Number of Testing Images to Read
#define MINIBATCH 50000 // Must divide into SAMPLE_SIZE evenly
#define epochs 10       // No of Epochs
#define eta 0.5         // Learning Rate

//training set
struct
{
    double input[inputN];
    double label[outN];
} data[num];

//testing set
struct
{
    double input[inputN];
    double label[1];
} test[test_num];

vector<vector<double>> images;
vector<int> labels;
vector<vector<double>> test_images;
vector<int> test_labels;

int to_int(char *p)
{
    return ((p[0] & 0xff) << 24) | ((p[1] & 0xff) << 16) | ((p[2] & 0xff) << 8) | ((p[3] & 0xff) << 0);
}

/*------------------------------------------------------------------------------------------*/
void fileload()
{
    string IMAGE_FILE_NAME = "train-images.idx3-ubyte";     // Training Images Data File
    string LABEL_FILE_NAME = "train-labels.idx1-ubyte";     // Training Labels Data File
    string TEST_IMAGE_FILE_NAME = "t10k-images.idx3-ubyte"; // Testing Images Data File
    string TEST_LABEL_FILE_NAME = "t10k-labels.idx1-ubyte"; // Testing Labels Data File

    // Loading Training Images
    int m_size, m_rows, m_cols;
    ifstream ifs(IMAGE_FILE_NAME.c_str(), ios::in | ios::binary);
    char p[4];
    ifs.read(p, 4);
    int magic_number = to_int(p);
    assert(magic_number == 0x803);
    ifs.read(p, 4);
    m_size = to_int(p);
    if (num != 0 && num < m_size)
        m_size = num;
    ifs.read(p, 4);
    m_rows = to_int(p);
    ifs.read(p, 4);
    m_cols = to_int(p);
    char *q = new char[m_rows * m_cols];
    for (int i = 0; i < m_size; i++)
    {
        ifs.read(q, m_rows * m_cols);
        vector<double> image(m_rows * m_cols);
        int val = m_rows * m_cols;
        for (int j = 0; j < val; j++)
            image[j] = q[j] / 255.0;
        images.push_back(image);
    }
    delete[] q;
    ifs.close();

    // Loading Training Labels
    ifstream ifs1(LABEL_FILE_NAME.c_str(), ios::in | ios::binary);
    ifs1.read(p, 4);
    magic_number = to_int(p);
    assert(magic_number == 0x801);
    ifs1.read(p, 4);
    int size = to_int(p);
    if (num != 0 && num < size)
        size = num;
    for (int i = 0; i < size; i++)
    {
        ifs1.read(p, 1);
        int label = p[0];
        labels.push_back(label);
    }
    ifs1.close();

    // Loading Test Images
    int mt_size, mt_rows, mt_cols;
    ifstream ifs2(TEST_IMAGE_FILE_NAME.c_str(), ios::in | ios::binary);
    ifs2.read(p, 4);
    magic_number = to_int(p);
    ifs2.read(p, 4);
    mt_size = to_int(p);
    if (test_num < mt_size)
        mt_size = test_num;
    ifs2.read(p, 4);
    mt_rows = to_int(p);
    ifs2.read(p, 4);
    mt_cols = to_int(p);
    char *q1 = new char[mt_rows * mt_cols];
    for (int i = 0; i < mt_size; i++)
    {
        ifs2.read(q1, mt_rows * mt_cols);
        vector<double> image(mt_rows * mt_cols);
        int val = mt_rows * mt_cols;
        for (int j = 0; j < val; j++)
        {
            image[j] = q1[j] / 255.0;
        }
        test_images.push_back(image);
    }
    delete[] q1;
    ifs2.close();

    //Loading Test Labels
    ifstream ifs3(TEST_LABEL_FILE_NAME.c_str(), ios::in | ios::binary);
    ifs3.read(p, 4);
    magic_number = to_int(p);
    ifs3.read(p, 4);
    int test_size = to_int(p);
    if (test_num != 0 && test_num < test_size)
        test_size = test_num;
    for (int i = 0; i < test_size; i++)
    {
        ifs3.read(p, 1);
        int label = p[0];
        test_labels.push_back(label);
    }
    ifs3.close();
}

/*------------------------------------------------------------------------------------------*/
//To initialize weight Matrix
void initialize(vector<vector<double>> &mat)
{
    for (int i = 0; i < mat.size(); i++)
        for (int j = 0; j < mat[1].size(); j++)
            mat[i][j] = RAND_FLOAT();
}

/*------------------------------------------------------------------------------------------*/
//To initialize bias
void init(vector<double> &vec)
{
    for (int i = 0; i < vec.size(); i++)
        vec[i] = RAND_FLOAT();
}
/*------------------------------------------------------------------------------------------*/
//avtivation function
double sigmoid(double x)
{
    return (1.0f / (1.0f + exp(-x)));
}

/*------------------------------------------------------------------------------------------*/
void feedforward(vector<vector<double>> &x, vector<vector<double>> &wh, vector<vector<double>> &hidden, vector<vector<double>> &wout, vector<vector<double>> &out, vector<double> &bh, vector<double> &bout)
{
    int i, j, k;
    double temp;
    //Input to hidden layer
    for (i = 0; i < x.size(); i++)
    {
        for (j = 0; j < wh[1].size(); j++)
        {
            temp = 0.0;
            for (k = 0; k < wh.size(); ++k)
                temp += x[i][k] * wh[k][j];
            temp += bh[j];
            hidden[i][j] = sigmoid(temp);
        }
    }

    //Hidden to output layer
    for (i = 0; i < x.size(); i++)
    {
        for (j = 0; j < wout[1].size(); j++)
        {
            temp = 0.0;
            for (k = 0; k < wout.size(); ++k)
                temp += hidden[i][k] * wout[k][j];
            temp += bout[j];
            out[i][j] = sigmoid(temp);
        }
    }
}

/*------------------------------------------------------------------------------------------*/
void testing(vector<double> &x, vector<vector<double>> &wh, vector<double> &hidden, vector<vector<double>> &wout, vector<double> &out, vector<double> &bh, vector<double> &bout)
{
    int i, j, k;
    double temp;

    //Input to hidden layer
    for (i = 0; i < wh[1].size(); i++)
    {
        temp = 0.0;
        for (j = 0; j < wh.size(); j++)
            temp += wh[j][i] * x[j];
        temp += bh[i];
        hidden[i] = sigmoid(temp);
    }

    //Hidden to output layer
    for (i = 0; i < wout[1].size(); i++)
    {
        temp = 0.0;
        for (j = 0; j < wout.size(); j++)
            temp += wout[j][i] * hidden[j];
        temp += bout[i];
        out[i] = sigmoid(temp);
    }
}

/*------------------------------------------------------------------------------------------*/
//To calculate accuracy for Test data
void accuracy(int test_y, vector<double> &out, int &count)
{
    vector<double>::iterator it;
    float max = -1.0;
    int index;
    for (int i = 0; i < out.size(); i++)
    {
        if (max < out[i])
        {
            max = out[i];
            index = i;
        }
    }
    if (test_y == index)
        count++;
}

/*------------------------------------------------------------------------------------------*/
int main(int argc, char const *argv[])
{
    fileload();
    cout.flush();
    int m, i, j, e, k, count = 0;
    double temp, errtemp, error, start, stop;
    int samples = images.size();
    int test_samples = test_images.size();
    int batches = samples / MINIBATCH;

    cout << "\nNumber of Training Samples: " << num;
    cout << "\nEita: " << eta;
    cout << "\nMini-Batch Size: " << MINIBATCH;
    cout << "\nNumber of Epochs: " << epochs;

    vector<vector<double>> x(MINIBATCH, vector<double>(inputN, 0));  //Input to input layer
    vector<vector<double>> hidden(MINIBATCH, vector<double>(hN, 0)); //hidden layer output
    vector<vector<double>> out(MINIBATCH, vector<double>(outN, 0));  //predicted output
    vector<vector<double>> y(MINIBATCH, vector<double>(outN, 0));    // actual output

    vector<vector<double>> wh(inputN, vector<double>(hN)); //weights from input to hidden layer
    vector<vector<double>> wout(hN, vector<double>(outN)); //weights from hidden to output layer

    vector<double> bh(hN);     //bias for hidden layer
    vector<double> bout(outN); // bias for output layer

    vector<vector<double>> h_delta(MINIBATCH, vector<double>(hN));     // delta of hidden layer
    vector<vector<double>> out_delta(MINIBATCH, vector<double>(outN)); // delta of output layer

    vector<double> test_x(inputN, 0);  //Input layer for testing
    vector<double> test_hidden(hN, 0); //hidden layer for testing
    vector<double> test_out(outN, 0);  //output layer for testing

    srand(time(0));

    // Generate Train data samples
    for (m = 0; m < samples; m++)
    {
        for (i = 0; i < inputN; i++)
            data[m].input[i] = images[m][i];
        for (i = 0; i < outN; i++)
            data[m].label[i] = (i == labels[m] ? 1 : 0); //Making onehot vector
    }

    // Generate Test data samples
    for (m = 0; m < test_num; m++)
    {
        for (i = 0; i < inputN; i++)
            test[m].input[i] = test_images[m][i];
        test[m].label[0] = test_labels[m];
    }

    //Initialize weight between input and hidden layer
    initialize(wh);

    //Initialize weight between hidden and output layer
    initialize(wout);

    //Initialize bias
    init(bh);
    init(bout);

    // Training with batch learning
    cout << "\nStarting Training: ";
    start = omp_get_wtime();
    for (e = 0; e < epochs; e++)
    {
        error = 0.0;
        for (m = 0; m < batches; m++)
        {
            for (k = 0; k < MINIBATCH; k++)
            {
                for (i = 0; i < inputN; i++)
                    x[k][i] = data[m].input[i];
                for (i = 0; i < outN; i++)
                    y[k][i] = data[m].label[i];
            }

            feedforward(x, wh, hidden, wout, out, bh, bout);

            // Backpropagation
            //For output layer
            for (k = 0; k < MINIBATCH; k++)
            {
                for (i = 0; i < outN; i++)
                {
                    errtemp = y[k][i] - out[k][i];
                    out_delta[k][i] = errtemp * out[k][i] * (1.0 - out[k][i]);
                    error += 0.5 * errtemp * errtemp; //mean square
                }
            }

            // Stochastic gradient descent,Updating weights of output layer
            for (k = 0; k < MINIBATCH; k++)
            {
                for (i = 0; i < outN; i++)
                {
                    bout[i] += (eta * out_delta[k][i]);
                    for (j = 0; j < hN; j++)
                        wout[j][i] += (eta * out_delta[k][i] * hidden[k][j]);
                }
            }

            //for hidden layer
            for (k = 0; k < MINIBATCH; k++)
            {
                for (i = 0; i < hN; i++)
                {
                    errtemp = 0.0;
                    // #pragma omp parallel for
                    for (j = 0; j < outN; j++)
                        errtemp += out_delta[k][j] * wout[i][j];
                    h_delta[k][i] = errtemp * (hidden[k][i]) * (1.0 - hidden[k][i]);
                }
            }

            //update Weights of hidden layer
            for (k = 0; k < MINIBATCH; k++)
            {
                for (i = 0; i < hN; i++)
                {
                    bh[i] += (eta * h_delta[k][i]);
                    for (j = 0; j < inputN; j++)
                        wh[j][i] += (eta * h_delta[k][i] * x[k][j]);
                }
            }
        }

        // Global error
        error = error;
        stop = omp_get_wtime();
        cout << "\nEpoch " << e << " Completed!";
        printf("The %d th training, error: %f\n", e, error);
        cout << "\nTime Elapsed: " << stop - start;
    }

    stop = omp_get_wtime();
    cout << "\nTraining Finished!";
    cout << "\nTime Elapsed: " << stop - start;

    //Testing
    for (m = 0; m < test_num; m++)
    {
        for (i = 0; i < inputN; i++)
            test_x[i] = test[m].input[i];
        testing(test_x, wh, test_hidden, wout, test_out, bh, bout);
        accuracy(test[m].label[0], test_out, count);
    }

    cout << "\nAccuracy: ";
    float acc = ((float)count / (float)test_num) * 100.0;
    cout << acc << "\n";

    return 0;
}