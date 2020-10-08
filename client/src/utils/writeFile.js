import * as XLSX from "xlsx";

export default function writeWorkbookToLocalFile(jsonArr, sheetName, mapType) {
  const newjsonArr = jsonArr.map((oldJson) => {
    const newJson = {};
    for (const key in oldJson) {
      const newKey = mapType[key];
      newJson[newKey] = oldJson[key];
    }
    console.log(newJson);
    return newJson;
  });
  const workSheet = XLSX.utils.json_to_sheet(newjsonArr);
  const workBook = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(workBook, workSheet, sheetName);
  XLSX.writeFile(workBook, `${sheetName}-导出.xlsx`);
}
