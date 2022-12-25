/*
 * reverb stuff
 *
 * This is a port of the public code from YetAnotherElectronicsChannel
 * Based on main.c I've created this module
 *
 * src: https://github.com/YetAnotherElectronicsChannel/STM32_DSP_Reverb/blob/master/code/Src/main.c
 *
 * The explanation of the original module can be found here: https://youtu.be/nRLXNmLmHqM
 *
 * Changes:
 * - optimized for buffer processing
 * - added interface to set the level
 *
 */

#define l_CB0 3620*2
#define l_CB1 1288*2
#define l_CB2 882*2
#define l_CB3 312*2
#define l_AP0 480*2
#define l_AP1 161*2
#define l_AP2 46*2

//rev_time 0.0 <-> 1.0
//rev_delay 0.0 <-> 1.0

class FxReverb {
	public:
	FxReverb() {}

	inline void Process( float *signal_l, float *signal_r ){

		float inSample;

		// create mono sample 
		inSample = *signal_l + *signal_r; // it may cause unwanted audible effects 
		//inSample *= 0.5f;

		// float newsample = (Do_Comb0(inSample) + Do_Comb1(inSample) + Do_Comb2(inSample) + Do_Comb3(inSample)) / 4.0f;
		float newsample = (Do_Comb0(inSample) + Do_Comb1(inSample) + Do_Comb2(inSample) + Do_Comb3(inSample)) * 0.125f;
		newsample = Do_Allpass0(newsample);
		newsample = Do_Allpass1(newsample);
		newsample = Do_Allpass2(newsample);

		// apply reverb level 
		newsample *= rev_level;

		*signal_l += newsample;
		*signal_r += newsample;

	};

	inline void Init(){ 
		SetLevel( 1.0f );
		SetTime( 0.7f );
	};
		
    inline void SetTime( float value ){
      rev_time = value;
      cf0_lim = (int)(rev_time * l_CB0);
      cf1_lim = (int)(rev_time * l_CB1);
      cf2_lim = (int)(rev_time * l_CB2);
      cf3_lim = (int)(rev_time * l_CB3);
      ap0_lim = (int)(rev_time * l_AP0);
      ap1_lim = (int)(rev_time * l_AP1);
      ap2_lim = (int)(rev_time * l_AP2);
      DEBF("reverb time: %0.3f\n", value);
    };
    
    inline void SetLevel( float value ){
      rev_level = value;
      DEBF("reverb level: %0.3f\n", value);
    };
		
	private:
		float rev_time = 0.5f;
		float rev_level = 0.5f;
		
		//define pointer limits = delay time
		int cf0_lim, cf1_lim, cf2_lim, cf3_lim, ap0_lim, ap1_lim, ap2_lim;

    
    inline float Do_Comb0( float inSample ){
      static float cfbuf0[l_CB0];
      static int cf0_p = 0;
      static float cf0_g = 0.805f;

      float readback = cfbuf0[cf0_p];
      float newV = readback * cf0_g + inSample;
      cfbuf0[cf0_p] = newV;
      cf0_p++;
      if( cf0_p == cf0_lim ){
        cf0_p = 0;
      }
      return readback;
    };

    inline float Do_Comb1( float inSample ){
      static float cfbuf1[l_CB1];
      static int cf1_p = 0;
      static float cf1_g = 0.827f;

      float readback = cfbuf1[cf1_p];
      float newV = readback * cf1_g + inSample;
      cfbuf1[cf1_p] = newV;
      cf1_p++;
      if( cf1_p == cf1_lim ){
        cf1_p = 0;
      }
      return readback;
    };

    inline float Do_Comb2( float inSample ){
      static float cfbuf2[l_CB2];
      static int cf2_p = 0;
      static float cf2_g = 0.783f;

      float readback = cfbuf2[cf2_p];
      float newV = readback * cf2_g + inSample;
      cfbuf2[cf2_p] = newV;
      cf2_p++;
      if( cf2_p == cf2_lim ){
        cf2_p = 0;
      }
      return readback;
    };

    inline float Do_Comb3( float inSample ){
      static float cfbuf3[l_CB3];
      static int cf3_p = 0;
      static float cf3_g = 0.764f;

      float readback = cfbuf3[cf3_p];
      float newV = readback * cf3_g + inSample;
      cfbuf3[cf3_p] = newV;
      cf3_p++;
      if( cf3_p == cf3_lim ){
        cf3_p = 0;
      }
      return readback;
    };


    inline float Do_Allpass0( float inSample ){
      static float apbuf0[l_AP0];
      static int ap0_p = 0;
      static float ap0_g = 0.7f;

      float readback = apbuf0[ap0_p];
      readback += (-ap0_g) * inSample;
      float newV = readback * ap0_g + inSample;
      apbuf0[ap0_p] = newV;
      ap0_p++;
      if( ap0_p == ap0_lim ){
        ap0_p = 0;
      }
      return readback;
    };

    inline float Do_Allpass1( float inSample ){
      static float apbuf1[l_AP1];
      static int ap1_p = 0;
      static float ap1_g = 0.7f;

      float readback = apbuf1[ap1_p];
      readback += (-ap1_g) * inSample;
      float newV = readback * ap1_g + inSample;
      apbuf1[ap1_p] = newV;
      ap1_p++;
      if( ap1_p == ap1_lim ){
        ap1_p = 0;
      }
      return readback;
    };

    inline float Do_Allpass2( float inSample ){
      static float apbuf2[l_AP2];
      static int ap2_p = 0;
      static float ap2_g = 0.7f;

      float readback = apbuf2[ap2_p];
      readback += (-ap2_g) * inSample;
      float newV = readback * ap2_g + inSample;
      apbuf2[ap2_p] = newV;
      ap2_p++;
      if( ap2_p == ap2_lim ){
        ap2_p = 0;
      }
      return readback;
    };


};
