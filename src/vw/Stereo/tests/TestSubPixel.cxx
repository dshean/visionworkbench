// __BEGIN_LICENSE__
// Copyright (C) 2006-2010 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__


// TestCorrelator.h
#include <gtest/gtest.h>

#include <vw/Image/UtilityViews.h>
#include <vw/Stereo/CorrelatorView.h>
#include <vw/Stereo/SubpixelView.h>
#include <vw/Image/Transform.h>
#include <vw/Image.h>  // write_image
#include <vw/FileIO.h>

#include <boost/random/linear_congruential.hpp>

using namespace vw;
using namespace vw::stereo;

namespace vw {
  template<> struct PixelFormatID<Vector3>   { static const PixelFormatEnum value = VW_PIXEL_GENERIC_3_CHANNEL; };
}

template <int istretch>
class SubPixelCorrelateTest : public ::testing::Test {
  const int IMAGE_SIZE, HALF_IMAGE_SIZE;

public:
  SubPixelCorrelateTest() : IMAGE_SIZE(100), HALF_IMAGE_SIZE(50) {}

protected:
  typedef LogStereoPreprocessingFilter PreFilter;

  void SetUp() {
    stretch = float(istretch)/100;

    boost::rand48 gen(10);
    image1 = transform(channel_cast_rescale<uint8>(uniform_noise_view( gen, IMAGE_SIZE, IMAGE_SIZE )),
                       AffineTransform(Matrix2x2(3,0,0,3),Vector2()),
                       ZeroEdgeExtension(), BicubicInterpolation());
    translation = HALF_IMAGE_SIZE-HALF_IMAGE_SIZE*stretch;
    image2 = transform(image1, AffineTransform(Matrix2x2(stretch,0,0,1),
                                               Vector2(translation,0) ),
                       ZeroEdgeExtension(), BicubicInterpolation());

    starting_disp.set_size(IMAGE_SIZE,IMAGE_SIZE);
    for ( int i = 0; i < IMAGE_SIZE ; i++ ) {
      int disparity = stretch * i + translation - i;
      for ( int j = 0; j < IMAGE_SIZE; j++ )
        starting_disp(i,j) = disparity;
    }
  }

  template <class ViewT>
  double check_error( ImageViewBase<ViewT> const& input,
                      int32& invalid_count ) {
    ViewT const& disparity = input.impl();
    double error = 0;
    for ( int i = 0; i < IMAGE_SIZE; i++ ) {
      float expected = stretch * float(i) + translation - i;
      for ( int j = 0; j < IMAGE_SIZE; j++ ) {
        error += disparity(i,j)[1] + fabs(disparity(i,j)[0] - expected);
        if ( !is_valid(disparity(i,j)) )
          invalid_count++;
      }
    }
    return error / (double(IMAGE_SIZE)*double(IMAGE_SIZE));
  }

  float stretch, translation;
  ImageView<uint8> image1, image2;
  ImageView<PixelMask<Vector2f> > starting_disp;
};

typedef SubPixelCorrelateTest<95> SubPixelCorrelate95Test;
typedef SubPixelCorrelateTest<90> SubPixelCorrelate90Test;
typedef SubPixelCorrelateTest<80> SubPixelCorrelate80Test;
typedef SubPixelCorrelateTest<70> SubPixelCorrelate70Test;

// Testing Parabola SubPixel
//--------------------------------------------------------------
TEST_F( SubPixelCorrelate95Test, Parabola95 ) {
  typedef SubpixelView<PreFilter, ImageView<uint8> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp, image1, image2,
             7, 7, true, true, 1,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.341);
  EXPECT_LE(invalid_count, 0);
}

TEST_F( SubPixelCorrelate90Test, Parabola90 ) {
  typedef SubpixelView<PreFilter, ImageView<uint8> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp, image1, image2,
             7, 7, true, true, 1,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.383);
  EXPECT_LE(invalid_count, 0);
}

TEST_F( SubPixelCorrelate80Test, Parabola80 ) {
  typedef SubpixelView<PreFilter, ImageView<uint8> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp, image1, image2,
             7, 7, true, true, 1,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.313);
  EXPECT_LE(invalid_count, 0);
}

TEST_F( SubPixelCorrelate70Test, Parabola70 ) {
  typedef SubpixelView<PreFilter, ImageView<uint8> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp, image1, image2,
             7, 7, true, true, 1,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.429);
  EXPECT_LE(invalid_count, 0);
}

// Testing Bayes EM SubPixel
//--------------------------------------------------------------
TEST_F( SubPixelCorrelate95Test, BayesEM95 ) {
  typedef SubpixelView<PreFilter, ImageView<float> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp,
             channel_cast_rescale<float>(image1),
             channel_cast_rescale<float>(image2),
             7, 7, true, true, 2,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.054);
  EXPECT_LE(invalid_count, 0);
}

TEST_F( SubPixelCorrelate90Test, BayesEM90 ) {
  typedef SubpixelView<PreFilter, ImageView<float> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp,
             channel_cast_rescale<float>(image1),
             channel_cast_rescale<float>(image2),
             7, 7, true, true, 2,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.078);
  EXPECT_LE(invalid_count, 3);
}

TEST_F( SubPixelCorrelate80Test, BayesEM80 ) {
  typedef SubpixelView<PreFilter, ImageView<float> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp,
             channel_cast_rescale<float>(image1),
             channel_cast_rescale<float>(image2),
             7, 7, true, true, 2,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.125);
  EXPECT_LE(invalid_count, 3);
}

TEST_F( SubPixelCorrelate70Test, BayesEM70 ) {
  typedef SubpixelView<PreFilter, ImageView<float> > SubView;
  ImageView<PixelMask<Vector2f> > disparity_map =
    SubView( starting_disp,
             channel_cast_rescale<float>(image1),
             channel_cast_rescale<float>(image2),
             7, 7, true, true, 2,
             PreFilter(1.4), false );
  int32 invalid_count = 0;
  double error = check_error( disparity_map, invalid_count );
  //std::cout << "Err: " << error << " Cnt: " << invalid_count << "\n";
  EXPECT_LT(error, 0.198);
  EXPECT_LE(invalid_count, 7);
}
