package utils

import (
	"log"
	"testing"
)

type ValueStruct struct {
	ValueTestA ValueStructA
	ValueTestB ValueStructB
}

type ValueStructA struct {
	StringValue    string
	NumericalValue int
	BoolValue      bool
}

type ValueStructB struct {
	FloatValue float64
}

func Test_JsonParse(t *testing.T) {
	jsonparse := NewJsonParser()

	value := ValueStruct{}

	err := jsonparse.Load("jsonparse_config.txt", &value)
	if err != nil {
		t.Error(err)
	}

	log.Println("value =", value)
	log.Println("value.ValueTestA.StringValue = ", value.ValueTestA.StringValue)

	value.ValueTestA.StringValue = "DZH"
	data, err := jsonparse.Marshal(&value)
	if err != nil {
		t.Error(err)
	}

	//save jsonstring
	err = jsonparse.Save("jsonparse_config2.txt", string(data))
	if err != nil {
		t.Error(err)
	}

	//save struct as json
	err = jsonparse.Save("jsonparse_config3.txt", value)
	if err != nil {
		t.Error(err)
	}
}

func Test_JsonReader(t *testing.T) {
	jsonReader := NewJsonReader()
	dataMap, err := jsonReader.Load("jsonparse_config.txt")
	if err != nil {
		t.Error(err)
	}
	log.Println("------")
	log.Println(dataMap)
	log.Println("ValueTestA.StringValue = ", dataMap["ValueTestA.StringValue"].(string))
	log.Println("ValueTestA.NumericalValue = ", dataMap["ValueTestA.NumericalValue"].(float64))
	log.Println("ValueTestA.BoolValue = ", dataMap["ValueTestA.BoolValue"].(bool))
	log.Println("ValueTestB.FloatValue = ", dataMap["ValueTestB.FloatValue"].(float64))
	log.Println("------")
}
