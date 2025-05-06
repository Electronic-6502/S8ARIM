# How the circuit works
The `LM393` with boundeled elements, creates a resonance based LC oscillator. The frequency of of oscillator is 
could be calculated from famous formula:

$f=\frac{1}{2\pi\sqrt{L*C}}$

(here, the `C` is `C3` and `L` is sum of `L1` and inductor under measure). But in this circuit, we have frequency and capacity of the capacitor.
and want to exactly calculate the `L1` and intended inductor. so the above formula is useless to us.
Now, why we have to find the value of `L1`? is it not intended to use a a ready made and fix inductor of `100uH`?
The reason is that, inductors in the market have a high tolerance (inductance error). this could cause a significant error in measurement. so first we need to calculate the precise value of `L1` with this formula:

$L1=4.\pi^2.f^2.C$

in above formula:
- `f`: amount of frequency (measured by micro controller) in `Hertz` unit
- `C`: value of `C3`, in `Farad` unit
This formula is implemented in the `EEPROM_Load_Saved` function.
Initial calibration of circuit is actually process of finding precise value of `L1` and saving that as frequency in EEPROm, noting more.
It should be confusing on how to save the `L1` value as frequency. Why it should be done like that?
The reson is that value of `L1` (which obviously is in `Hanry` unit), is a variable of type `float`. and saving a `float` variable in EEPROM have some troubles.
So i decided to make the code in a way that instead of directly storing the value of `L1` inductor, the frequency of osscilator in calibration process be stored in EEPROM.
And after each time the circuit is turned on, with above formula the `L1` be calculated from the above formula. such a clever solution.

No we have precise value of `L1`, and it is time to calculate the value of inductor under measurement. for one could use this formulation:

$L_x=(\frac{f1^2}{fx^2}-1)*L1$

In above formula, `f1` is the same frequency which is stored in the EEPROM. `fx` is also the frequency of resonator that is currently under measure. And `L1` is from last formula.

These are implemented in the `Measure_Inductance` function.

# Acknowledgement

Special thanks from `epsi1on` Because Translate and Create this Document    

Github : `github.com\epsi1on`