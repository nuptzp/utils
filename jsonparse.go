package utils

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

type JsonParser struct {
}

func NewJsonParser() *JsonParser {
	return &JsonParser{}
}

func (self *JsonParser) Load(fileOrString string, v interface{}) error {
	data, err := ioutil.ReadFile(fileOrString)

	if err != nil {
		data = []byte(fileOrString)
	}

	err = json.Unmarshal(data, v)
	if err != nil {
		return err
	}
	return nil
}

func (self *JsonParser) Save(fileName string, v interface{}) (err error) {
	var data []byte
	switch v.(type) {
	case string:
		data = []byte(v.(string))
	default:
		data, err = json.Marshal(v)
		if err != nil {
			return err
		}
	}
	err = ioutil.WriteFile(fileName, data, os.ModeAppend)
	if err != nil {
		return err
	}
	return nil
}

func (self *JsonParser) Marshal(v interface{}) ([]byte, error) {
	data, err := json.Marshal(v)
	if err != nil {
		return nil, err
	}
	return data, nil
}

type JsonReader struct {
}

func NewJsonReader() *JsonReader {
	return &JsonReader{}
}

func (self *JsonReader) Load(fileOrString string) (map[string]interface{}, error) {
	data, err := ioutil.ReadFile(fileOrString)

	if err != nil {
		data = []byte(fileOrString)
	}

	jsonDecoder := json.NewDecoder(strings.NewReader(string(data)))

	dataMap := map[string]interface{}{}
	err = jsonDecoder.Decode(&dataMap)
	if err != nil {
		return nil, err
	}

	kvMap := getMap(dataMap)

	return kvMap, nil
}

func getMap(dataMap map[string]interface{}) map[string]interface{} {
	kvMap := make(map[string]interface{})
	for key, value := range dataMap {
		switch value.(type) {
		case map[string]interface{}:
			subkvMap := getMap(value.(map[string]interface{}))
			tempMap := fmtKVMap(key, subkvMap)
			appendMap(kvMap, tempMap)
		default:
			kvMap[key] = value
		}
	}
	return kvMap
}

func fmtKVMap(key string, subkvMap map[string]interface{}) map[string]interface{} {
	retMap := make(map[string]interface{})
	for subKey, value := range subkvMap {
		keyString := fmt.Sprintf("%s.%s", key, subKey)
		retMap[keyString] = value
	}
	return retMap
}

func appendMap(addedMap, addMap map[string]interface{}) map[string]interface{} {
	for key, value := range addMap {
		addedMap[key] = value
	}
	return addedMap
}
