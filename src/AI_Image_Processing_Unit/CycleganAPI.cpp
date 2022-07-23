#include "CycleganAPI.h"
#include <Logger.h>

namespace AI::ImageProcessingUnit
{
    ConvTranspose2d::ConvTranspose2d(int64_t in_channels, int64_t out_channels, int64_t kernel_size, int64_t stride, int64_t padding, int64_t output_padding) :
        stride_(2, stride),
        padding_(2, padding),
        output_padding_(2, output_padding),
        dilation_(2, 1),
        weight{ register_parameter("weight", torch::randn({ out_channels, in_channels, kernel_size, kernel_size })) },
        bias{ register_parameter("bias", torch::randn({ out_channels })) }
    {}

    Tensor ConvTranspose2d::forward(const Tensor &inp)
    {
        return conv_transpose2d(inp, weight, bias, stride_, padding_, output_padding_, /*groups=*/1, dilation_);
    }

    ResNetBlock::ResNetBlock(int64_t dim)
        : conv_block(
            torch::nn::ReflectionPad2d(1),
            torch::nn::Conv2d(torch::nn::Conv2dOptions(dim, dim, 3)),
            torch::nn::InstanceNorm2d(torch::nn::InstanceNorm2dOptions(dim)),
            torch::nn::ReLU(/*inplace=*/true),
            torch::nn::ReflectionPad2d(1),
            torch::nn::Conv2d(torch::nn::Conv2dOptions(dim, dim, 3)),
            torch::nn::InstanceNorm2d(
                torch::nn::InstanceNorm2dOptions(dim)))
    {
        register_module("conv_block", conv_block);
    }

    Tensor ResNetBlock::forward(const Tensor &inp)
    {
        return inp + conv_block->forward(inp);
    }


    ResNetGeneratorImpl::ResNetGeneratorImpl(int64_t input_nc, int64_t output_nc, int64_t ngf, int64_t n_blocks)
    {
        TORCH_CHECK(n_blocks >= 0);
        model->push_back(torch::nn::ReflectionPad2d(3));
        model->push_back(torch::nn::Conv2d(torch::nn::Conv2dOptions(input_nc, ngf, 7)));
        model->push_back(torch::nn::InstanceNorm2d(torch::nn::InstanceNorm2dOptions(7)));
        model->push_back(torch::nn::ReLU(/*inplace=*/true));
        constexpr int64_t n_downsampling = 2;

        for (int64_t i = 0; i < n_downsampling; i++)
        {
            int64_t mult = 1 << i;

            model->push_back(torch::nn::Conv2d(torch::nn::Conv2dOptions(ngf * mult, ngf * mult * 2, 3).stride(2).padding(1)));
            model->push_back(torch::nn::InstanceNorm2d(torch::nn::InstanceNorm2dOptions(ngf * mult * 2)));
            model->push_back(torch::nn::ReLU(/*inplace=*/true));
        }

        int64_t mult = 1 << n_downsampling;
        for (int64_t i = 0; i < n_blocks; i++)
        {
            model->push_back(ResNetBlock(ngf * mult));
        }

        for (int64_t i = 0; i < n_downsampling; i++)
        {
            mult = 1 << (n_downsampling - i);
            model->push_back(ConvTranspose2d(ngf * mult, ngf * mult / 2, /*kernel_size=*/3,/*stride=*/2, /*padding=*/1, /*output_padding=*/1));
            model->push_back(torch::nn::InstanceNorm2d(torch::nn::InstanceNorm2dOptions((ngf * mult / 2))));
            model->push_back(torch::nn::ReLU(/*inplace=*/true));
        }

        model->push_back(torch::nn::ReflectionPad2d(3));
        model->push_back(torch::nn::Conv2d(torch::nn::Conv2dOptions(ngf, output_nc, 7)));
        model->push_back(torch::nn::Tanh());
        register_module("model", model);
    }

    Tensor ResNetGeneratorImpl::forward(const Tensor &inp)
    {
        return model->forward(inp);
    }

    void RunCycleganAPI::processImage(const void* pSender, InputArg& args)
    {
        // AI_Image_Processing_Unit
        using namespace AI::ImageProcessingUnit;
        ResNetGenerator model;

        // Load AI model
        try {
            torch::load(model, args.xmlModel_);
        }
        catch (const Poco::Exception& e)
        {
            Common::Logging::Logger::log("error", "Horse2Zebra model", -1, e.what());
        }

        // Load  downloaded image 
        cimg_library::CImg<float> image(args.inputImagePath_.c_str());
        image.resize(400, 400);

        auto input_ = torch::tensor(torch::ArrayRef<float>(image.data(), image.size()));

        auto input = input_.reshape({ 1, 3, image.height(), image.width() });

        torch::NoGradGuard no_grad;

        model->eval();

        auto output = model->forward(input);

        cimg_library::CImg<float> out_img(output.data_ptr<float>(), static_cast<unsigned int>(output.size(3)),
            static_cast<unsigned int>(output.size(2)), 1, static_cast<unsigned int>(output.size(1)));

        // Normalize and save the output image
        out_img.normalize(0, 255);
        out_img.save(args.outputImagePath_.c_str());


#if 0
        cimg_library::CImgDisplay disp(out_img, "See a C++ API zebra!");

        while (!disp.is_closed())
        {
            disp.wait();
        }
#endif
    }

}