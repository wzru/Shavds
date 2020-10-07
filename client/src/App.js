import React, { useReducer, useState } from "react";
import "./App.css";
import Code from "./components/code/code";
import File from "./components/file/file";
import Graph from "./components/graph/graph";
import Modal from "./components/modal/modal";
import Progress from "./components/progress/progress";
import Result from "./components/result/result";
import Tabbar from "./components/tabbar/tabbar";
import { CFG } from "./constants/pages";
import modalReducer from "./reducer/modalReducer";
import progressReducer from "./reducer/progressReducer";
import resultReducer from "./reducer/resultReducer";

function App() {
  const [modalState, dispatchModal] = useReducer(modalReducer, {});
  const [progressState, dispatchProgress] = useReducer(progressReducer, { "file1&file2": 0.5 });
  const [resultState, dispatchResult] = useReducer(resultReducer, { "file1&file2": [] });
  const [singleSelected, setSingleSelected] = useState("");
  const [multiSelected, setMultiSelected] = useState([]);
  const [code, setCode] = useState("");
  const [curTab, setCurTab] = useState(CFG);
  const [graph, setGraph] = useState({
    call: [""],
    graph: [""],
  });
  return (
    <div className="App">
      <Modal {...modalState} dispatchModal={dispatchModal} />
      <div className="left">
        <Tabbar
          curTab={curTab}
          setCurTab={setCurTab}
          multiSelected={multiSelected}
          singleSelected={singleSelected}
          dispatchProgress={dispatchProgress}
          dispatchResult={dispatchResult}
        />
        <File
          data={{
            singleSelected,
            setSingleSelected,
            multiSelected,
            setMultiSelected,
            setCode,
            graph,
            setGraph,
            dispatchModal,
          }}
        />
      </div>
      <div className="right">
        <div className="right-top">
          <Code code={code} name={singleSelected} />
          <Graph {...graph} />
        </div>
        <div className="right-bottom">
          <Progress progress={progressState} num={multiSelected.length} />
          <Result result={resultState} />
        </div>
      </div>
    </div>
  );
}

export default App;
