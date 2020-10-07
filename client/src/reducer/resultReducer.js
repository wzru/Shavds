export default function resultReducer(state, action) {
  switch (action.type) {
    case "empty":
      return {};
    case "add":
      return { ...state, ...action.data };
    default:
      return state;
  }
}
