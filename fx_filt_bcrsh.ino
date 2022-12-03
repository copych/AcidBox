/*
 * this file includes some simple effects
 * - dual filter
 * - bit crusher
 *
 * Author: Marcel Licence
 */

struct filterCoeffT{
    float aNorm[2] = {0.0f, 0.0f};
    float bNorm[3] = {1.0f, 0.0f, 0.0f};
};

struct filterProcT{
    struct filterCoeffT *filterCoeff;
    float w[3];
};

struct filterCoeffT filterGlobalC_LP, filterGlobalC_HP;
struct filterProcT mainFilterL_LP, mainFilterR_LP, mainFilterL_HP, mainFilterR_HP;

#define WAVEFORM_BIT  10UL
#define WAVEFORM_CNT  (1<<WAVEFORM_BIT)
#define WAVEFORM_Q4   (1<<(WAVEFORM_BIT-2))
#define WAVEFORM_MSK  ((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i) ((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK

float sine[WAVEFORM_CNT];


/*
 * calculate coefficients of the 2nd order IIR filter
 */
inline void Filter_CalculateTP(float c, float reso, struct filterCoeffT *const  filterC ){
    float *aNorm = filterC->aNorm;
    float *bNorm = filterC->bNorm;

    float Q = reso;
    float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

    /*
     * change curve of cutoff a bit
     * maybe also log or exp function could be used
     */
    c = c * c * c;

    if (c > 0.9975f ){
        omega = 0.9975f;
    }else if( c < 0.0025f ){
        omega = 0.0025f;
    }else{
        omega = c;
    }

    /*
     * use lookup here to get quicker results
     */
    cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
    sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];

    alpha = sinOmega / (2.0 * Q);
    b[0] = (1 - cosOmega) / 2;
    b[1] = 1 - cosOmega;
    b[2] = b[0];
    a[0] = 1 + alpha;
    a[1] = -2 * cosOmega;
    a[2] = 1 - alpha;

    // Normalize filter coefficients
    float factor = 1.0f / a[0];

    aNorm[0] = a[1] * factor;
    aNorm[1] = a[2] * factor;

    bNorm[0] = b[0] * factor;
    bNorm[1] = b[1] * factor;
    bNorm[2] = b[2] * factor;
}

inline void Filter_CalculateHP(float c, float reso, struct filterCoeffT *const  filterC ){
    float *aNorm = filterC->aNorm;
    float *bNorm = filterC->bNorm;

    float Q = reso;
    float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

    /*
     * change curve of cutoff a bit
     * maybe also log or exp function could be used
     */
    c = c * c * c;

    if (c > 0.9975f ){
        omega = 0.9975f;
    }else if( c < 0.0025f ){
        omega = 0.0025f;
    }else{
        omega = c;
    }

    /*
     * use lookup here to get quicker results
     */
    cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
    sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];

    alpha = sinOmega / (2.0 * Q);
    b[0] = (1 + cosOmega) / 2;
    b[1] = -(1 + cosOmega);
    b[2] = b[0];
    a[0] = 1 + alpha;
    a[1] = -2 * cosOmega;
    a[2] = 1 - alpha;

    // Normalize filter coefficients
    float factor = 1.0f / a[0];

    aNorm[0] = a[1] * factor;
    aNorm[1] = a[2] * factor;

    bNorm[0] = b[0] * factor;
    bNorm[1] = b[1] * factor;
    bNorm[2] = b[2] * factor;
}

inline void Filter_Process( float *const signal, struct filterProcT *const filterP ){
    const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
    filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
    filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
    *signal = out;
}


void Effect_Init( void ){
    for( int i = 0; i < WAVEFORM_CNT; i++ ){
        float val = (float)sin(i * 2.0 * PI / WAVEFORM_CNT);
        sine[i] = val;
    }

    mainFilterL_LP.filterCoeff = &filterGlobalC_LP;
    mainFilterR_LP.filterCoeff = &filterGlobalC_LP;
    mainFilterL_HP.filterCoeff = &filterGlobalC_HP;
    mainFilterR_HP.filterCoeff = &filterGlobalC_HP;   
}

float highpassC = 0.0f;
float lowpassC = 1.0f;
float filtReso = 1.0f;

float cutoff_hp_slow = 0.0f;
float cutoff_lp_slow = 1.0f;

static uint8_t effect_prescaler = 0;

float bitCrusher = 1.0f;

void Effect_Process( float *left, float *right ){
    effect_prescaler ++;

    Filter_Process(left, &mainFilterL_LP);
    Filter_Process(right, &mainFilterR_LP);
    Filter_Process(left, &mainFilterL_HP);
    Filter_Process(right, &mainFilterR_HP);

    if( cutoff_lp_slow > lowpassC ){
        cutoff_lp_slow -= 0.001;
    }
    if( cutoff_lp_slow < lowpassC ){
        cutoff_lp_slow += 0.001;
    }

    if( cutoff_hp_slow > highpassC ){
        cutoff_hp_slow -= 0.001;
    }
    if( cutoff_hp_slow < highpassC ){
        cutoff_hp_slow += 0.001;
    }

    /* we can not calculate in each cycle */
    if( effect_prescaler % 8 == 0 ){
        Filter_CalculateTP(cutoff_lp_slow, filtReso, &filterGlobalC_LP);
        Filter_CalculateHP(cutoff_hp_slow, filtReso, &filterGlobalC_HP);
    }

    if( bitCrusher < 1.0f ){
        int32_t ul = *left * bitCrusher * (1 << 29);
        *left = ((float)ul) / (bitCrusher * (float)(1 << 29));

        int32_t ur = *right * bitCrusher * (1 << 29);
        *right = ((float)ur) / (bitCrusher * (float)(1 << 29));
    }
}



void Effect_SetBiCutoff(float value ){
    highpassC = value >= 0.5 ? (value - 0.5f) * 2.0f : 0.0f;
    lowpassC = value <= 0.5 ? (value) * 2.0f : 1.0f;

    DEBF("Filter TP: %0.2f, HP: %02f\n", lowpassC, highpassC);
}

void Effect_SetBiReso(float value ){
    filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */

    DEBF("main filter reso: %0.3f\n", filtReso);

}

void Effect_SetBitCrusher( float value ){
    bitCrusher = pow(2, -32.0f * value);

    DEBF("main filter bitCrusher: %0.3f\n", bitCrusher);

}
/*
void Effect_Sync_Values(){
  
  // Sync the Values with the MIDI-Values created by the other Core
  if( global_biCutoff_midi != global_biCutoff_midi_old ){
    global_biCutoff_midi_old = global_biCutoff_midi;
    Effect_SetBiCutoff( global_biCutoff_midi *NORM127MUL );
  } 
  if( global_biReso_midi != global_biReso_midi_old ){
    global_biReso_midi_old = global_biReso_midi;
    Effect_SetBiReso( global_biReso_midi *NORM127MUL );
  } 
  if( global_bitcrush_midi != global_bitcrush_midi_old ){
    global_bitcrush_midi_old = global_bitcrush_midi;
    Effect_SetBitCrusher( global_bitcrush_midi *NORM127MUL );
  } 
  
}  
*/
