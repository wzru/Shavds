export default function progressReducer(state, action) {
  switch (action.type) {
    case "empty":
      return {};
    case "add":
      return { ...state, ...action.data };
    default:
      return state;
  }
}
