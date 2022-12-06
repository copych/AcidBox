#ifndef SP_TBVCF_H
#define SP_TBVCF_H


class SP_TBVCF {
	public:
		SP_TBVCF() {}
		inline void Init(float smp_rate);
		inline void setCutoff(float cutoff) { _fco = cutoff;} //1500.0 - 10000.0
		inline void setResonance(float reso) { _res = reso;} // 0.0 - 2.0
		inline float Process(float in);

	private:
		float _fco, _res, _dist, _asym, _iskip, _y, _y1, _y2;
		int _fcocod, _rezcod;
		float _sr;
		float _onedsr;
};
 


#endif
