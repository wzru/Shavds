import React, { useState, useEffect, useReducer } from "react";
import css from "./file.module.scss";
import Axios from "../../config/axios";
import DELETEICON from "../../assets/delete.svg";
import UPICON from "../../assets/upload.svg";
import ALLICON from "../../assets/all.svg";

export default function File(props) {
  useEffect(() => {
    getFileList();
  }, []);
  const {
    singleSelected,
    multiSelected,
    setSingleSelected,
    setMultiSelected,
    setCode,
    setGraph,
    dispatchModal,
  } = props.data;
  // 文件列表
  const [fileList, setFileList] = useState([]);
  // 单选文件
  const onSelectSingleFile = (e) => {
    const { name } = e.currentTarget.dataset;
    if (singleSelected === name) {
      setSingleSelected("");
      setCode("");
    } else {
      setSingleSelected(name);
      // 获取文件内容
      Axios.get(`/file/${name}`).then((res) => {
        setCode(res.data);
      });
      // 绘图
      Axios.post(`/draw?type=callgraph&file=${name}`).then((callRes) => {
        const newGraph = {};
        newGraph.call = callRes.data.data.images;
        Axios.post(`/draw?type=cfg&file=${name}`).then((cfgRes) => {
          newGraph.cfg = cfgRes.data.data.images;
          setGraph(newGraph);
        });
      });
    }
  };
  // 多选文件
  const onSelectMultiFile = (e) => {
    e.stopPropagation();
    const { name } = e.currentTarget.dataset;
    let newMultiSelected = multiSelected.slice(0);
    const index = multiSelected.indexOf(name);
    if (index === -1) {
      newMultiSelected = [...newMultiSelected, name];
    } else {
      newMultiSelected.splice(index, 1);
    }
    setMultiSelected(newMultiSelected);
  };
  // 上传文件
  const getFileList = () => {
    if (localStorage.getItem("token")) {
      Axios.get("/list").then((listRes) => {
        setFileList(listRes.data.data.list);
      });
    }
  };
  // 选择文件并上传
  const onSelectFile = (e) => {
    console.log(e.target.files);
    const { files } = e.target;
    const formData = new FormData();
    for (let i = 0; i < files.length; ++i) {
      formData.append("files", files[i]);
    }
    console.log(formData.getAll("files"));
    Axios.post("/upload", formData, {
      headers: { "Content-Type": "multipart/form-data" },
    }).then((uploadRes) => {
      localStorage.setItem("token", uploadRes.data.data);
      getFileList();
    });
  };
  // 删除文件
  const onDeleteClick = () => {
    if (singleSelected) {
      dispatchModal({
        type: "open",
        data: {
          title: "删除文件",
          content: `您确定要删除文件${singleSelected}吗？`,
          onConfirm: () => {
            dispatchModal({ type: "close" });
            Axios.delete(`/file/${singleSelected}`).then(() => getFileList());
          },
        },
      });
    }
  };
  // 全选文件
  const onAllClick = () => {
    if (multiSelected.length === fileList.length) {
      setMultiSelected([]);
    } else {
      setMultiSelected(fileList);
    }
  };
  return (
    <div className={css["index"]}>
      <div className={css["header"]}>
        <div className={css["header-title"]}>资源管理器</div>
        <div className={css["header-icons"]}>
          <img
            alt=""
            className={css["header-icons-item"]}
            src={DELETEICON}
            onClick={onDeleteClick}
            title="删除文件"
          />
          <div className={css["header-icons-item"]}>
            <img alt="" src={UPICON} />
            <input
              type="file"
              multiple
              accept=".c, .cpp, .go, .html, .css, .scss, .js, .json"
              onChange={onSelectFile}
            />
          </div>
          <img
            alt=""
            className={css["header-icons-item"]}
            src={ALLICON}
            onClick={onAllClick}
            title={
              multiSelected.length === fileList.length ? "取消全选" : "全选文件"
            }
          />
        </div>
      </div>
      <div className={css["list"]}>
        {fileList.map((item) => {
          return (
            <div
              key={item}
              data-name={item}
              className={css["list-item"]}
              onClick={onSelectSingleFile}
              style={{
                backgroundColor: singleSelected === item ? "#0074e8" : "",
                color: singleSelected === item ? "white" : "",
              }}
            >
              <img
                className={css["list-item-icon"]}
                src={require(`../../assets/${item.split(".")[1]}.svg`)}
                alt=""
              />
              <div className={css["list-item-name"]}>{item}</div>
              <input
                readOnly
                type="checkbox"
                checked={multiSelected.indexOf(item) === -1 ? false : true}
                onClick={onSelectMultiFile}
                data-name={item}
              />
            </div>
          );
        })}
      </div>
    </div>
  );
}
