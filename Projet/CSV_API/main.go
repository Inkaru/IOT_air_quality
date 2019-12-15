package main

import (
	"database/sql"
	"encoding/csv"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	_ "github.com/mattn/go-sqlite3"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

func handleError(err error){
	if err != nil {
		fmt.Println(err)
	}
}

// fonction utilisée par l'import depuis le m5satck
func ReadCSVFromHttpRequest(w http.ResponseWriter, req *http.Request) {
	// parse POST body as csv
	reader := csv.NewReader(req.Body)
	var results [][]string
	for {
		// read one row from csv
		record, err := reader.Read()
		if err == io.EOF {
			break
		}
		handleError(err)

		// add record to result set
		if record[0] != "2000-0-0 0:0:0" {
			results = append(results, record)
		}
	}

	fmt.Println("Starting import...")
	//database, _ := sql.Open("sqlite3", "./data.db")
	//database, _ := sql.Open("mysql", "iotapi:Inserts_AirQuality19@tcp(127.0.0.1:3306)/iotpollution")
	database, _ := sql.Open("mysql", "root:mysqladmin@tcp(127.0.0.1:3306)/test")
	statement, _ := database.Prepare("INSERT INTO data (timestamp, latitude, longitude, temperature, PM1_0 , PM2_5 , PM10 , 0_3um , 0_5um , 1_0um , 2_5um , 5_0um , 10um ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")

	tx, err := database.Begin()
	handleError(err)

	// Iterate through the records
	for _, s := range results {
		// Read each record from csv
		record := s
		if err == io.EOF {
			break
		}
		handleError(err)

		fmt.Println(record)
		_, err = tx.Stmt(statement).Exec(record[0], record[1], record[2], record[3], record[4], record[5], record[6], record[7], record[8], record[9], record[10], record[11], record[12])

		if err != nil {
			fmt.Println("error during import")
			log.Fatal(err)
			break
		}
	}

	if err != nil {
		fmt.Println("doing rollback")
		tx.Rollback()
	} else {
		tx.Commit()
	}

	fmt.Println("Import ended")
}

// fonctions utilisée par l'import depuis la page html
func uploadFile(w http.ResponseWriter, r *http.Request) {
	fmt.Println("File Upload Endpoint Hit")
	// Parse our multipart form, 10 << 20 specifies a maximum
	// upload of 10 MB files.
	r.ParseMultipartForm(10 << 20)
	// FormFile returns the first file for the given key `myFile`
	// it also returns the FileHeader so we can get the Filename,
	// the Header and the size of the file
	file, handler, err := r.FormFile("myFile")
	if err != nil {
		fmt.Println("Error Retrieving the File")
		fmt.Println(err)
		return
	}
	defer file.Close()
	fmt.Printf("Uploaded File: %+v\n", handler.Filename)
	fmt.Printf("File Size: %+v\n", handler.Size)
	fmt.Printf("MIME Header: %+v\n", handler.Header)

	// Create a temporary file within our temp-csv directory that follows
	// a particular naming pattern
	tempFile, err := ioutil.TempFile("temp-csv", "upload-*.csv")
	if err != nil {
		fmt.Println(err)
	}
	defer os.Remove(tempFile.Name())
	defer tempFile.Close()

	// read all of the contents of our uploaded file into a
	// byte array
	fileBytes, err := ioutil.ReadAll(file)
	if err != nil {
		fmt.Println(err)
	}
	// write this byte array to our temporary file
	tempFile.Write(fileBytes)
	// return that we have successfully uploaded our file!
	fmt.Fprintf(w, "Successfully Uploaded File\n")

	importCSV(tempFile.Name())

}

func importCSV(file string) {
	// Open the file
	csvfile, err := os.Open(file)
	defer csvfile.Close()
	if err != nil {
		log.Fatalln("Couldn't open the csv file", err)
	}

	// Parse the file
	r := csv.NewReader(csvfile)
	//r := csv.NewReader(bufio.NewReader(csvfile))

	fmt.Println("Starting import...")
	//database, _ := sql.Open("sqlite3", "./data.db")
	//database, _ := sql.Open("mysql", "iotapi:Inserts_AirQuality19@tcp(127.0.0.1:3306)/iotpollution")
	database, _ := sql.Open("mysql", "root:mysqladmin@tcp(127.0.0.1:3306)/test")
	statement, _ := database.Prepare("INSERT INTO data (timestamp, latitude, longitude, temperature, PM1_0 , PM2_5 , PM10 , 0_3um , 0_5um , 1_0um , 2_5um , 5_0um , 10um ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")

	tx, err := database.Begin()
	if err != nil {
		fmt.Println(err)
	}

	// Iterate through the records
	for {
		// Read each record from csv
		record, err := r.Read()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatal(err)
		}

		_, err = tx.Stmt(statement).Exec(record[0], record[1], record[2], record[3], record[4], record[5], record[6], record[7], record[8], record[9], record[10], record[11], record[12])

		if err != nil {
			fmt.Println("error during import")
			log.Fatal(err)
			break
		}
	}

	if err != nil {
		fmt.Println("doing rollback")
		tx.Rollback()
	} else {
		tx.Commit()
	}

	fmt.Println("Import ended")
}

func setupRoutes() {
	http.HandleFunc("/upload", uploadFile)
	http.HandleFunc("/csv", ReadCSVFromHttpRequest)
	http.HandleFunc("/get", func(w http.ResponseWriter, r *http.Request){
		fmt.Println("GET")
	})
	http.ListenAndServe(":8080", nil)
}

func setupDB() {
	//database, _ := sql.Open("sqlite3", "./data.db")
	//database, _ := sql.Open("mysql", "admin:Inserts_AirQuality19@tcp(127.0.0.1:3306)/iotpollution")
	database, _ := sql.Open("mysql", "root:mysqladmin@tcp(127.0.0.1:3306)/test")
	statement, err := database.Prepare("CREATE TABLE IF NOT EXISTS data (timestamp TIMESTAMP ,latitude FLOAT ,longitude FLOAT,temperature FLOAT,humidity FLOAT, PM1_0 INT, PM2_5 INT, PM10 INT, 0_3um INT, 0_5um INT, 1_0um INT, 2_5um INT, 5_0um INT, 10um INT)")
	if err != nil {
		fmt.Println(err)
	}
	statement.Exec()
}

func main() {
	fmt.Println("Hello World")
	setupDB()
	setupRoutes()
}
