import React from "react";
import css from "./tabbar.module.scss";
import { BUG, CFG, FUN } from "../../constants/pages";
import Axios from "../../config/axios";

import COPYICON from "../../assets/copy.svg";
import COPYICONB from "../../assets/copy-B.svg";
import ROCKETICON from "../../assets/rocket.svg";
import ROCKETICONO from "../../assets/rocket-O.svg";
import EXPORTICONG from "../../assets/daochu-G.svg";
import BUGICON from "../../assets/bug.svg";
import BUGICONR from "../../assets/bug-R.svg";

function Tabbar(props) {
  const { curTab, setCurTab, multiSelected, singleSelected, dispatchProgress, dispatchResult } = props;
  // 传入两个文件名并返回一个新对象
  const getObjByNames = (file1, file2, data) => {
    const newObj = {};
    newObj[`${file1}&${file2}`] = data;
    return newObj;
  };

  // 点击运行按钮
  const onRunClick = () => {
    dispatchProgress({ type: "empty" });
    dispatchResult({ type: "empty" });
    // 如果选择了多个文件
    if (multiSelected.length && multiSelected.length !== 1 && curTab !== BUG) {
      const type = curTab === CFG ? "cmpcfg" : "cmpfun";
      for (let i = 0; i < multiSelected.length; ++i) {
        for (let j = i + 1; j < multiSelected.length; ++j) {
          const file1 = multiSelected[i];
          const file2 = multiSelected[j];
          // 将查询进度设置为0
          dispatchProgress({ type: "add", data: getObjByNames(file1, file2, 0) });
          // 定义一个查询任务
          const id = setInterval(() => {
            // 查询进度
            Axios.get(`/progress?file1=${file1}&file2=${file2}`).then((res) => {
              dispatchProgress({ type: "add", data: getObjByNames(file1, file2, res.data.data) });
            });
          }, 100);
          Axios.get(`/${type}?file1=${file1}&file2=${file2}`).then((res) => {
            // 传递查询结果
            dispatchResult({ type: "add", data: getObjByNames(file1, file2, res.data.data) });
            // 将查询进度设置为1
            dispatchProgress({ type: "add", data: getObjByNames(file1, file2, 1) });
            // 清除查询任务
            clearInterval(id);
          });
        }
      }
    }
  };
  return (
    <div className={css["index"]}>
      <div className={css["button"]} onClick={() => setCurTab(CFG)}>
        <img alt="" src={curTab === CFG ? COPYICONB : COPYICON} />
        <div style={{ whiteSpace: "pre" }}>{"同源性检测\n(基于CFG)"}</div>
      </div>
      <div className={css["button"]} onClick={() => setCurTab(FUN)}>
        <img alt="" src={curTab === FUN ? COPYICONB : COPYICON} />
        <div style={{ whiteSpace: "pre" }}>{"同源性检测\n(基于字符串)"}</div>
      </div>
      <div className={css["button"]} onClick={() => setCurTab(BUG)}>
        <img alt="" src={curTab === BUG ? BUGICONR : BUGICON} />
        <div>漏洞检测</div>
      </div>
      <div className={css["button"]} onClick={onRunClick} style={{ marginTop: "50px" }}>
        <img
          alt=""
          src={
            (singleSelected && curTab === BUG) || (multiSelected.length && multiSelected.length !== 1 && curTab !== BUG)
              ? ROCKETICONO
              : ROCKETICON
          }
        />
        <div>运行</div>
      </div>
      <div className={css["button"]}>
        <img className="exporticon" alt="" src={EXPORTICONG} />
        <div>导出结果</div>
      </div>
    </div>
  );
}

export default Tabbar;
