// Package aqicalc provides a calculator to calculate AQI from the
// concentration value.
package main

type Conc struct {
	PM10_24hr, PM25_24hr float64
}

type AirQuality struct {
	AQI     int
	Primary string
}


var pm10_RANGE = []float64{0, 50, 150, 250, 350, 420, 500, 600}
var pm25_RANGE = []float64{0, 35, 75, 115, 150, 250, 350, 500}
var iaqi_RANGE = []float64{0, 50, 100, 150, 200, 300, 400, 500}

func linear(value, fromLow, fromHigh, toLow, toHigh float64) float64 {
	return ((value-fromLow)/(fromHigh-fromLow))*(toHigh-toLow) + toLow
}

func constrain(x, a, b float64) float64 {
	if x < a {
		return a
	} else if x > b {
		return b
	} else {
		return x
	}
}

func findDomainRange(d, r []float64, v float64) (d_lo, d_hi, r_lo, r_hi float64) {
	var idx int
	for i, _ := range d[:len(d)-1] {
		idx = i
		if d[i] <= v && v <= d[i+1] {
			break
		}
	}
	d_lo, d_hi, r_lo, r_hi = d[idx], d[idx+1], r[idx], r[idx+1]
	return
}

func toIAQI(conc float64, d []float64) float64 {
	var d_lo, d_hi, r_lo, r_hi float64 = findDomainRange(d, iaqi_RANGE, conc)
	return constrain(linear(conc, d_lo, d_hi, r_lo, r_hi), 0, 500)
}

func CalculateAQI(conc Conc) AirQuality {
	var ipm10, ipm25, max float64
	var primary string

	ipm10 = toIAQI(conc.PM10_24hr, pm10_RANGE)
	ipm25 = toIAQI(conc.PM25_24hr, pm25_RANGE)

	max, primary = ipm10, "PM10"

	if ipm25 >= max {
		max, primary = ipm25, "PM2.5"
	}

	return AirQuality{int(max + 0.5), primary}
}
