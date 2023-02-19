#pragma once

#include <torch/torch.h>
#define cimg_use_jpeg
#include <CImg.h>

namespace AI::ImageProcessingUnit
{
using torch::Tensor; 

// at the time of writing this code (shortly after PyTorch 1.3),
// the C++ api wasn't complete and (in the case of ReLU) bug-free,
// so we define some Modules ad-hoc here.
// Chances are, that you can take standard models if and when
// they are done.

struct ConvTranspose2d : torch::nn::Module 
{
  // we don't do any of the running stats business
  std::vector<int64_t> stride_;
  std::vector<int64_t> padding_;
  std::vector<int64_t> output_padding_;
  std::vector<int64_t> dilation_;
  Tensor weight;
  Tensor bias;

  ConvTranspose2d(int64_t in_channels, int64_t out_channels, int64_t kernel_size, int64_t stride, 
                  int64_t padding, int64_t output_padding);

  Tensor forward(const Tensor &inp);
};

struct ResNetBlock : torch::nn::Module 
{
  torch::nn::Sequential conv_block;

  explicit ResNetBlock(int64_t dim);

  Tensor forward(const Tensor &inp);
};

struct ResNetGeneratorImpl : torch::nn::Module 
{
  torch::nn::Sequential model;

  ResNetGeneratorImpl(int64_t input_nc = 3, int64_t output_nc = 3, int64_t ngf = 64, int64_t n_blocks = 9); 
  
  Tensor forward(const Tensor &inp);
};

struct InputArg
{
	std::string inputImagePath_;
	std::string outputImagePath_;
	std::string xmlModel_;
};

class RunCycleganAPI 
{
public:
	void processImage(const void* pSender, InputArg& args);
};

TORCH_MODULE(ResNetGenerator);
}