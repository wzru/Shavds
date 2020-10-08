import React from "react";
import Axios from "../../config/axios";
import { BUGMAP } from "../../constants/bug";
import { BUG } from "../../constants/pages";
import css from "./result.module.scss";

export default function Result(props) {
  const { result, dispatchModal, curTab, setSingleSelected, setCode } = props;
  // 点击查看结果
  const onResultClick = (e) => {
    const { name } = e.currentTarget.dataset;
    // 如果是漏洞检测
    if (curTab === BUG) {
      // 单选文件
      setSingleSelected(name);
      Axios.get(`/file/${name}`).then((res) => {
        setCode(res.data);
        // 设置正常代码为绿色
        const allLines = document.querySelectorAll(".pre-line-norm");
        for (let i = 0; i < allLines.length; ++i) {
          allLines[i].style.display = "block";
        }
        // 将有漏洞的代码设置为红色
        result[name].map((bug) => {
          const bugMark = document.getElementsByClassName(`${name}-${bug.line}`)[0];
          // 去掉漏洞代码的绿色
          allLines[bug.line - 1].style.display = "none";
          if (bugMark.className.indexOf("bug") === -1) {
            bugMark.style.display = "flex";
            bugMark.innerHTML += BUGMAP[bug.type] + " ";
          }
        });
      });
    }
    // 如果是同源性检测
    else {
      dispatchModal({
        type: "open",
        data: {
          width: "40%",
          height: "40%",
          top: "30%",
          left: "30%",
          title: `${name}的比较结果`,
          content: result[name].map((cmp) => {
            return cmp.similarity !== 0 ? (
              <div className={css["result-item-item"]} key={cmp.file1 + cmp.file2 + cmp.func1 + cmp.func2}>
                第 <code> {cmp.line1}</code> 行的函数 <code> {cmp.func1}</code> 与第 <code> {cmp.line2}</code> 行的函数
                <code> {cmp.func2}</code>
                的相似度为 <code> {(cmp.similarity * 100).toFixed(2)} %</code>
              </div>
            ) : null;
          }),
          onConfirm: () => {
            dispatchModal({ type: "close" });
          },
          hideCancel: true,
        },
      });
    }
  };
  return (
    <div className={css["index"]}>
      <div className={css["title"]}>比较结果（点击查看详情）</div>
      <div className={css["result"]}>
        {Object.keys(result).map((filenames) => {
          return (
            <div key={filenames} className={css["result-item"]} data-name={filenames} onClick={onResultClick}>
              点击查看{filenames}的{curTab === BUG ? "漏洞检测结果" : "相似度"}
            </div>
          );
        })}
      </div>
    </div>
  );
}
